// \brief
//		Rendering The Scene
//

#ifndef __JETX_SCENE_RENDER_H__
#define __JETX_SCENE_RENDER_H__

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <OpenGL/OpenGLDrv.h>
#include "ShaderType.h"


// class view-context
struct FViewContext
{
	glm::uvec4	viewport;
	glm::mat4	view;
	glm::mat4	model;
	glm::mat4	projection;
};

// class render-policy
struct FRenderPolicy
{
	TRefCountPtr<FMeshShaderType>			MeshShader;
	TRefCountPtr<FSkinningMeshShaderType>	SkinMeshShader;
	TRefCountPtr<FLinesShaderType>			LinesShader;
};

// draw full screen quad
class FDrawFullQuadHelper
{
public:
	FDrawFullQuadHelper()
	{
		// quad
		static GLfloat quadVerts[] = {
			0.f, 0.f, 0.f, 0.f,  // v0, t0
			1.f, 0.f, 1.f, 0.f,  // v1, t1
			1.f, 1.f, 1.f, 1.f,  // v2, t2
			0.f, 1.f, 0.f, 1.f   // v3, t3
		};
		static GLuint quadIndices[] = {
			0, 1, 2,   // triangle 0
			0, 2, 3    // triangle 1
		};

		FVertexElementsList quadElements;
		quadElements.push_back(FVertexElement(0, 0, 0, 4 * sizeof(GLfloat), VET_Float2));
		quadElements.push_back(FVertexElement(0, 1, 2 * sizeof(GLfloat), 4 * sizeof(GLfloat), VET_Float2));

		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
		quadVertBuffer = GLDriver.CreateVertexBuffer(sizeof(quadVerts), quadVerts);
		quadIndexBuffer = GLDriver.CreateIndexBuffer(sizeof(quadIndices), quadIndices, sizeof(quadIndices[0]));
		quadVertDecl = GLDriver.CreateVertexDeclaration(quadElements);
	}

	void Draw(const TRefCountPtr<FGlobalShaderType> &InShader, const FOpenGLTexture2DRef &InTexture)
	{
		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

		InShader->SetUp();

		GLDriver.SetTexture2D(0, InTexture);
		GLDriver.SetStreamSource(0, quadVertBuffer);
		GLDriver.SetVertexDeclaration(quadVertDecl);
		GLDriver.DrawIndexedPrimitive(quadIndexBuffer, GL_TRIANGLES, 0, 6);
	}

protected:
	FOpenGLVertexBufferRef  quadVertBuffer;
	FOpenGLIndexBufferRef   quadIndexBuffer;
	FOpenGLVertexDeclarationRef quadVertDecl;
};

// class renderer
class FRenderer
{
public:

};

#endif // __JETX_SCENE_RENDER_H__
