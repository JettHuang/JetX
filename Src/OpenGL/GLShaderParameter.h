// \brief
//		shader parameter class
//

#ifndef __JETX_GL_SHADERPARAMETER_H__
#define __JETX_GL_SHADERPARAMETER_H__


#include <string>
#include <vector>
#include <GL/glew.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Common/RefCounting.h>
#include "GLShader.h"


// \brief
//	Abstract Shader Program Parameter
class FShaderParameter : public FRefCountedObject
{
public:
	FShaderParameter(const std::string &InName)
		: Name(InName)
	{}

	virtual ~FShaderParameter()
	{}

	const std::string& GetName() { return Name; }

	virtual void ApplyValue(FOpenGLProgram& Program) const = 0;

protected:
	std::string		Name;
};


//////////////////////////////////////////////////////////////////////////
class FShaderParameter_IntegerVector : public FShaderParameter
{
public:
	FShaderParameter_IntegerVector(const std::string &InName, GLint *InValue, GLsizei  InCount)
		: FShaderParameter(InName)
		, pValue(InValue)
		, Count(InCount)
	{}

protected:
	GLint		*pValue;
	GLsizei		Count;
};

// integer 1 vector parameter
class FShaderParameter_Integer1v : public FShaderParameter_IntegerVector
{
public:
	FShaderParameter_Integer1v(const std::string &InName, GLint *InValue, GLsizei  InCount)
		: FShaderParameter_IntegerVector(InName, InValue, InCount)
		, SavedVal(0)
	{}

	FShaderParameter_Integer1v(const std::string &InName, GLint InValue)
		: FShaderParameter_IntegerVector(InName, &SavedVal, 1)
		, SavedVal(InValue)
	{}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLint		SavedVal;
};

// integer 2 vector parameter
class FShaderParameter_Integer2v : public FShaderParameter_IntegerVector
{
public:
	FShaderParameter_Integer2v(const std::string &InName, GLint *InValue, GLsizei InCount)
		: FShaderParameter_IntegerVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = 0;
	}

	FShaderParameter_Integer2v(const std::string &InName, GLint InVal0, GLint InVal1)
		: FShaderParameter_IntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLint	SavedVal[2];
};

// integer 3 vector parameter
class FShaderParameter_Integer3v : public FShaderParameter_IntegerVector
{
public:
	FShaderParameter_Integer3v(const std::string &InName, GLint *InValue, GLsizei InCount)
		: FShaderParameter_IntegerVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = SavedVal[2] = 0;
	}

	FShaderParameter_Integer3v(const std::string &InName, GLint InVal0, GLint InVal1, GLint InVal2)
		: FShaderParameter_IntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLint	SavedVal[3];
};

// integer 4 vector parameter
class FShaderParameter_Integer4v : public FShaderParameter_IntegerVector
{
public:
	FShaderParameter_Integer4v(const std::string &InName, GLint *InValue, GLsizei InCount)
		: FShaderParameter_IntegerVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = SavedVal[2] = SavedVal[3] = 0;
	}

	FShaderParameter_Integer4v(const std::string &InName, GLint InVal0, GLint InVal1, GLint InVal2, GLint InVal3)
		: FShaderParameter_IntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
		SavedVal[3] = InVal3;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLint	SavedVal[4];
};

//////////////////////////////////////////////////////////////////////////
// unsigned integer parameter

class FShaderParameter_UnIntegerVector : public FShaderParameter
{
public:
	FShaderParameter_UnIntegerVector(const std::string &InName, GLuint *InValue, GLsizei  InCount)
		: FShaderParameter(InName)
		, pValue(InValue)
		, Count(InCount)
	{}

protected:
	GLuint		*pValue;
	GLsizei		Count;
};

// unsigned integer 1 vector parameter
class FShaderParameter_UnInteger1v : public FShaderParameter_UnIntegerVector
{
public:
	FShaderParameter_UnInteger1v(const std::string &InName, GLuint *InValue, GLsizei  InCount)
		: FShaderParameter_UnIntegerVector(InName, InValue, InCount)
		, SavedVal(0)
	{}

	FShaderParameter_UnInteger1v(const std::string &InName, GLuint InValue)
		: FShaderParameter_UnIntegerVector(InName, &SavedVal, 1)
		, SavedVal(InValue)
	{}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLuint	SavedVal;
};

// unsigned integer 2 vector parameter
class FShaderParameter_UnInteger2v : public FShaderParameter_UnIntegerVector
{
public:
	FShaderParameter_UnInteger2v(const std::string &InName, GLuint *InValue, GLsizei InCount)
		: FShaderParameter_UnIntegerVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = 0;
	}

	FShaderParameter_UnInteger2v(const std::string &InName, GLuint InVal0, GLuint InVal1)
		: FShaderParameter_UnIntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLuint	SavedVal[2];
};

// unsigned integer 3 vector parameter
class FShaderParameter_UnInteger3v : public FShaderParameter_UnIntegerVector
{
public:
	FShaderParameter_UnInteger3v(const std::string &InName, GLuint *InValue, GLsizei InCount)
		: FShaderParameter_UnIntegerVector(InName, InValue, InCount)
	{}

