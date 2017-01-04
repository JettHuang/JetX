// \brief
//		implementation of Shader-Type
//

#include <iostream>
#include <Common/UtilityHelper.h>
#include <OpenGL/OpenGLDrv.h>

#include "Render.h"
#include "Scene.h"
#include "Mesh.h"
#include "SkinMesh.h"
#include "ShaderType.h"



FShaderType::FShaderType(const std::string &InVsFile, const std::string &InPsFile)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

	// Load Shader
	std::string  vShaderCode = FUtilityHelper::ReadTextFile(InVsFile.c_str());
	std::string  pShaderCode = FUtilityHelper::ReadTextFile(InPsFile.c_str());

	VertexShaderRef = GLDriver.CreateVertexShader(vShaderCode.c_str());
	PixelShaderRef = GLDriver.CreatePixelShader(pShaderCode.c_str());
	ProgramRef = GLDriver.CreateProgram(VertexShaderRef, PixelShaderRef);

	std::cout << "======== Load Shader: " << InVsFile << ", " << InPsFile << std::endl;
	VertexShaderRef->DumpDebugInfo();
	PixelShaderRef->DumpDebugInfo();
	ProgramRef->DumpDebugInfo();
}

FMeshShaderType::FMeshShaderType(const std::string &InVsFile, const std::string &InPsFile)
	: FShaderType(InVsFile, InPsFile)
{

}

void FMeshShaderType::Prepare(const FViewContext &InView, const FMesh &InMesh)
{
	ProgramParams.clear();

	ProgramParams.push_back(new FShaderParameter_Matrix4fv("model", InView.model));
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("view", InView.view));
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("projection", InView.projection));
	ProgramParams.push_back(new FShaderParameter_Integer1v("diffuseTex", 0));
	ProgramParams.push_back(new FShaderParameter_Integer1v("specularTex", 1));
	ProgramParams.push_back(new FShaderParameter_Integer1v("normalTex", 2));

	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

	FMaterialRef Material = InMesh.Material;
	GLDriver.SetTexture2D(0, IsValidRef(Material->TexDiffuse) ? Material->TexDiffuse->GetRHITexture() : nullptr);
	GLDriver.SetTexture2D(1, IsValidRef(Material->TexSpecular) ? Material->TexSpecular->GetRHITexture() : nullptr);
	GLDriver.SetTexture2D(2, IsValidRef(Material->TexNormal) ? Material->TexNormal->GetRHITexture() : nullptr);
}

void FMeshShaderType::SetUp(const FViewContext &InView, const FMesh &InMesh)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
	Prepare(InView, InMesh);

	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetShaderProgramParameters(&ProgramParams);
}

//////////////////////////////////////////////////////////////////////////

FSkinningMeshShaderType::FSkinningMeshShaderType(const std::string &InVsFile, const std::string &InPsFile)
	: FMeshShaderType(InVsFile, InPsFile)
{

}

void FSkinningMeshShaderType::Prepare(const FViewContext &InView, const FSkinMesh &InMesh)
{
	Super::Prepare(InView, InMesh);

	// Set Bone Matrix Uniform
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("gBones[0]", (float*)(InMesh.FinalMats.data()), InMesh.FinalMats.size()));
}

void FSkinningMeshShaderType::SetUp(const FViewContext &InView, const FSkinMesh &InMesh)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
	Prepare(InView, InMesh);

	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetShaderProgramParameters(&ProgramParams);
}

//////////////////////////////////////////////////////////////////////////

FLinesShaderType::FLinesShaderType(const std::string &InVsFile, const std::string &InPsFile)
	: FShaderType(InVsFile, InPsFile)
{

}

void FLinesShaderType::Prepare(const FViewContext &InView, const FLinesPatch &InLines)
{
	ProgramParams.clear();

	ProgramParams.push_back(new FShaderParameter_Matrix4fv("model", InView.model));
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("view", InView.view));
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("projection", InView.projection));
}

void FLinesShaderType::SetUp(const FViewContext &InView, const FLinesPatch &InLines)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
	Prepare(InView, InLines);

	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetShaderProgramParameters(&ProgramParams);
}

//////////////////////////////////////////////////////////////////////////

FGlobalShaderType::FGlobalShaderType(const std::string &InVsFile, const std::string &InPsFile)
	: FShaderType(InVsFile, InPsFile)
{

}

void FGlobalShaderType::Prepare()
{
	ProgramParams.clear();

	glm::mat4 MVP = glm::ortho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
	ProgramParams.push_back(new FShaderParameter_Matrix4fv("mvp", MVP));
	ProgramParams.push_back(new FShaderParameter_Integer1v("quadTex", 0));
}

void FGlobalShaderType::SetUp()
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
	Prepare();

	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetShaderProgramParameters(&ProgramParams);
}

