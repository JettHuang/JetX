// \brief
//		implementation of shader parameter class
//

#include <iostream>
#include "GLShaderParameter.h"


void FShaderParameter_Integer1v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform1iv(Location, Count, pValue);
	}
}

void FShaderParameter_Integer2v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform2iv(Location, Count, pValue);
	}
}

void FShaderParameter_Integer3v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform3iv(Location, Count, pValue);
	}
}

void FShaderParameter_Integer4v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform4iv(Location, Count, pValue);
	}
}

void FShaderParameter_UnInteger1v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform1uiv(Location, Count, pValue);
	}
}

void FShaderParameter_UnInteger2v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform2uiv(Location, Count, pValue);
	}
}

void FShaderParameter_UnInteger3v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform3uiv(Location, Count, pValue);
	}
}

void FShaderParameter_UnInteger4v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform4uiv(Location, Count, pValue);
	}
}

void FShaderParameter_Float1v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform1fv(Location, Count, pValue);
	}
}

void FShaderParameter_Float2v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform2fv(Location, Count, pValue);
	}
}

void FShaderParameter_Float3v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform3fv(Location, Count, pValue);
	}
}

void FShaderParameter_Float4v::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniform4fv(Location, Count, pValue);
	}
}

void FShaderParameter_Matrix4fv::ApplyValue(FOpenGLProgram& Program) const
{
	GLint Location = Program.GetParamLocation(Name);
	if (Location >= 0)
	{
		glUniformMatrix4fv(Location, Count, GL_FALSE, pValue);
	}
}
