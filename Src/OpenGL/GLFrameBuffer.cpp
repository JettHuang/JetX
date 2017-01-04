// \brief
//		Frame Buffer Implementation
//

#include <iostream>

#include "OpenGLDrv.h"
#include "GLFrameBuffer.h"


FOpenGLFrameBuffer::FOpenGLFrameBuffer(FOpenGLDrv &InOwner)
	: Owner(InOwner)
	, DrawBufferMode(GL_FRONT)
	, ReadBufferMode(GL_FRONT)
{
	glGenFramebuffers(1, &Resource);
}

FOpenGLFrameBuffer::~FOpenGLFrameBuffer()
{
	glDeleteFramebuffers(1, &Resource);
}

// for texture
void FOpenGLFrameBuffer::SetColorAttachment(GLuint Index, const FOpenGLTexture2DRef &InTex2D)
{
	assert(Index < MAX_COLOR_ATTACHMENTS_NUM);
	ColorAttachments[Index] = InTex2D;

	GLuint TexName = 0;
	if (IsValidRef(InTex2D))
	{
		TexName = InTex2D->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GL_TEXTURE_2D, TexName, 0);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetDepthAttachment(const FOpenGLTexture2DRef &InTex2D)
{
	DepthAttachment = InTex2D;

	GLuint TexName = 0;
	if (IsValidRef(InTex2D))
	{
		TexName = InTex2D->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TexName, 0);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetStencilAttachment(const FOpenGLTexture2DRef &InTex2D)
{
	StencilAttachment = InTex2D;

	GLuint TexName = 0;
	if (IsValidRef(InTex2D))
	{
		TexName = InTex2D->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, TexName, 0);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetDepthStencilAttachment(const FOpenGLTexture2DRef &InTex2D)
{
	DepthAttachment = InTex2D;
	StencilAttachment = InTex2D;

	GLuint TexName = 0;
	if (IsValidRef(InTex2D))
	{
		TexName = InTex2D->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, TexName, 0);

	Owner.CheckError(__FILE__, __LINE__);
}

// for render-buffer
void FOpenGLFrameBuffer::SetColorAttachment(GLuint Index, const FOpenGLRenderBufferRef &InRenderBuffer)
{
	assert(Index < MAX_COLOR_ATTACHMENTS_NUM);
	ColorAttachments[Index] = InRenderBuffer;

	GLuint RenderName = 0;
	if (IsValidRef(InRenderBuffer))
	{
		RenderName = InRenderBuffer->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Index, GL_RENDERBUFFER, RenderName);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetDepthAttachment(const FOpenGLRenderBufferRef &InRenderBuffer)
{
	DepthAttachment = InRenderBuffer;

	GLuint RenderName = 0;
	if (IsValidRef(InRenderBuffer))
	{
		RenderName = InRenderBuffer->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderName);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetStencilAttachment(const FOpenGLRenderBufferRef &InRenderBuffer)
{
	StencilAttachment = InRenderBuffer;

	GLuint RenderName = 0;
	if (IsValidRef(InRenderBuffer))
	{
		RenderName = InRenderBuffer->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderName);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetDepthStencilAttachment(const FOpenGLRenderBufferRef &InRenderBuffer)
{
	DepthAttachment = InRenderBuffer;
	StencilAttachment = InRenderBuffer;

	GLuint RenderName = 0;
	if (IsValidRef(InRenderBuffer))
	{
		RenderName = InRenderBuffer->GetGLResource();
	}

	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderName);

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLFrameBuffer::SetDrawBuffer(GLenum InMode)
{
	DrawBufferMode = InMode;
	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glDrawBuffer(InMode);
}

void FOpenGLFrameBuffer::SetDrawBuffers(GLsizei n, const GLenum *InBufs)
{
	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glDrawBuffers(n, InBufs);
}

void FOpenGLFrameBuffer::SetReadBuffer(GLenum InMode)
{
	ReadBufferMode = InMode;
	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	glReadBuffer(InMode);
}

GLenum FOpenGLFrameBuffer::CheckStatus()
{
	Owner.CachedBindFrameBuffer(GL_FRAMEBUFFER, Resource);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	return Status;
}
