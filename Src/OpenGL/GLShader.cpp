// Implementation of GL-Shader 
//
//

#include <cassert>
#include <iostream>

#include "GLShader.h"
#include "OpenGLDrv.h"


FOpenGLShader::FOpenGLShader(GLenum InType, const GLchar *InSource, GLint InLength)
	: ShaderType(InType)
	, Resource(0)
	, CompileStatus(GL_FALSE)
	, InfoLogLength(0)
	, InfoLog(nullptr)
{
	assert(InSource);

	Resource = glCreateShader(ShaderType);
	glShaderSource(Resource, 1, &InSource, &InLength);
	glCompileShader(Resource);
	glGetShaderiv(Resource, GL_COMPILE_STATUS, &CompileStatus);
	glGetShaderiv(Resource, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0)
	{
		InfoLog = new GLchar[InfoLogLength];
		if (InfoLog)
		{
			glGetShaderInfoLog(Resource, InfoLogLength, nullptr, InfoLog);
		}
	}
}

FOpenGLShader::~FOpenGLShader()
{
	glDeleteShader(Resource);
	delete[] InfoLog;
}

bool FOpenGLShader::IsValid() const
{
	return glIsShader(Resource) == GL_TRUE;
}

void FOpenGLShader::DumpDebugInfo()
{
	std::cout << "GL-Shader Dump Debug Info:" << std::endl
		      << "    CompileStatus: " << (CompileStatus ? "GL_TRUE" : "GL_FALSE") << std::endl
		      << "    Info Log: " << (InfoLog ? InfoLog : "") << std::endl;
}


FOpenGLVertexShader::FOpenGLVertexShader(const GLchar *InSource, GLint InLength)
	: FOpenGLShader(GL_VERTEX_SHADER, InSource, InLength)
{
	
}

FOpenGLPixelShader::FOpenGLPixelShader(const GLchar *InSource, GLint InLength)
	: FOpenGLShader(GL_FRAGMENT_SHADER, InSource, InLength)
{

}


//////////////////////////////////////////////////////////////////////////
// Program
FOpenGLProgram::FOpenGLProgram(FOpenGLVertexShader *InVertexShader, FOpenGLPixelShader *InPixelShader)
	: Resource(0)
	, LinkStatus(GL_FALSE)
	, InfoLogLength(0)
	, InfoLog(nullptr)
	, AttributesNum(0)
	, UniformsNum(0)
{
	assert(InVertexShader && InPixelShader);

	Resource = glCreateProgram();
	glAttachShader(Resource, InVertexShader->GetGLResource());
	glAttachShader(Resource, InPixelShader->GetGLResource());
	glLinkProgram(Resource);
	glGetProgramiv(Resource, GL_LINK_STATUS, &LinkStatus);
	glGetProgramiv(Resource, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0)
	{
		InfoLog = new GLchar[InfoLogLength];
		if (InfoLog)
		{
			glGetProgramInfoLog(Resource, InfoLogLength, nullptr, InfoLog);
		}
	}
	glGetProgramiv(Resource, GL_ACTIVE_ATTRIBUTES, &AttributesNum);
	glGetProgramiv(Resource, GL_ACTIVE_UNIFORMS, &UniformsNum);

	GLchar VarName[128];
	GLint VarSize, VarLocation;
	GLenum VarType;

	// get active attributes
	for (GLint Index = 0; Index < AttributesNum; Index++)
	{
		glGetActiveAttrib(Resource, Index, sizeof(VarName), nullptr, &VarSize, &VarType, VarName);
		VarLocation = glGetAttribLocation(Resource, VarName);
		Attributes.push_back(FOpenGLVertexAttribute(VarName, VarType, VarSize, VarLocation));
	} // end for

	// get active uniforms
	for (GLint Index = 0; Index < UniformsNum; Index++)
	{
		glGetActiveUniform(Resource, Index, sizeof(VarName), nullptr, &VarSize, &VarType, VarName);
		VarLocation = glGetUniformLocation(Resource, VarName);
		Uniforms.push_back(FOpenGLUniformParam(VarName, VarType, VarSize, VarLocation));
	} // end for
}

FOpenGLProgram::~FOpenGLProgram()
{
	if (Resource)
	{
		glDeleteProgram(Resource);
	}
	delete[] InfoLog;
}

bool FOpenGLProgram::IsValid() const
{
	return glIsProgram(Resource) == GL_TRUE;
}

void FOpenGLProgram::DumpDebugInfo()
{
	std::cout << "GL-Program Dump Debug Info:" << std::endl
		<< "    LinkStatus: " << (LinkStatus ? "GL_TRUE" : "GL_FALSE") << std::endl
		<< "    Active Attributes Count: " << AttributesNum << std::endl
		<< "    Active Uniforms Count: " << UniformsNum << std::endl
		<< "    Info Log: " << (InfoLog ? InfoLog : "") << std::endl
		<< "    Attributes List: " << std::endl;
	// dump attributes
	for (size_t Index = 0; Index < Attributes.size(); Index++)
	{
		const FOpenGLVertexAttribute &Element = Attributes[Index];
		std::cout << "       name=" << Element.Name << ", type=" << FOpenGLDrv::LookupShaderAttributeTypeName(Element.Type) 
			<< ", size=" << Element.Size << ", location=" << Element.Location << std::endl;
	}
	// dump uniform params
	std::cout << "    Uniform Params List:" << std::endl;
	for (size_t Index = 0; Index < Uniforms.size(); Index++)
	{
		const FOpenGLUniformParam &Element = Uniforms[Index];
		std::cout << "       name=" << Element.Name << ", type=" << FOpenGLDrv::LookupShaderUniformTypeName(Element.Type) 
			<< ", size=" << Element.Size << ", location=" << Element.Location << std::endl;
	}
}

GLint FOpenGLProgram::GetParamLocation(const std::string &InParamName) const
{
	std::vector<FOpenGLUniformParam>::const_iterator It = Uniforms.begin();

	for (; It != Uniforms.end(); It++)
	{
		const FOpenGLUniformParam &Entry = *It;
		if (InParamName == Entry.Name)
		{
			return Entry.Location;
		}
	} // end for It

	return -1;
}
