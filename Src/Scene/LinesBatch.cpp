// \brief
//		LinesPatch implementation
//

#include <cassert>
#include <OpenGL/OpenGLDrv.h>

#include "LinesBatch.h"


FLinesPatch::FLinesPatch(unsigned int InLinesCount)
	: VertexCount(InLinesCount << 1)
{

}

void FLinesPatch::Draw(const FViewContext &InViewContext, FRenderPolicy &InPolicy)
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

	if (!IsValidRef(VertexDeclRef))
	{
		FVertexElementsList VertexElementList;
		VertexElementList.push_back(FVertexElement(0, 0, STRUCT_VAR_OFFSET(FSimpleVertex, Position), sizeof(FSimpleVertex), VET_Float3));	// POSITION
		VertexDeclRef = GLDriver.CreateVertexDeclaration(VertexElementList);
	}

	// set up shader & parameters
	InPolicy.LinesShader->SetUp(InViewContext, *this);

	GLDriver.SetStreamSource(0, VertexBuffer);
	GLDriver.SetVertexDeclaration(VertexDeclRef);

	GLDriver.DrawArrayedPrimitive(GL_LINES, 0, VertexCount);
}

void FLinesPatch::UpdateVertex(const std::vector<FSimpleVertex> &InVertexes)
{
	if (IsValidRef(VertexBuffer))
	{
		unsigned int Count = MIN(VertexCount, InVertexes.size());
		unsigned int Bytes = Count*sizeof(FSimpleVertex);

		void* PtrDst = VertexBuffer->Lock(0, Bytes, false, false);
		memcpy_s(PtrDst, Bytes, InVertexes.data(), Bytes);
		VertexBuffer->UnLock();
	}
}

void FLinesPatch::InitRHI()
{
	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

	VertexBuffer = GLDriver.CreateVertexBuffer(VertexCount*sizeof(FSimpleVertex), nullptr, GL_DYNAMIC_DRAW);
}

void FLinesPatch::ReleaseRHI()
{
	if (IsValidRef(VertexBuffer))
	{
		VertexBuffer.SafeRelease();
	}
}
