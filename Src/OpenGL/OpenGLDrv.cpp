// implementation for FOpenGLDrv
// NOTE: the glew init code is from glew-project,visualinfo.c
//

#include <iostream>

#include <GL/glew.h>
#if defined(GLEW_OSMESA)
#define GLAPI extern
#include <GL/osmesa.h>
#elif defined(GLEW_EGL)
#include <GL/eglew.h>
#elif defined(_WIN32)
#include <GL/wglew.h>
#elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#elif !defined(__HAIKU__)
#include <GL/glxew.h>
#endif

#ifdef GLEW_MX
GLEWContext _glewctx;
#  define glewGetContext() (&_glewctx)
#  ifdef _WIN32
WGLEWContext _wglewctx;
#    define wglewGetContext() (&_wglewctx)
#  elif !defined(__APPLE__) && !defined(__HAIKU__) || defined(GLEW_APPLE_GLX)
GLXEWContext _glxewctx;
#    define glxewGetContext() (&_glxewctx)
#  endif
#endif /* GLEW_MX */

typedef struct GLContextStruct
{
#if defined(GLEW_OSMESA)
	OSMesaContext ctx;
#elif defined(GLEW_EGL)
	EGLContext ctx;
#elif defined(_WIN32)
	HWND wnd;
	HDC dc;
	HGLRC rc;
#elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)
	CGLContextObj ctx, octx;
#elif !defined(__HAIKU__)
	Display* dpy;
	XVisualInfo* vi;
	GLXContext ctx;
	Window wnd;
	Colormap cmap;
#endif
} GLContext;

static void InitContext(GLContext* ctx);
static GLboolean CreateContext(GLContext* ctx);
static void DestroyContext(GLContext* ctx);

//\brief
//	initialize the gl extension functions address.
static bool InitializeGlew()
{
	GLenum err;
	GLContext ctx;

	/* create OpenGL rendering context */
	InitContext(&ctx);
	if (GL_TRUE == CreateContext(&ctx))
	{
		std::cout << "Error: CreateContext failed" << std::endl;
		DestroyContext(&ctx);
		return false;
	}

	/* initialize GLEW */
	glewExperimental = GL_TRUE;
#ifdef GLEW_MX
	err = glewContextInit(glewGetContext());
#  ifdef _WIN32
	err = err || wglewContextInit(wglewGetContext());
#  elif !defined(__APPLE__) && !defined(__HAIKU__) || defined(GLEW_APPLE_GLX)
	err = err || glxewContextInit(glxewGetContext());
#  endif
#else
	err = glewInit();
#endif
	if (GLEW_OK != err)
	{
		std::cout << "Error [main]: glewInit failed: " << glewGetErrorString(err) << std::endl;
		DestroyContext(&ctx);
		return false;
	}

	DestroyContext(&ctx);
	return true;
}


#if defined(GLEW_OSMESA)
static void InitContext(GLContext* ctx)
{
	ctx->ctx = NULL;
}

static const GLint osmFormat = GL_UNSIGNED_BYTE;
static const GLint osmWidth = 640;
static const GLint osmHeight = 480;
static GLubyte *osmPixels = NULL;

static GLboolean CreateContext(GLContext* ctx)
{
	if (NULL == ctx) return GL_TRUE;
	ctx->ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);
	if (NULL == ctx->ctx) return GL_TRUE;
	if (NULL == osmPixels)
	{
		osmPixels = (GLubyte *)calloc(osmWidth*osmHeight * 4, 1);
	}
	if (!OSMesaMakeCurrent(ctx->ctx, osmPixels, GL_UNSIGNED_BYTE, osmWidth, osmHeight))
	{
		return GL_TRUE;
	}
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	if (NULL == ctx) return;
	if (NULL != ctx->ctx) OSMesaDestroyContext(ctx->ctx);
}
/* ------------------------------------------------------------------------ */

#elif defined(GLEW_EGL)
static void InitContext(GLContext* ctx)
{
	ctx->ctx = NULL;
}

static GLboolean CreateContext(GLContext* ctx)
{
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	if (NULL == ctx) return;
	return;
}

/* ------------------------------------------------------------------------ */

#elif defined(_WIN32)