	FShaderParameter_UnInteger3v(const std::string &InName, GLuint InVal0, GLuint InVal1, GLuint InVal2)
		: FShaderParameter_UnIntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLuint	SavedVal[3];
};

// unsigned integer 4 vector parameter
class FShaderParameter_UnInteger4v : public FShaderParameter_UnIntegerVector
{
public:
	FShaderParameter_UnInteger4v(const std::string &InName, GLuint *InValue, GLsizei InCount)
		: FShaderParameter_UnIntegerVector(InName, InValue, InCount)
	{}

	FShaderParameter_UnInteger4v(const std::string &InName, GLuint InVal0, GLuint InVal1, GLuint InVal2, GLuint InVal3)
		: FShaderParameter_UnIntegerVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
		SavedVal[3] = InVal3;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLuint	SavedVal[4];
};


//////////////////////////////////////////////////////////////////////////
// float parameter
class FShaderParameter_FloatVector : public FShaderParameter
{
public:
	FShaderParameter_FloatVector(const std::string &InName, GLfloat *InValue, GLsizei  InCount)
		: FShaderParameter(InName)
		, pValue(InValue)
		, Count(InCount)
	{}

protected:
	GLfloat		*pValue;
	GLsizei		Count;
};

// float 1 vector parameter
class FShaderParameter_Float1v : public FShaderParameter_FloatVector
{
public:
	FShaderParameter_Float1v(const std::string &InName, GLfloat *InValue, GLsizei  InCount)
		: FShaderParameter_FloatVector(InName, InValue, InCount)
	{}

	FShaderParameter_Float1v(const std::string &InName, GLfloat InValue)
		: FShaderParameter_FloatVector(InName, &SavedVal, 1)
		, SavedVal(InValue)
	{}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLfloat		SavedVal;
};

// float 2 vector parameter
class FShaderParameter_Float2v : public FShaderParameter_FloatVector
{
public:
	FShaderParameter_Float2v(const std::string &InName, GLfloat *InValue, GLsizei InCount)
		: FShaderParameter_FloatVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = 0.f;
	}

	FShaderParameter_Float2v(const std::string &InName, GLfloat InVal0, GLfloat InVal1)
		: FShaderParameter_FloatVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLfloat		SavedVal[2];
};

// float 3 vector parameter
class FShaderParameter_Float3v : public FShaderParameter_FloatVector
{
public:
	FShaderParameter_Float3v(const std::string &InName, GLfloat *InValue, GLsizei InCount)
		: FShaderParameter_FloatVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = SavedVal[2] = 0.f;
	}

	FShaderParameter_Float3v(const std::string &InName, GLfloat InVal0, GLfloat InVal1, GLfloat InVal2)
		: FShaderParameter_FloatVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLfloat		SavedVal[3];
};

// float 4 vector parameter
class FShaderParameter_Float4v : public FShaderParameter_FloatVector
{
public:
	FShaderParameter_Float4v(const std::string &InName, GLfloat *InValue, GLsizei InCount)
		: FShaderParameter_FloatVector(InName, InValue, InCount)
	{
		SavedVal[0] = SavedVal[1] = SavedVal[2] = SavedVal[3] = 0.f;
	}

	FShaderParameter_Float4v(const std::string &InName, GLfloat InVal0, GLfloat InVal1, GLfloat InVal2, GLfloat InVal3)
		: FShaderParameter_FloatVector(InName, SavedVal, 1)
	{
		SavedVal[0] = InVal0;
		SavedVal[1] = InVal1;
		SavedVal[2] = InVal2;
		SavedVal[3] = InVal3;
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	GLfloat		SavedVal[4];
};


// Matrix vector parameter
class FShaderParameter_MatrixVector : public FShaderParameter
{
public:
	FShaderParameter_MatrixVector(const std::string &InName, GLfloat *InValue, GLsizei InCount)
		: FShaderParameter(InName)
		, pValue(InValue)
		, Count(InCount)
	{
	}

protected:
	GLfloat		*pValue;
	GLsizei		Count;
};

// Maxtrix4f vector parameter
class FShaderParameter_Matrix4fv : public FShaderParameter_MatrixVector
{
public:
	FShaderParameter_Matrix4fv(const std::string &InName, GLfloat *InValue, GLsizei InCount)
		: FShaderParameter_MatrixVector(InName, InValue, InCount)
	{
	}

	FShaderParameter_Matrix4fv(const std::string &InName, const glm::mat4 &InMat4)
		: FShaderParameter_MatrixVector(InName, glm::value_ptr(Matrix4), 1)
		, Matrix4(InMat4)
	{
	}

	virtual void ApplyValue(FOpenGLProgram& Program) const override;

protected:
	glm::mat4	Matrix4;
};


typedef std::vector<TRefCountPtr<FShaderParameter>> FProgramParameters;

#endif // __JETX_GL_SHADERPARAMETER_H__
