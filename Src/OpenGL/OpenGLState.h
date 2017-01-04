//  \brief
//		encapsulate the opengl state
//

#ifndef __JETX_GL_STATE_H__
#define __JETX_GL_STATE_H__

#include <GL/glew.h>
#include "GLBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLVertexDeclaration.h"
#include "GLShaderParameter.h"


#define	NUM_GL_STREAM_SOURCE		16
#define NUM_GL_VERTEX_ATTRIS		16
#define NUM_GL_TEXTURE_UNITS		8

// Vertex Stream State
struct FOpenGLStream
{
	FOpenGLVertexBuffer		*VertexBuffer;

	FOpenGLStream()
		: VertexBuffer(nullptr)
	{
	}
};

// Vertex Attributes State
struct FOpenGLCachedAttri
{
	GLuint		Buffer;
	GLint		Size;
	GLenum		Type;
	GLsizei		Stride;
	GLuint		Offset;
	GLboolean	Normalized;

	GLboolean	Enabled;

	FOpenGLCachedAttri()
		: Buffer(0)
		, Size(0)
		, Type(GL_NONE)
		, Stride(0)
		, Offset(0)
		, Normalized(GL_FALSE)
		, Enabled(GL_FALSE)
	{
	}
};


// Texture Sampler State
struct FOpenGLSamplerState
{
	GLuint		Texture;
	FOpenGLSamplerState()
		: Texture(0)
	{}
};

// Texture Sampler Stage
struct FOpenGLSamplerStage
{
	FOpenGLTexture2DRef	Texture2DRef;
};

struct FOpenGLState
{
	FOpenGLState()
		: VertexDeclaration(nullptr)
		, ShaderProgram(nullptr)
		, ShaderParameters(nullptr)
		, ActivetTexUnitIndex(0)
		, BindVertexBuffer(0)
		, BindIndexBuffer(0)
		, BindProgram(0)
		, SharedVertexArray(0)
		, BindRenderBuffer(0)
		, BindDrawFrameBuffer(0)
		, BindReadFrameBuffer(0)
	{
	}

	FOpenGLVertexDeclaration		*VertexDeclaration;
	FOpenGLStream					VertexStreams[NUM_GL_STREAM_SOURCE];
	FOpenGLCachedAttri				CachedAttris[NUM_GL_VERTEX_ATTRIS];
	FOpenGLProgram					*ShaderProgram;
	FProgramParameters				*ShaderParameters;

	FOpenGLSamplerStage				Texture2DStages[NUM_GL_TEXTURE_UNITS];
	FOpenGLSamplerState				Texture2DUnits[NUM_GL_TEXTURE_UNITS];
	GLint							ActivetTexUnitIndex;

	GLuint							BindVertexBuffer;
	GLuint							BindIndexBuffer;
	GLuint							BindProgram;
	GLuint							SharedVertexArray;
	GLuint							BindRenderBuffer;
	GLuint							BindDrawFrameBuffer;
	GLuint							BindReadFrameBuffer;
};
#endif // __JETX_GL_STATE_H__