static void InitContext(GLContext* ctx)
{
	ctx->wnd = NULL;
	ctx->dc = NULL;
	ctx->rc = NULL;
}

static GLboolean CreateContext(GLContext* ctx)
{
	WNDCLASS wc;
	PIXELFORMATDESCRIPTOR pfd;
	/* check for input */
	if (NULL == ctx) return GL_TRUE;
	/* register window class */
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = TEXT("GLEW");
	if (0 == RegisterClass(&wc)) return GL_TRUE;
	/* create window */
	ctx->wnd = CreateWindow(TEXT("GLEW"), TEXT("GLEW"), 0, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
		GetModuleHandle(NULL), NULL);
	if (NULL == ctx->wnd) return GL_TRUE;
	/* get the device context */
	ctx->dc = GetDC(ctx->wnd);
	if (NULL == ctx->dc) return GL_TRUE;
	/* find pixel format */
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	int iPixelFormat = ChoosePixelFormat(ctx->dc, &pfd);
	
	/* set the pixel format for the dc */
	if (FALSE == SetPixelFormat(ctx->dc, iPixelFormat, &pfd)) return GL_TRUE;
	/* create rendering context */
	ctx->rc = wglCreateContext(ctx->dc);
	if (NULL == ctx->rc) return GL_TRUE;
	if (FALSE == wglMakeCurrent(ctx->dc, ctx->rc)) return GL_TRUE;
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	if (NULL == ctx) return;
	if (NULL != ctx->rc) wglMakeCurrent(NULL, NULL);
	if (NULL != ctx->rc) wglDeleteContext(wglGetCurrentContext());
	if (NULL != ctx->wnd && NULL != ctx->dc) ReleaseDC(ctx->wnd, ctx->dc);
	if (NULL != ctx->wnd) DestroyWindow(ctx->wnd);
	UnregisterClass(TEXT("GLEW"), GetModuleHandle(NULL));
}

/* ------------------------------------------------------------------------ */

#elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)

static void InitContext(GLContext* ctx)
{
	ctx->ctx = NULL;
	ctx->octx = NULL;
}

static GLboolean CreateContext(GLContext* ctx)
{
	CGLPixelFormatAttribute attrib[] = { kCGLPFAAccelerated, 0 };
	CGLPixelFormatObj pf;
	GLint npix;
	CGLError error;
	/* check input */
	if (NULL == ctx) return GL_TRUE;
	error = CGLChoosePixelFormat(attrib, &pf, &npix);
	if (error) return GL_TRUE;
	error = CGLCreateContext(pf, NULL, &ctx->ctx);
	if (error) return GL_TRUE;
	CGLReleasePixelFormat(pf);
	ctx->octx = CGLGetCurrentContext();
	error = CGLSetCurrentContext(ctx->ctx);
	if (error) return GL_TRUE;
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	if (NULL == ctx) return;
	CGLSetCurrentContext(ctx->octx);
	if (NULL != ctx->ctx) CGLReleaseContext(ctx->ctx);
}

/* ------------------------------------------------------------------------ */

#elif defined(__HAIKU__)

static void InitContext(GLContext* ctx)
{
	/* TODO */
}

static GLboolean CreateContext(GLContext* ctx)
{
	/* TODO */
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	/* TODO */
}

/* ------------------------------------------------------------------------ */

#else /* __UNIX || (__APPLE__ && GLEW_APPLE_GLX) */

static void InitContext(GLContext* ctx)
{
	ctx->dpy = NULL;
	ctx->vi = NULL;
	ctx->ctx = NULL;
	ctx->wnd = 0;
	ctx->cmap = 0;
}

