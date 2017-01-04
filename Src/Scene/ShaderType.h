// \brief
//		Shader Type description a shader algorithm. 
//

#ifndef __JETX_SCENE_SHADERTYPE_H__
#define __JETX_SCENE_SHADERTYPE_H__

#include <string>

#include <Common/RefCounting.h>
#include <OpenGL/GLShader.h>
#include <OpenGL/GLShaderParameter.h>


struct FViewContext;
class FMesh;
class FSkinMesh;
class FLinesPatch;


// shader type
class FShaderType : public FRefCountedObject
{
public:
	FShaderType(const std::string &InVsFile, const std::string &InPsFile);

protected:
	// Set Shader Parameters
	FProgramParameters ProgramParams;

	FOpenGLVertexShaderRef VertexShaderRef;
	FOpenGLPixelShaderRef PixelShaderRef;
	FOpenGLProgramRef ProgramRef;
};

// mesh shader type
class FMeshShaderType : public FShaderType
{
public:
	FMeshShaderType(const std::string &InVsFile, const std::string &InPsFile);

	virtual void Prepare(const FViewContext &InView, const FMesh &InMesh);

	void SetUp(const FViewContext &InView, const FMesh &InMesh);
};

// skinning mesh shader type
class FSkinningMeshShaderType : public FMeshShaderType
{
public:
	typedef FMeshShaderType Super;

	FSkinningMeshShaderType(const std::string &InVsFile, const std::string &InPsFile);

	virtual void Prepare(const FViewContext &InView, const FSkinMesh &InMesh);

	void SetUp(const FViewContext &InView, const FSkinMesh &InMesh);
};

// line shader type
class FLinesShaderType : public FShaderType
{
public:
	FLinesShaderType(const std::string &InVsFile, const std::string &InPsFile);

	virtual void Prepare(const FViewContext &InView, const FLinesPatch &InLines);

	void SetUp(const FViewContext &InView, const FLinesPatch &InLines);
};

// global shader type
class FGlobalShaderType : public FShaderType
{
public:
	FGlobalShaderType(const std::string &InVsFile, const std::string &InPsFile);

	virtual void Prepare();

	void SetUp();
};

#endif // __JETX_SCENE_SHADERTYPE_H__

