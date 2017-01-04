// \brief
//		Render Buffer Object
//

#ifndef __JETX_GL_RENDERBUFFER_H__
#define __JETX_GL_RENDERBUFFER_H__


#include <GL/glew.h>
#include <Common/RefCounting.h>

class FOpenGLDrv;


// render buffer class
class FOpenGLRenderBuffer : public FRefCountedObject
{
public:
	FOpenGLRenderBuffer(FOpenGLDrv &InOwner, GLenum InInternalformat, GLsizei InWidth, GLsizei InHeight);
	virtual ~FOpenGLRenderBuffer();

	GLuint GetGLResource() { return Resource; }

protected:
	FOpenGLDrv	&Owner;
	GLenum		Internalformat;
	GLsizei		Width;
	GLsizei		Height;
	GLuint		Resource;
};

typedef TRefCountPtr<FOpenGLRenderBuffer>	FOpenGLRenderBufferRef;

#endif // __JETX_GL_RENDERBUFFER_H__
