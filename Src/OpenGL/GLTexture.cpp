// \brief
//		Implementation for GL-Texture
//

#include "GLTexture.h"
#include "OpenGLDrv.h"


FOpenGLTexture2D::FOpenGLTexture2D(FOpenGLDrv &InOwner, GLint InInternalFormat, GLsizei InWidth, GLsizei InHeight, GLenum InDataFormat, GLenum InDataType, const GLvoid* InData)
	: Owner(InOwner)
	, Resource(0)
	, WrapS(GL_REPEAT)
	, WrapT(GL_REPEAT)
	, MinFilter(GL_LINEAR_MIPMAP_LINEAR)
	, MagFilter(GL_LINEAR)
{
	BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 0.f;

	glGenTextures(1, &Resource);
	Owner.CachedBindTextrue(0, GL_TEXTURE_2D, Resource);
	glTexImage2D(GL_TEXTURE_2D, 0, InInternalFormat, InWidth, InHeight, 0, InDataFormat, InDataType, InData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
	if (InData)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Owner.CheckError(__FILE__, __LINE__);
}

void FOpenGLTexture2D::SetWrapMode(GLint InWrapS, GLint InWrapT)
{
	WrapS = InWrapS;
	WrapT = InWrapT;

	Owner.CachedBindTextrue(0, GL_TEXTURE_2D, Resource);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapT);
}

void FOpenGLTexture2D::SetFilterMode(GLint InMin, GLint InMag)
{
	MinFilter = InMin;
	MagFilter = InMag;

	Owner.CachedBindTextrue(0, GL_TEXTURE_2D, Resource);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
}

void FOpenGLTexture2D::SetBorderColor(GLfloat r, float g, float b, float a)
{
	BorderColor[0] = r;
	BorderColor[1] = g;
	BorderColor[2] = b;
	BorderColor[3] = a;

	Owner.CachedBindTextrue(0, GL_TEXTURE_2D, Resource);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);
}

FOpenGLTexture2D::~FOpenGLTexture2D()
{
	if (Resource)
	{
		glDeleteTextures(1, &Resource);
	}
}