static GLboolean CreateContext(GLContext* ctx)
{
	int attrib[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
	int erb, evb;
	XSetWindowAttributes swa;
	/* check input */
	if (NULL == ctx) return GL_TRUE;
	/* open display */
	ctx->dpy = XOpenDisplay(display);
	if (NULL == ctx->dpy) return GL_TRUE;
	/* query for glx */
	if (!glXQueryExtension(ctx->dpy, &erb, &evb)) return GL_TRUE;
	/* choose visual */
	ctx->vi = glXChooseVisual(ctx->dpy, DefaultScreen(ctx->dpy), attrib);
	if (NULL == ctx->vi) return GL_TRUE;
	/* create context */
	ctx->ctx = glXCreateContext(ctx->dpy, ctx->vi, None, True);
	if (NULL == ctx->ctx) return GL_TRUE;
	/* create window */
	/*wnd = XCreateSimpleWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 1, 1, 1, 0, 0);*/
	ctx->cmap = XCreateColormap(ctx->dpy, RootWindow(ctx->dpy, ctx->vi->screen),
		ctx->vi->visual, AllocNone);
	swa.border_pixel = 0;
	swa.colormap = ctx->cmap;
	ctx->wnd = XCreateWindow(ctx->dpy, RootWindow(ctx->dpy, ctx->vi->screen),
		0, 0, 1, 1, 0, ctx->vi->depth, InputOutput, ctx->vi->visual,
		CWBorderPixel | CWColormap, &swa);
	/* make context current */
	if (!glXMakeCurrent(ctx->dpy, ctx->wnd, ctx->ctx)) return GL_TRUE;
	return GL_FALSE;
}

static void DestroyContext(GLContext* ctx)
{
	if (NULL != ctx->dpy && NULL != ctx->ctx) glXDestroyContext(ctx->dpy, ctx->ctx);
	if (NULL != ctx->dpy && 0 != ctx->wnd) XDestroyWindow(ctx->dpy, ctx->wnd);
	if (NULL != ctx->dpy && 0 != ctx->cmap) XFreeColormap(ctx->dpy, ctx->cmap);
	if (NULL != ctx->vi) XFree(ctx->vi);
	if (NULL != ctx->dpy) XCloseDisplay(ctx->dpy);
}

#endif /* __UNIX || (__APPLE__ && GLEW_APPLE_GLX) */


//////////////////////////////////////////////////////////////////////////
#include "OpenGLDrv.h"

FOpenGLDrv& FOpenGLDrv::SharedInstance()
{
	static FOpenGLDrv GLDriver;

	return GLDriver;
}


FOpenGLDrv::FOpenGLDrv()
{

}

FOpenGLDrv::~FOpenGLDrv()
{

}

bool FOpenGLDrv::Initialize()
{
	if (!InitializeGlew())
	{
		return false;
	}

	return true;
}

void FOpenGLDrv::DeferredInitialize()
{
	CachedBindSharedVertexArrayObject();
}

void FOpenGLDrv::Terminate()
{
	if (CurrentState.SharedVertexArray == 0)
	{
		glDeleteVertexArrays(1, &CurrentState.SharedVertexArray);
	}
}

void FOpenGLDrv::CheckError(const char* FILE, int LINE)
{
	GLenum Error = glGetError();

	if (Error != GL_NO_ERROR)
	{
		std::cout << "OpenGL Error: " << FILE << " At Line: " << LINE << ", Error-Code=" << Error
			<< " --- " << LookupErrorCode(Error) << std::endl;
	}
}

FOpenGLVertexBufferRef FOpenGLDrv::CreateVertexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage)
{
	return new FOpenGLVertexBuffer(*this, InSize, InData, InUsage);
}

FOpenGLIndexBufferRef FOpenGLDrv::CreateIndexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLuint InStride, GLenum InUsage)
{
	return new FOpenGLIndexBuffer(*this, InSize, InData, InStride, InUsage);
}

FOpenGLVertexShaderRef FOpenGLDrv::CreateVertexShader(const GLchar *InSource, GLint InLength)
{
	return new FOpenGLVertexShader(InSource, InLength);
}

FOpenGLPixelShaderRef FOpenGLDrv::CreatePixelShader(const GLchar *InSource, GLint InLength)
{
	return new FOpenGLPixelShader(InSource, InLength);
}

FOpenGLProgramRef FOpenGLDrv::CreateProgram(const FOpenGLVertexShaderRef &InVertexShader, const FOpenGLPixelShaderRef &InPixelShader)
{
	return new FOpenGLProgram((FOpenGLVertexShader*)InVertexShader, (FOpenGLPixelShader*)InPixelShader);
}

