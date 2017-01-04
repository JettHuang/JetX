// \brief
//	 abstract for OpenGL texture object 
//

#ifndef __JETX_GL_TEXTURE_H__
#define __JETX_GL_TEXTURE_H__


#include <GL/glew.h>
#include <Common/RefCounting.h>

class FOpenGLDrv;

// \brief
//		Texture
class FOpenGLTexture2D : public FRefCountedObject
{
public:
	FOpenGLTexture2D(class FOpenGLDrv &InOwner, GLint InInternalFormat, GLsizei InWidth, GLsizei InHeight, GLenum InDataFormat, GLenum InDataType, const GLvoid* InData);
	virtual ~FOpenGLTexture2D();

	void SetWrapMode(GLint InWrapS, GLint InWrapT);
	void SetFilterMode(GLint InMin, GLint InMag);
	void SetBorderColor(GLfloat r, float g, float b, float a);


	GLuint GetGLResource() { return Resource; }

protected:
	FOpenGLDrv	&Owner;
	GLuint		Resource;
	GLint		WrapS;
	GLint		WrapT;
	GLint		MinFilter;
	GLint		MagFilter;
	GLfloat		BorderColor[4];
};

typedef TRefCountPtr<FOpenGLTexture2D>		FOpenGLTexture2DRef;

#endif // !__JETX_GL_TEXTURE_H__
