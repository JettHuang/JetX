// \brief
//		GL Shader Object
//

#ifndef __JETX_GL_SHADER_H__
#define __JETX_GL_SHADER_H__

#include <string>
#include <vector>
#include <GL/glew.h>
#include <Common/RefCounting.h>

class FOpenGLDrv;


// \brief
//		OpenGL Shader
class FOpenGLShader : public FRefCountedObject
{
public:
	virtual ~FOpenGLShader();
	
	GLuint GetGLResource() const { return Resource; }
	bool IsValid() const;
	void DumpDebugInfo();

protected:
	FOpenGLShader(GLenum InType, const GLchar *InSource, GLint InLength=-1);

private:
	GLenum		ShaderType;
	GLuint		Resource;
	GLint		CompileStatus;
	GLint		InfoLogLength;
	GLchar     *InfoLog;
};


// Vertex Shader
class FOpenGLVertexShader : public FOpenGLShader
{
public:
	FOpenGLVertexShader(const GLchar *InSource, GLint InLength = -1);
};

// Fragment Shader
class FOpenGLPixelShader : public FOpenGLShader
{
public:
	FOpenGLPixelShader(const GLchar *InSource, GLint InLength = -1);
};

typedef TRefCountPtr<FOpenGLVertexShader>	FOpenGLVertexShaderRef;
typedef TRefCountPtr<FOpenGLPixelShader>	FOpenGLPixelShaderRef;


//////////////////////////////////////////////////////////////////////////
// GL Program

// Vertex Attributes of Program
class FOpenGLVertexAttribute
{
public:
	FOpenGLVertexAttribute()
		: Name("undefined")
		, Type(0)
		, Size(0)
		, Location(-1)
	{
	}

	FOpenGLVertexAttribute(const GLchar* InName, GLenum InType, GLint InSize, GLint InLocation)
		: Name(InName)
		, Type(InType)
		, Size(InSize)
		, Location(InLocation)
	{
	}

public:
	std::string		Name;
	GLenum			Type;
	GLint			Size;
	GLint			Location;
};

// Uniform Parameter of Program
class FOpenGLUniformParam
{
public:
	FOpenGLUniformParam()
		: Name("undefined")
		, Type(0)
		, Size(0)
		, Location(-1)
	{
	}

	FOpenGLUniformParam(const GLchar* InName, GLenum InType, GLint InSize, GLint InLocation)
		: Name(InName)
		, Type(InType)
		, Size(InSize)
		, Location(InLocation)
	{
	}

public:
	std::string		Name;
	GLenum			Type;
	GLint			Size;
	GLint			Location;
};

class FOpenGLProgram : public FRefCountedObject
{
public:
	FOpenGLProgram(FOpenGLVertexShader *InVertexShader, FOpenGLPixelShader *InPixelShader);
	virtual ~FOpenGLProgram();

	GLuint GetGLResource() const { return Resource; }
	bool IsValid() const;
	void DumpDebugInfo();

	GLint GetParamLocation(const std::string &InParamName) const;

private:
	GLuint		Resource;
	GLint		LinkStatus;
	GLint		InfoLogLength;
	GLchar     *InfoLog;

	GLint		AttributesNum;
	GLint		UniformsNum;
	std::vector<FOpenGLVertexAttribute>	Attributes;
	std::vector<FOpenGLUniformParam>	Uniforms;
};

typedef TRefCountPtr<FOpenGLProgram>	FOpenGLProgramRef;


#endif // __JETX_GL_SHADER_H__
