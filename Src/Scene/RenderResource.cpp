// \brief
//		implementation of render-resource class
//

#include <cassert>
#include <iostream>

#include <SOIL.h>

#include <OpenGL/OpenGLDrv.h>
#include "RenderResource.h"

//////////////////////////////////////////////////////////////////////////

void FVertexBuffer::FillBuffer(const std::vector<FVertex> &InVertexes)
{
	Vertexes = InVertexes;
}

void FVertexBuffer::InitRHI()
{
	if (!bInitialized)
	{
		Buffer = FOpenGLDrv::SharedInstance().CreateVertexBuffer(Vertexes.size()*sizeof(FVertex), Vertexes.data());
		bInitialized = true;
	}
}

void FVertexBuffer::ReleaseRHI()
{
	if (bInitialized)
	{
		Buffer.SafeRelease();
		bInitialized = false;
	}
}

//////////////////////////////////////////////////////////////////////////

void FVertexSkinBuffer::FillBuffer(const std::vector<FVertexSkin> &InVertexes)
{
	Vertexes = InVertexes;
}

void FVertexSkinBuffer::InitRHI()
{
	if (!bInitialized)
	{
		Buffer = FOpenGLDrv::SharedInstance().CreateVertexBuffer(Vertexes.size()*sizeof(FVertexSkin), Vertexes.data());
		bInitialized = true;
	}
}

void FVertexSkinBuffer::ReleaseRHI()
{
	if (bInitialized)
	{
		Buffer.SafeRelease();
		bInitialized = false;
	}
}

//////////////////////////////////////////////////////////////////////////

void FIndexBuffer::FillBuffer(const std::vector<GLuint> &InIndices)
{
	Indices = InIndices;
}

void FIndexBuffer::InitRHI()
{
	if (!bInitialized)
	{
		Buffer = FOpenGLDrv::SharedInstance().CreateIndexBuffer(Indices.size()*sizeof(GLint), Indices.data(), sizeof(GLuint));
		bInitialized = true;
	}
}

void FIndexBuffer::ReleaseRHI()
{
	if (bInitialized)
	{
		Buffer.SafeRelease();
		bInitialized = false;
	}
}

//////////////////////////////////////////////////////////////////////////

FTexture2D::FTexture2D()
	: Width(0)
	, Height(0)
	, ImageData(nullptr)
{

}

FTexture2D::~FTexture2D()
{
	SafeReleaseData();
}

void FTexture2D::SafeReleaseData()
{
	if (ImageData)
	{
		SOIL_free_image_data(ImageData); ImageData = nullptr;
	}
}

FTexture2DRef FTexture2D::CreateTexture(const std::string &InFilename)
{
	FTexture2D *NewTex2D = new FTexture2D();
	NewTex2D->LoadFromFile(InFilename);

	return NewTex2D;
}

void FTexture2D::LoadFromFile(const std::string &InFilename)
{
	SafeReleaseData();
	ImageData = SOIL_load_image(InFilename.c_str(), &Width, &Height, nullptr, SOIL_LOAD_RGB);
	if (!ImageData)
	{
		std::cout << "FTexture2D::LoadFromFile Failed: " << InFilename.c_str() << std::endl;
	}
}

void FTexture2D::InitRHI()
{
	if (!bInitialized)
	{
		assert(ImageData);
		Tex2D = FOpenGLDrv::SharedInstance().CreateTexture2D(GL_RGB, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, ImageData);
		bInitialized = true;
	}
}

void FTexture2D::ReleaseRHI()
{
	if (bInitialized)
	{
		Tex2D.SafeRelease();
		bInitialized = false;
	}
}

void FMaterial::InitRHI()
{
	if (IsValidRef(TexDiffuse))
	{
		TexDiffuse->InitRHI();
	}
	if (IsValidRef(TexSpecular))
	{
		TexSpecular->InitRHI();
	}
	if (IsValidRef(TexNormal))
	{
		TexNormal->InitRHI();
	}
}

void FMaterial::ReleaseRHI()
{
	if (IsValidRef(TexDiffuse))
	{
		TexDiffuse->ReleaseRHI();
	}
	if (IsValidRef(TexSpecular))
	{
		TexSpecular->ReleaseRHI();
	}
	if (IsValidRef(TexNormal))
	{
		TexNormal->ReleaseRHI();
	}
}