FOpenGLVertexDeclarationRef FOpenGLDrv::CreateVertexDeclaration(const FVertexElementsList &InVertexElements)
{
	return new FOpenGLVertexDeclaration(InVertexElements);
}

FOpenGLTexture2DRef FOpenGLDrv::CreateTexture2D(GLint InInternalFormat, GLsizei InWidth, GLsizei InHeight, GLenum InDataFormat, GLenum InDataType, const GLvoid* InData)
{
	return new FOpenGLTexture2D(*this, InInternalFormat, InWidth, InHeight, InDataFormat, InDataType, InData);
}

FOpenGLRenderBufferRef FOpenGLDrv::CreateRenderBuffer(GLenum InInternalformat, GLsizei InWidth, GLsizei InHeight)
{
	return new FOpenGLRenderBuffer(*this, InInternalformat, InWidth, InHeight);
}

FOpenGLFrameBufferRef FOpenGLDrv::CreateFrameBuffer()
{
	return new FOpenGLFrameBuffer(*this);
}

// Operation State
void FOpenGLDrv::SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glClearColor(r, g, b, a);
}

void FOpenGLDrv::ClearBuffer(GLbitfield mask)
{
	glClear(mask);
}

void FOpenGLDrv::SetStreamSource(GLuint StreamIndex, const FOpenGLVertexBufferRef &InVertexBuffer)
{
	assert(StreamIndex < NUM_GL_STREAM_SOURCE);

	PendingState.VertexStreams[StreamIndex].VertexBuffer = (FOpenGLVertexBuffer*)InVertexBuffer;
}

void FOpenGLDrv::SetVertexDeclaration(const FOpenGLVertexDeclarationRef &InVertexDecl)
{
	PendingState.VertexDeclaration = (FOpenGLVertexDeclaration*)InVertexDecl;
}

void FOpenGLDrv::SetShaderProgram(const FOpenGLProgramRef &InProgram)
{
	PendingState.BindProgram = 0;
	PendingState.ShaderProgram = (FOpenGLProgram*)InProgram;
	if (PendingState.ShaderProgram)
	{
		PendingState.BindProgram = InProgram->GetGLResource();
	}
}

void FOpenGLDrv::SetShaderProgramParameters(FProgramParameters *InParameters)
{
	PendingState.ShaderParameters = InParameters;
}

void FOpenGLDrv::SetTexture2D(GLuint TexIndex, const FOpenGLTexture2DRef &InTexture)
{
	assert(TexIndex < NUM_GL_TEXTURE_UNITS);
	PendingState.Texture2DStages[TexIndex].Texture2DRef = InTexture;
}

void FOpenGLDrv::SetFrameBuffer(const FOpenGLFrameBufferRef &InFrameBuffer)
{
	GLuint Resource = 0; // default buffer

	if (IsValidRef(InFrameBuffer))
	{
		Resource = InFrameBuffer->GetGLResource();
	}

	CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
}

void FOpenGLDrv::BlitFramebuffer(const FOpenGLFrameBufferRef &InSrcFrameBuffer, const FOpenGLFrameBufferRef &InDstFrameBuffer, GLint InWidth, GLint InHeight,
	GLbitfield InMask, GLenum InFilter)
{
	GLuint Src = 0, Dst = 0;

	if (IsValidRef(InSrcFrameBuffer))
	{
		Src = InSrcFrameBuffer->GetGLResource();
	}
	if (IsValidRef(InDstFrameBuffer))
	{
		Dst = InDstFrameBuffer->GetGLResource();
	}

	CachedBindFrameBuffer(GL_READ_FRAMEBUFFER, Src);
	CachedBindFrameBuffer(GL_DRAW_FRAMEBUFFER, Dst);
	glBlitFramebuffer(0, 0, InWidth, InHeight, 0, 0, InWidth, InHeight, InMask, InFilter);
}

