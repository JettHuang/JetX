// \brief
//		Mesh class
//

#ifndef __JETX_SCENE_MESH_H__
#define __JETX_SCENE_MESH_H__


#include <Common/RefCounting.h>
#include <Common/UtilityHelper.h>
#include <OpenGL/GLVertexDeclaration.h>
#include "RenderResource.h"
#include "Render.h"


class FModel;
struct FMeshInstance;

// class mesh
class FMesh : public FRefCountedObject
{
public:
	FMesh() 
		: PrimitiveMode(GL_TRIANGLES)
	{}

	FMesh(const FMaterialRef& InMaterial, const FVertexBufferRef& VBuffer, const FIndexBufferRef& IBuffer, GLenum InPrimitiveMode)
		: Material(InMaterial)
		, VertexBuffer(VBuffer)
		, IndexBuffer(IBuffer)
		, PrimitiveMode(InPrimitiveMode)
	{
	}

	virtual void Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy, const FModel &InModel, const FMeshInstance &MeshInstance);

	virtual void InitRHI();
	virtual void ReleaseRHI();
public:
	FOpenGLVertexDeclarationRef		VertexDeclRef;

	FMaterialRef		Material;
	FVertexBufferRef	VertexBuffer;
	FIndexBufferRef		IndexBuffer;
	GLenum				PrimitiveMode;
};

typedef TRefCountPtr<FMesh>		FMeshRef;


#endif // __JETX_SCENE_MESH_H__
