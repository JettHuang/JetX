// \brief
//	 abstract for OpenGL buffer object 
//

#ifndef __JETX_GL_BUFFER_H__
#define __JETX_GL_BUFFER_H__

#include <GL/glew.h>
#include <Common/RefCounting.h>

class FOpenGLDrv;

// glbuffer class
class FOpenGLBuffer : public FRefCountedObject
{
public:
	FOpenGLBuffer(FOpenGLDrv& InOwner, GLenum InType, GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage);

	virtual ~FOpenGLBuffer();

	bool IsValid() const;

	void Bind();

	// Lock Buffer
	// bDiscard:  discard the mapping data-content for write-only
	void* Lock(GLintptr InOffset, GLsizeiptr InLength, bool bReadOnly, bool bDiscard);

	void UnLock();

	GLuint GetGLResource() const { return Name; }

protected:
	FOpenGLDrv		&Owner;
	GLuint			Name;
	GLenum			Type;
	GLsizeiptr		SizeBytes;
	GLenum			Usage;
	bool			bIsLocked;
};


// Vertex Buffer
class FOpenGLVertexBuffer : public FOpenGLBuffer
{
public:
	FOpenGLVertexBuffer(FOpenGLDrv &InOwner, GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage = GL_STATIC_DRAW)
		: FOpenGLBuffer(InOwner, GL_ARRAY_BUFFER, InSize, InData, InUsage)
	{
	}
};

// Index Buffer
class FOpenGLIndexBuffer : public FOpenGLBuffer
{
public:
	FOpenGLIndexBuffer(FOpenGLDrv &InOwner, GLsizeiptr InSize, const GLvoid *InData, GLuint InStride, GLenum InUsage = GL_STATIC_DRAW)
		: FOpenGLBuffer(InOwner, GL_ELEMENT_ARRAY_BUFFER, InSize, InData, InUsage)
		, Stride(InStride)
	{
	}

	// the stride must be 2 or 4
	GLuint GetStride() { return Stride; }

protected:
	GLuint	Stride;
};

typedef TRefCountPtr<FOpenGLVertexBuffer>	FOpenGLVertexBufferRef;
typedef TRefCountPtr<FOpenGLIndexBuffer>	FOpenGLIndexBufferRef;

#endif // __JETX_GL_BUFFER_H__
