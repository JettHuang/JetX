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

void FOpenGLDrv::Terminate()
{

}

void FOpenGLDrv::CheckError(const char* FILE, int LINE)
{
	GLenum Error = glGetError();

	if (Error != GL_NO_ERROR)
	{
		std::cout << "OpenGL Error: " << FILE << " At Line: " << LINE << ", Error-Code=" << Error;
	}
}

FOpenGLVertexBufferRef FOpenGLDrv::CreateVertexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage)
{
	return new FOpenGLVertexBuffer(*this, InSize, InData, InUsage);
}

FOpenGLIndexBufferRef FOpenGLDrv::CreateIndexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage)
{
	return new FOpenGLIndexBuffer(*this, InSize, InData, InUsage);
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

// bind gl-buffer
void FOpenGLDrv::CachedBindBuffer(GLenum InType, GLuint InName)
{

}

void FOpenGLDrv::OnDeleteBuffer(GLenum InType, GLuint InName)
{

}

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
