// \brief
//	abstract for opengl device layer. based on openg3.3 core
//

#ifndef __JETX_OPENGLDRV_H__
#define __JETX_OPENGLDRV_H__

#include <GL/glew.h>
#include "GLBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLVertexDeclaration.h"
#include "OpenGLState.h"
#include "GLRenderBuffer.h"
#include "GLFrameBuffer.h"


// OpenGL Device 
class FOpenGLDrv
{
public:
	static FOpenGLDrv& SharedInstance();

	bool Initialize();
	void DeferredInitialize();
	void Terminate();

	// Create Resource
	FOpenGLVertexBufferRef CreateVertexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage = GL_STATIC_DRAW);
	FOpenGLIndexBufferRef CreateIndexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLuint InStride, GLenum InUsage = GL_STATIC_DRAW);
	FOpenGLVertexShaderRef CreateVertexShader(const GLchar *InSource, GLint InLength = -1);
	FOpenGLPixelShaderRef CreatePixelShader(const GLchar *InSource, GLint InLength = -1);
	FOpenGLProgramRef CreateProgram(const FOpenGLVertexShaderRef &InVertexShader, const FOpenGLPixelShaderRef &InPixelShader);
	FOpenGLVertexDeclarationRef CreateVertexDeclaration(const FVertexElementsList &InVertexElements);
	FOpenGLTexture2DRef CreateTexture2D(GLint InInternalFormat, GLsizei InWidth, GLsizei InHeight, GLenum InDataFormat, GLenum InDataType, const GLvoid* InData);
	FOpenGLRenderBufferRef CreateRenderBuffer(GLenum InInternalformat, GLsizei InWidth, GLsizei InHeight);
	FOpenGLFrameBufferRef CreateFrameBuffer();

	// Operation State
	void SetClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void ClearBuffer(GLbitfield mask);

	void SetStreamSource(GLuint StreamIndex, const FOpenGLVertexBufferRef &InVertexBuffer);
	void SetVertexDeclaration(const FOpenGLVertexDeclarationRef &InVertexDecl);
	void SetShaderProgram(const FOpenGLProgramRef &InProgram);
	void SetShaderProgramParameters(FProgramParameters *InParameters);

	void SetTexture2D(GLuint TexIndex, const FOpenGLTexture2DRef &InTexture);
	void SetFrameBuffer(const FOpenGLFrameBufferRef &InFrameBuffer);

	void DrawIndexedPrimitive(const FOpenGLIndexBufferRef &InIndexBuffer, GLenum InMode, GLuint InStart, GLsizei InCount);
	void DrawArrayedPrimitive(GLenum InMode, GLint InStart, GLsizei InCount);

	// Frame-Buffer operate
	void BlitFramebuffer(const FOpenGLFrameBufferRef &InSrcFrameBuffer, const FOpenGLFrameBufferRef &InDstFrameBuffer, GLint InWidth, GLint InHeight,
		GLbitfield InMask = (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT), GLenum InFilter = GL_NEAREST);

	// Helper Functions
	void CheckError(const char* FILE, int LINE);

	// bind gl-buffer
	void CachedBindBuffer(GLenum InType, GLuint InName);
	void OnDeleteBuffer(GLenum InType, GLuint InName);
	// bind-texture
	void CachedBindTextrue(GLuint InTexUnit, GLenum InTarget, GLuint InTexName);
	// bind-render-buffer
	void CachedBindRenderBuffer(GLuint InName);
	// bind-frame-buffer
	void CachedBindFrameBuffer(GLenum InTarget, GLuint InName);

	// helpers
	static const GLchar* LookupShaderAttributeTypeName(GLenum InType);
	static const GLchar* LookupShaderUniformTypeName(GLenum InType);
	static const GLchar* LookupErrorCode(GLenum InError);

protected:
	FOpenGLDrv();
	virtual ~FOpenGLDrv();

	void SetupPendingShaderProgram();
	void SetupPendingVertexAttributeArray();
	void CachedEnableVertexAttributePointer(GLuint InBuffer, const FOpenGLVertexElement &InVertexElement);
	void CachedBindSharedVertexArrayObject();
	void SetupPendingTexture();
	void SetupPendingShaderProgramParameters();

	FOpenGLState	PendingState;
	FOpenGLState	CurrentState;
};



#endif // __JETX_OPENGLDRV_H__
