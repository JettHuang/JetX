// \brief
//	 Skinning Mesh class
//

#ifndef __JETX_SCENE_SKIN_MESH_H__
#define __JETX_SCENE_SKIN_MESH_H__


#include <Common/RefCounting.h>
#include <Common/UtilityHelper.h>
#include <OpenGL/GLVertexDeclaration.h>
#include "RenderResource.h"
#include "Render.h"
#include "Mesh.h"


class FModel;

// Bones Information
struct FMeshBone
{
	FMeshBone(const std::string &InName, const glm::mat4 &InMat)
		: BoneName(InName)
		, MeshToBone(InMat)
	{}

	std::string		BoneName;
	int				BoneIdx;
	glm::mat4		MeshToBone; // vertex transform from mesh space to bone space
};

// class skinning mesh
class FSkinMesh : public FMesh
{
public:
	typedef FMesh Super;

	FSkinMesh()
		: bBoneIdxCached(false)
	{}

	FSkinMesh(const FMaterialRef& InMaterial, const FVertexBufferRef& VBuffer, const FVertexSkinBufferRef& SBuffer, const FIndexBufferRef& IBuffer, GLenum InPrimitiveMode,
		const std::vector<FMeshBone> &InMeshBones)
		: Super(InMaterial, VBuffer, IBuffer, InPrimitiveMode)
		, VertexSkinBuffer(SBuffer)
		, MeshBones(InMeshBones)
		, bBoneIdxCached(false)
	{
	}

	virtual void Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy, const FModel &InModel, const FMeshInstance &MeshInstance) override;

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

public:
	FVertexSkinBufferRef	VertexSkinBuffer;

	std::vector<FMeshBone>	MeshBones;
	std::vector<glm::mat4>	FinalMats;
	bool					bBoneIdxCached;
};

typedef TRefCountPtr<FSkinMesh>		FSkinMeshRef;

#endif // __JETX_SCENE_SKIN_MESH_H__