void FOpenGLDrv::DrawIndexedPrimitive(const FOpenGLIndexBufferRef &InIndexBuffer, GLenum InMode, GLuint InStart, GLsizei InCount)
{
	// bind shader program
	SetupPendingShaderProgram();
	// Set Program Parameters
	SetupPendingShaderProgramParameters();
	// Bind Vertex Attributes
	SetupPendingVertexAttributeArray();
	// Setup Texture
	SetupPendingTexture();

	GLenum IndexType = InIndexBuffer->GetStride() == sizeof(GLushort) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	GLuint StartPtr = InStart * InIndexBuffer->GetStride();
	CachedBindBuffer(GL_ELEMENT_ARRAY_BUFFER, InIndexBuffer->GetGLResource());
	glDrawElements(InMode, InCount, IndexType, (GLvoid*)StartPtr);
	CheckError(__FILE__, __LINE__);
}

void FOpenGLDrv::DrawArrayedPrimitive(GLenum InMode, GLint InStart, GLsizei InCount)
{
	// bind shader program
	SetupPendingShaderProgram();
	// Set Program Parameters
	SetupPendingShaderProgramParameters();
	// Bind Vertex Attributes
	SetupPendingVertexAttributeArray();
	// Setup Texture
	SetupPendingTexture();

	glDrawArrays(InMode, InStart, InCount);
	CheckError(__FILE__, __LINE__);
}

void FOpenGLDrv::SetupPendingShaderProgram()
{
	assert(PendingState.ShaderProgram);
	if (CurrentState.BindProgram != PendingState.BindProgram)
	{
		glUseProgram(PendingState.BindProgram);
		CheckError(__FILE__, __LINE__);

		CurrentState.BindProgram = PendingState.BindProgram;
	}
}

void FOpenGLDrv::SetupPendingVertexAttributeArray()
{
	assert(PendingState.VertexDeclaration);
	GLboolean VertexAttrisEnables[NUM_GL_VERTEX_ATTRIS] = { GL_FALSE };
	const FOpenGLVertexElementsList &VertexElementList = PendingState.VertexDeclaration->GLVertexElements;
	for (size_t Index = 0; Index < VertexElementList.size(); Index++)
	{
		const FOpenGLVertexElement &Element = VertexElementList[Index];
		const GLuint kStreamIndex = Element.StreamIndex;
		const GLuint kAttriIndex = Element.AttributeIndex;

		assert(kStreamIndex < NUM_GL_STREAM_SOURCE);
		assert(kAttriIndex < NUM_GL_VERTEX_ATTRIS);
		FOpenGLVertexBuffer *SourceStream = PendingState.VertexStreams[kStreamIndex].VertexBuffer;
		assert(SourceStream);

		CachedEnableVertexAttributePointer(SourceStream->GetGLResource(), Element);
		VertexAttrisEnables[kAttriIndex] = GL_TRUE;
	} // end for
	for (GLuint Index = 0; Index < NUM_GL_VERTEX_ATTRIS; Index++)
	{
		if (VertexAttrisEnables[Index] == GL_FALSE && CurrentState.CachedAttris[Index].Enabled)
		{
			glDisableVertexAttribArray(Index);
			CheckError(__FILE__, __LINE__);
			CurrentState.CachedAttris[Index].Enabled = GL_FALSE;
		}
	} // end for
}

void FOpenGLDrv::CachedEnableVertexAttributePointer(GLuint InBuffer, const FOpenGLVertexElement &InVertexElement)
{
	const GLuint kAttriIndex = InVertexElement.AttributeIndex;
	FOpenGLCachedAttri &CurrentAttri = CurrentState.CachedAttris[kAttriIndex];

	if (CurrentAttri.Buffer != InBuffer ||
		CurrentAttri.Type != InVertexElement.Type ||
		CurrentAttri.Size != InVertexElement.Size ||
		CurrentAttri.Stride != InVertexElement.Stride ||
		CurrentAttri.Offset != InVertexElement.Offset ||
		CurrentAttri.Normalized != InVertexElement.Normalized)
	{
		CachedBindBuffer(GL_ARRAY_BUFFER, InBuffer);
		if (InVertexElement.ShouldConvertToFloat)
		{
			glVertexAttribPointer(kAttriIndex, InVertexElement.Size, InVertexElement.Type, InVertexElement.Normalized, InVertexElement.Stride, (GLvoid*)InVertexElement.Offset);
		}
		else
		{
			glVertexAttribIPointer(kAttriIndex, InVertexElement.Size, InVertexElement.Type, InVertexElement.Stride, (GLvoid*)InVertexElement.Offset);
		}
		CheckError(__FILE__, __LINE__);

		CurrentAttri.Buffer = InBuffer;
		CurrentAttri.Type = InVertexElement.Type;
		CurrentAttri.Size = InVertexElement.Size;
		CurrentAttri.Stride = InVertexElement.Stride;
		CurrentAttri.Offset = InVertexElement.Offset;
		CurrentAttri.Normalized = InVertexElement.Normalized;
	}

	if (!CurrentAttri.Enabled)
	{
		glEnableVertexAttribArray(kAttriIndex);
		CheckError(__FILE__, __LINE__);

		CurrentAttri.Enabled = GL_TRUE;
	}
}

