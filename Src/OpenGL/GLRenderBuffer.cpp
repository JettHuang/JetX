// \brief
//		implementation of Render Buffer
//

#include "GLRenderBuffer.h"
#include "OpenGLDrv.h"


FOpenGLRenderBuffer::FOpenGLRenderBuffer(FOpenGLDrv &InOwner, GLenum InInternalformat, GLsizei InWidth, GLsizei InHeight)
	: Owner(InOwner)
	, Internalformat(InInternalformat)
	, Width(InWidth)
	, Height(InHeight)
{
	glGenRenderbuffers(1, &Resource);
	Owner.CachedBindRenderBuffer(Resource);
	glRenderbufferStorage(GL_RENDERBUFFER, Internalformat, Width, Height);
}

FOpenGLRenderBuffer::~FOpenGLRenderBuffer()
{
	glDeleteRenderbuffers(1, &Resource);
}
