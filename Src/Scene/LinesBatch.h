// \brief
//		Lines patch class
//

#ifndef __JETX_SCENE_LINESPATCH_H__
#define __JETX_SCENE_LINESPATCH_H__


#include <Common/RefCounting.h>
#include <Common/UtilityHelper.h>
#include <OpenGL/GLVertexDeclaration.h>
#include "RenderResource.h"
#include "Render.h"


// class lines patch
class FLinesPatch : public FRefCountedObject
{
public:
	FLinesPatch(unsigned int InLinesCount);

	virtual void Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy);

	void UpdateVertex(const std::vector<FSimpleVertex> &InVertexes);

	virtual void InitRHI();
	virtual void ReleaseRHI();
public:
	FOpenGLVertexDeclarationRef		VertexDeclRef;
	FOpenGLVertexBufferRef			VertexBuffer;

	unsigned int	VertexCount;
};
typedef TRefCountPtr<FLinesPatch>		FLinesPatchRef;


#endif // __JETX_SCENE_LINESPATCH_H__
