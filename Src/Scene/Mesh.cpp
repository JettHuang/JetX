// \brief
//		FMesh implementation
//

#include <cassert>
#include <OpenGL/OpenGLDrv.h>

#include "Mesh.h"
#include "Model.h"


void FMesh::InitRHI()
{
	if (IsValidRef(Material))
	{
		Material->InitRHI();
	}
	if (IsValidRef(VertexBuffer))
	{
		VertexBuffer->InitRHI();
	}
	if (IsValidRef(IndexBuffer))
	{
		IndexBuffer->InitRHI();
	}
}

void FMesh::ReleaseRHI()
{
	if (IsValidRef(Material))
	{
		Material->ReleaseRHI();
	}
	if (IsValidRef(VertexBuffer))
	{
		VertexBuffer->ReleaseRHI();
	}
	if (IsValidRef(IndexBuffer))
	{
		IndexBuffer->ReleaseRHI();
	}
}

void FMesh::Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy, const FModel &InModel, const FMeshInstance &MeshInstance)
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

		VertexDeclRef = GLDriver.CreateVertexDeclaration(VertexElementList);
	}

	FViewContext LocalViewContext(InViewContext);
	if (MeshInstance.NodeIdx != NODE_INDEX_NONE)
	{
		FNodeHierarchyRef NodeHierarchy = InModel.GetNodeHierarchy();
		assert(IsValidRef(NodeHierarchy));

		glm::mat4 ModelTrans = NodeHierarchy->GetNode(MeshInstance.NodeIdx).ModelMat;
		LocalViewContext.model = LocalViewContext.model * ModelTrans;
	}

	// set up shader & parameters
	InPolicy.MeshShader->SetUp(LocalViewContext, *this);

	GLDriver.SetStreamSource(0, VertexBuffer->GetRHIBuffer());
	GLDriver.SetVertexDeclaration(VertexDeclRef);

	GLDriver.DrawIndexedPrimitive(IndexBuffer->GetRHIBuffer(), PrimitiveMode, 0, IndexBuffer->GetElementCount());
}