void FOpenGLDrv::CachedBindSharedVertexArrayObject()
{
	if (CurrentState.SharedVertexArray == 0)
	{
		glGenVertexArrays(1, &CurrentState.SharedVertexArray);
		glBindVertexArray(CurrentState.SharedVertexArray);
	}
}

void FOpenGLDrv::SetupPendingTexture()
{
	for (int Index = 0; Index < NUM_GL_TEXTURE_UNITS; Index++)
	{
		FOpenGLTexture2D *Texture = (FOpenGLTexture2D*)(PendingState.Texture2DStages[Index].Texture2DRef);
		if (!Texture)
		{
			CachedBindTextrue(Index, GL_TEXTURE_2D, 0);
			continue;
		}

		CachedBindTextrue(Index, GL_TEXTURE_2D, Texture->GetGLResource());
	} // end for
}

void FOpenGLDrv::SetupPendingShaderProgramParameters()
{
	FProgramParameters *Parameters = PendingState.ShaderParameters;
	FOpenGLProgram *Program = PendingState.ShaderProgram;

	assert(Program);
	if (Parameters)
	{
		for (FProgramParameters::const_iterator It = Parameters->begin(); It != Parameters->end(); It++)
		{
			FShaderParameter *Param = *It;
			assert(Param);
			Param->ApplyValue(*Program);
			CheckError(__FILE__, __LINE__);
		} // end for
	}
}

// bind gl-buffer
void FOpenGLDrv::CachedBindBuffer(GLenum InType, GLuint InName)
{
	switch (InType)
	{
	case GL_ARRAY_BUFFER:
	{
		if (CurrentState.BindVertexBuffer != InName)
		{
			glBindBuffer(GL_ARRAY_BUFFER, InName);
			CheckError(__FILE__, __LINE__);
			CurrentState.BindVertexBuffer = InName;
		}
	}
	break;
	case GL_ELEMENT_ARRAY_BUFFER:
	{
		if (CurrentState.BindIndexBuffer != InName)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, InName);
			CheckError(__FILE__, __LINE__);
			CurrentState.BindIndexBuffer = InName;
		}
	}
	break;
	default:
		std::cout << "Error: Not Implement Type In CachedBindBuffer()" << std::endl;
		assert(false);
		break;
	}
}

void FOpenGLDrv::OnDeleteBuffer(GLenum InType, GLuint InName)
{
	switch (InType)
	{
	case GL_ARRAY_BUFFER:
	{
		if (CurrentState.BindVertexBuffer == InName)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			CurrentState.BindVertexBuffer = 0;
		}
	}
	break;
	case GL_ELEMENT_ARRAY_BUFFER:
	{
		if (CurrentState.BindIndexBuffer == InName)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			CurrentState.BindIndexBuffer = 0;
		}
	}
	break;
	default:
		std::cout << "Error: Not Implement Type In OnDeleteBuffer()" << std::endl;
		assert(false);
		break;
	}
}

void FOpenGLDrv::CachedBindTextrue(GLuint InTexUnit, GLenum InTarget, GLuint InTexName)
{
	assert(InTexUnit < NUM_GL_TEXTURE_UNITS);
	if (InTexUnit != CurrentState.ActivetTexUnitIndex)
	{
		glActiveTexture(GL_TEXTURE0 + InTexUnit);
		CurrentState.ActivetTexUnitIndex = InTexUnit;
		CheckError(__FILE__, __LINE__);
	}

	FOpenGLSamplerState &SamplerState = CurrentState.Texture2DUnits[InTexUnit];
	if (SamplerState.Texture != InTexName)
	{
		assert(InTarget == GL_TEXTURE_2D);
		glBindTexture(InTarget, InTexName);
		SamplerState.Texture = InTexName;
		CheckError(__FILE__, __LINE__);
	}
}

