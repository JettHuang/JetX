// \brief
//		Frame Buffer class
//

#ifndef __JETX_GL_FRAMEBUFFER_H__
#define __JETX_GL_FRAMEBUFFER_H__

#include <vector>
#include <GL/glew.h>

#include <Common/RefCounting.h>
#include "GLTexture.h"
#include "GLRenderBuffer.h"


#define MAX_COLOR_ATTACHMENTS_NUM		4

class FOpenGLDrv;

// FrameBuffer class
class FOpenGLFrameBuffer : public FRefCountedObject
{
public:
	FOpenGLFrameBuffer(FOpenGLDrv &InOwner);
	virtual ~FOpenGLFrameBuffer();

	// for texture
	void SetColorAttachment(GLuint Index, const FOpenGLTexture2DRef &InTex2D);
	void SetDepthAttachment(const FOpenGLTexture2DRef &InTex2D);
	void SetStencilAttachment(const FOpenGLTexture2DRef &InTex2D);
	void SetDepthStencilAttachment(const FOpenGLTexture2DRef &InTex2D);

	// for render-buffer
	void SetColorAttachment(GLuint Index, const FOpenGLRenderBufferRef &InRenderBuffer);
	void SetDepthAttachment(const FOpenGLRenderBufferRef &InRenderBuffer);
	void SetStencilAttachment(const FOpenGLRenderBufferRef &InRenderBuffer);
	void SetDepthStencilAttachment(const FOpenGLRenderBufferRef &InRenderBuffer);

	// set draw buffer
	void SetDrawBuffer(GLenum InMode);
	void SetDrawBuffers(GLsizei n, const GLenum *InBufs);
	void SetReadBuffer(GLenum InMode);

	GLenum CheckStatus();

	GLuint GetGLResource() { return Resource; }

protected:
	FOpenGLDrv		&Owner;
	GLuint			Resource;

	FRefCountedObjectRef	ColorAttachments[MAX_COLOR_ATTACHMENTS_NUM];
	FRefCountedObjectRef	DepthAttachment;
	FRefCountedObjectRef	StencilAttachment;
	GLenum					DrawBufferMode;
	GLenum					ReadBufferMode;
};

typedef TRefCountPtr<FOpenGLFrameBuffer>	FOpenGLFrameBufferRef;


#endif // __JETX_GL_FRAMEBUFFER_H__
