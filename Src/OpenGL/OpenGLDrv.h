// \brief
//	abstract for opengl device layer. based on openg3.3 core
//

#ifndef __JETX_OPENGLDRV_H__
#define __JETX_OPENGLDRV_H__

#include <GL/glew.h>
#include "GLBuffer.h"
#include "GLShader.h"

// OpenGL Device 
class FOpenGLDrv
{
public:
	static FOpenGLDrv& SharedInstance();

	bool Initialize();
	void Terminate();

	// Create Resource
	FOpenGLVertexBufferRef CreateVertexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage = GL_STATIC_DRAW);
	FOpenGLIndexBufferRef CreateIndexBuffer(GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage = GL_STATIC_DRAW);
	FOpenGLVertexShaderRef CreateVertexShader(const GLchar *InSource, GLint InLength = -1);
	FOpenGLPixelShaderRef CreatePixelShader(const GLchar *InSource, GLint InLength = -1);
	FOpenGLProgramRef CreateProgram(const FOpenGLVertexShaderRef &InVertexShader, const FOpenGLPixelShaderRef &InPixelShader);

	// Operation State


	// Helper Functions
	void CheckError(const char* FILE, int LINE);

	// bind gl-buffer
	void CachedBindBuffer(GLenum InType, GLuint InName);
	void OnDeleteBuffer(GLenum InType, GLuint InName);

	// helpers
	static const GLchar* LookupShaderAttributeTypeName(GLenum InType);
	static const GLchar* LookupShaderUniformTypeName(GLenum InType);

protected:
	FOpenGLDrv();
	virtual ~FOpenGLDrv();
};



#endif // __JETX_OPENGLDRV_H__