// bind-render-buffer
void FOpenGLDrv::CachedBindRenderBuffer(GLuint InName)
{
	if (InName != CurrentState.BindRenderBuffer)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, InName);
		CurrentState.BindRenderBuffer = InName;
	}
}

// bind-frame-buffer
void FOpenGLDrv::CachedBindFrameBuffer(GLenum InTarget, GLuint InName)
{
	assert(InTarget == GL_FRAMEBUFFER || InTarget == GL_DRAW_FRAMEBUFFER || InTarget == GL_READ_FRAMEBUFFER);
	if (InTarget == GL_FRAMEBUFFER)
	{
		if (CurrentState.BindReadFrameBuffer != InName
			|| CurrentState.BindDrawFrameBuffer != InName)
		{
			glBindFramebuffer(InTarget, InName);
			CurrentState.BindReadFrameBuffer = InName;
			CurrentState.BindDrawFrameBuffer = InName;
		}
	}
	else if (InTarget == GL_DRAW_FRAMEBUFFER)
	{
		if (CurrentState.BindDrawFrameBuffer != InName)
		{
			glBindFramebuffer(InTarget, InName);
			CurrentState.BindDrawFrameBuffer = InName;
		}
	} 
	else
	{
		if (CurrentState.BindReadFrameBuffer != InName)
		{
			glBindFramebuffer(InTarget, InName);
			CurrentState.BindReadFrameBuffer = InName;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// helpers

struct FTypeNamePair
{
	GLenum	Type;
	const GLchar *Name;
};
static const GLchar *kUnknown = "unknown";

#define DEF_TYPENAME_PAIR(Type)		{ Type, #Type }
#define DEF_ARRAYCOUNT(Array)		(sizeof(Array)/sizeof(Array[0]))

const GLchar* FOpenGLDrv::LookupShaderAttributeTypeName(GLenum InType)
{
	static const FTypeNamePair kTypeNames[] =
	{
		DEF_TYPENAME_PAIR(GL_FLOAT),
		DEF_TYPENAME_PAIR(GL_FLOAT_VEC2),
        DEF_TYPENAME_PAIR(GL_FLOAT_VEC3),
		DEF_TYPENAME_PAIR(GL_FLOAT_VEC4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2x3),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2x4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3x2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3x4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4x2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4x3),
		DEF_TYPENAME_PAIR(GL_INT),
		DEF_TYPENAME_PAIR(GL_INT_VEC2),
		DEF_TYPENAME_PAIR(GL_INT_VEC3),
		DEF_TYPENAME_PAIR(GL_INT_VEC4),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC2),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC3),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC4),
		DEF_TYPENAME_PAIR(GL_DOUBLE),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2x3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2x4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3x2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3x4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4x2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4x3)
	};

	for (GLint Index = 0; Index < DEF_ARRAYCOUNT(kTypeNames); Index++)
	{
		const FTypeNamePair &Element = kTypeNames[Index];
		if (Element.Type == InType)
		{
			return Element.Name;
		}
	} // end for

	return kUnknown;
}

