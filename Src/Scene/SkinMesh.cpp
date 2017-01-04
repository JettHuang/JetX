// \brief
//		FMesh2 implementation
//

#include <cassert>
#include <OpenGL/OpenGLDrv.h>

#include "Model.h"
#include "SkinMesh.h"

void FSkinMesh::InitRHI()
{
	Super::InitRHI();
	if (IsValidRef(VertexSkinBuffer))
	{
		VertexSkinBuffer->InitRHI();
	}
}

void FSkinMesh::ReleaseRHI()
{
	if (IsValidRef(VertexSkinBuffer))
	{
		VertexSkinBuffer->ReleaseRHI();
	}
	Super::ReleaseRHI();
}

void FSkinMesh::Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy, const FModel &InModel, const FMeshInstance &MeshInstance)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

	if (!IsValidRef(VertexDeclRef))
	{
		FVertexElementsList VertexElementList;
		VertexElementList.push_back(FVertexElement(0, 0, STRUCT_VAR_OFFSET(FVertex, Position), sizeof(FVertex), VET_Float3));	// POSITION
		VertexElementList.push_back(FVertexElement(0, 1, STRUCT_VAR_OFFSET(FVertex, Normal), sizeof(FVertex), VET_Float3));		// NORMAL
		VertexElementList.push_back(FVertexElement(0, 2, STRUCT_VAR_OFFSET(FVertex, TexCoords), sizeof(FVertex), VET_Float2));	// TEX-COORD
		VertexElementList.push_back(FVertexElement(0, 3, STRUCT_VAR_OFFSET(FVertex, Tangent), sizeof(FVertex), VET_Float3));	// Tangent
		VertexElementList.push_back(FVertexElement(0, 4, STRUCT_VAR_OFFSET(FVertex, Bitangent), sizeof(FVertex), VET_Float3));	// BiTangent
		VertexElementList.push_back(FVertexElement(1, 5, STRUCT_VAR_OFFSET(FVertexSkin, Indices), sizeof(FVertexSkin), VET_UByte4));	// Bone Indices
		VertexElementList.push_back(FVertexElement(1, 6, STRUCT_VAR_OFFSET(FVertexSkin, Weights), sizeof(FVertexSkin), VET_Float4));	// Bone Weights
		VertexDeclRef = GLDriver.CreateVertexDeclaration(VertexElementList);
	}

	FNodeHierarchyRef NodeHierarchy = InModel.GetNodeHierarchy();
	assert(IsValidRef(NodeHierarchy));

	if (FinalMats.size() != MeshBones.size())
	{
		FinalMats.resize(MeshBones.size());
	}
	if (!bBoneIdxCached)
	{
		for (unsigned int Index = 0; Index < MeshBones.size(); Index++)
		{
			FMeshBone &Bone = MeshBones[Index];
			Bone.BoneIdx = NodeHierarchy->GetNodeIndex(Bone.BoneName);
		
			assert(Bone.BoneIdx != NODE_INDEX_NONE);
		} // end for
		
		bBoneIdxCached = true;
	}

	for (unsigned int Index = 0; Index < MeshBones.size(); Index++)
	{
		const FMeshBone &Bone = MeshBones[Index];
		assert(Bone.BoneIdx != NODE_INDEX_NONE);
		const FNode &SkeletonNode = NodeHierarchy->GetNode(Bone.BoneIdx);

		FinalMats[Index] = SkeletonNode.ModelMat * Bone.MeshToBone;
	}

#if 0
	// CPU SKIN
	FVertex* pVertexs = (FVertex*) VertexBuffer->Buffer->Lock(0, VertexBuffer->Vertexes.size() * sizeof(FVertex), false, false);
	for (unsigned int k = 0; k < VertexSkinBuffer->Vertexes.size(); k++)
	{
		const FVertexSkin &SkinInfo = VertexSkinBuffer->Vertexes[k];
		const glm::vec4 localPos = glm::vec4(VertexBuffer->Vertexes[k].Position, 1.f);

		glm::vec4 ObjPos;
		ObjPos = FinalMats[SkinInfo.Indices[0]] * localPos * SkinInfo.Weights[0];
		ObjPos += FinalMats[SkinInfo.Indices[1]] * localPos * SkinInfo.Weights[1];
		ObjPos += FinalMats[SkinInfo.Indices[2]] * localPos * SkinInfo.Weights[2];
		ObjPos += FinalMats[SkinInfo.Indices[3]] * localPos * SkinInfo.Weights[3];

		pVertexs[k].Position = glm::vec3(ObjPos.x, ObjPos.y, ObjPos.z);
	}
	VertexBuffer->Buffer->UnLock();

	// set up shader & parameters
	InPolicy.MeshShader->SetUp(InViewContext, *this);
#else
	// GPU SKIN
	// set up shader & parameters
	InPolicy.SkinMeshShader->SetUp(InViewContext, *this);
#endif

	GLDriver.SetStreamSource(0, VertexBuffer->GetRHIBuffer());
	GLDriver.SetStreamSource(1, VertexSkinBuffer->GetRHIBuffer());
	GLDriver.SetVertexDeclaration(VertexDeclRef);

	GLDriver.DrawIndexedPrimitive(IndexBuffer->GetRHIBuffer(), PrimitiveMode, 0, IndexBuffer->GetElementCount());
}