const GLchar* FOpenGLDrv::LookupShaderUniformTypeName(GLenum InType)
{
	static const FTypeNamePair kTypeNames[] =
	{
		DEF_TYPENAME_PAIR(GL_FLOAT),
		DEF_TYPENAME_PAIR(GL_FLOAT_VEC2),
		DEF_TYPENAME_PAIR(GL_FLOAT_VEC3),
		DEF_TYPENAME_PAIR(GL_FLOAT_VEC4),
		DEF_TYPENAME_PAIR(GL_DOUBLE),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_VEC4),
		DEF_TYPENAME_PAIR(GL_INT),
		DEF_TYPENAME_PAIR(GL_INT_VEC2),
		DEF_TYPENAME_PAIR(GL_INT_VEC3),
		DEF_TYPENAME_PAIR(GL_INT_VEC4),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC2),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC3),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_VEC4),
		DEF_TYPENAME_PAIR(GL_BOOL),
		DEF_TYPENAME_PAIR(GL_BOOL_VEC2),
		DEF_TYPENAME_PAIR(GL_BOOL_VEC3),
		DEF_TYPENAME_PAIR(GL_BOOL_VEC4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2x3),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT2x4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3x2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT3x4),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4x2),
		DEF_TYPENAME_PAIR(GL_FLOAT_MAT4x3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2x3),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT2x4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3x2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT3x4),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4x2),
		DEF_TYPENAME_PAIR(GL_DOUBLE_MAT4x3),
		DEF_TYPENAME_PAIR(GL_SAMPLER_1D),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D),
		DEF_TYPENAME_PAIR(GL_SAMPLER_3D),
		DEF_TYPENAME_PAIR(GL_SAMPLER_CUBE),
		DEF_TYPENAME_PAIR(GL_SAMPLER_1D_SHADOW),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_SHADOW),
		DEF_TYPENAME_PAIR(GL_SAMPLER_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_SAMPLER_1D_ARRAY_SHADOW),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_ARRAY_SHADOW),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_SAMPLER_CUBE_SHADOW),
		DEF_TYPENAME_PAIR(GL_SAMPLER_BUFFER),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_RECT),
		DEF_TYPENAME_PAIR(GL_SAMPLER_2D_RECT_SHADOW),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_1D),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_2D),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_3D),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_CUBE),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_BUFFER),
		DEF_TYPENAME_PAIR(GL_INT_SAMPLER_2D_RECT),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_1D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_2D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_3D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_CUBE),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_BUFFER),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_SAMPLER_2D_RECT),
		DEF_TYPENAME_PAIR(GL_IMAGE_1D),
		DEF_TYPENAME_PAIR(GL_IMAGE_2D),
		DEF_TYPENAME_PAIR(GL_IMAGE_3D),
		DEF_TYPENAME_PAIR(GL_IMAGE_2D_RECT),
		DEF_TYPENAME_PAIR(GL_IMAGE_CUBE),
		DEF_TYPENAME_PAIR(GL_IMAGE_BUFFER),
		DEF_TYPENAME_PAIR(GL_IMAGE_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_IMAGE_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_IMAGE_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_IMAGE_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_1D),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_2D),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_3D),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_2D_RECT),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_CUBE),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_BUFFER),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_1D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_2D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_3D),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_2D_RECT),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_CUBE),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_BUFFER),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_1D_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_2D_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY),
		DEF_TYPENAME_PAIR(GL_UNSIGNED_INT_ATOMIC_COUNTER)
	};

	for (GLint Index = 0; Index < DEF_ARRAYCOUNT(kTypeNames); Index++)
	{
		const FTypeNamePair &Element = kTypeNames[Index];
		if (Element.Type == InType)
		{
			return Element.Name;
		}
	} // end for

	return kUnknown;
}

const GLchar* FOpenGLDrv::LookupErrorCode(GLenum InError)
{
	static const FTypeNamePair kTypeNames[] =
	{
		DEF_TYPENAME_PAIR(GL_NO_ERROR),
		DEF_TYPENAME_PAIR(GL_INVALID_ENUM),
		DEF_TYPENAME_PAIR(GL_INVALID_VALUE),
		DEF_TYPENAME_PAIR(GL_INVALID_OPERATION),
		DEF_TYPENAME_PAIR(GL_INVALID_FRAMEBUFFER_OPERATION),
		DEF_TYPENAME_PAIR(GL_OUT_OF_MEMORY),
		DEF_TYPENAME_PAIR(GL_STACK_UNDERFLOW),
		DEF_TYPENAME_PAIR(GL_STACK_OVERFLOW)
	};

	for (GLint Index = 0; Index < DEF_ARRAYCOUNT(kTypeNames); Index++)
	{
		const FTypeNamePair &Element = kTypeNames[Index];
		if (Element.Type == InError)
		{
			return Element.Name;
		}
	} // end for

	return kUnknown;
}
