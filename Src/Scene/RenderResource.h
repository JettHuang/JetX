// \brief
//		resource on the engine level
//

#ifndef __JETX_SCENE_RENDERRESOURCE_H__
#define __JETX_SCENE_RENDERRESOURCE_H__

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Common/RefCounting.h>
#include <OpenGL/GLBuffer.h>
#include <OpenGL/GLTexture.h>


// Base Resource
class FRenderResource : public FRefCountedObject
{
public:
	FRenderResource() : bInitialized(false)
	{}
	virtual ~FRenderResource()
	{}

	virtual void InitRHI() = 0;
	virtual void ReleaseRHI() = 0;

protected:
	bool	bInitialized;
};

// simple vertex format
struct FSimpleVertex
{
	glm::vec3	Position; // Position
};

// vertex format
struct FVertex
{
	glm::vec3 Position;		// Position
	glm::vec3 Normal;		// Normal
	glm::vec2 TexCoords; 	// TexCoords
	glm::vec3 Tangent; 		// Tangent
	glm::vec3 Bitangent; 	// Bitangent
};

struct FVertexSkin
{
	FVertexSkin()
	{
		Indices[0] = Indices[1] = Indices[2] = Indices[3] = 0;
		Weights[0] = Weights[1] = Weights[2] = Weights[3] = 0.f;
	}

	unsigned char Indices[4]; // bone indices
	float		  Weights[4]; // bone weights
};

// Vertex Buffer
class FVertexBuffer : public FRenderResource
{
public:
	FVertexBuffer() {}
	virtual ~FVertexBuffer() {}

	void FillBuffer(const std::vector<FVertex> &InVertexes);

	void InitRHI() override;
	void ReleaseRHI() override;

	FOpenGLVertexBufferRef GetRHIBuffer() { return Buffer; }

public:
	std::vector<FVertex>	Vertexes;
	FOpenGLVertexBufferRef	Buffer;
};

typedef TRefCountPtr<FVertexBuffer>		FVertexBufferRef;

// Vertex Skin Blend Buffer
class FVertexSkinBuffer : public FRenderResource
{
public:
	FVertexSkinBuffer() {}
	virtual ~FVertexSkinBuffer() {}

	void FillBuffer(const std::vector<FVertexSkin> &InVertexes);

	void InitRHI() override;
	void ReleaseRHI() override;

	FOpenGLVertexBufferRef GetRHIBuffer() { return Buffer; }

public:
	std::vector<FVertexSkin>	Vertexes;
	FOpenGLVertexBufferRef	Buffer;
};

typedef TRefCountPtr<FVertexSkinBuffer>	FVertexSkinBufferRef;

// Indices Buffer
class FIndexBuffer : public FRenderResource
{
public:
	FIndexBuffer() {}
	virtual ~FIndexBuffer() {}

	void FillBuffer(const std::vector<GLuint> &InIndexes);

	void InitRHI() override;
	void ReleaseRHI() override;

	FOpenGLIndexBufferRef GetRHIBuffer() { return Buffer; }
	GLuint GetElementCount() { return Indices.size(); }

protected:
	std::vector<GLuint>		Indices;
	FOpenGLIndexBufferRef	Buffer;
};

typedef TRefCountPtr<FIndexBuffer>		FIndexBufferRef;

// Texture
class FTexture2D;
typedef TRefCountPtr<FTexture2D>		FTexture2DRef;

class FTexture2D : public FRenderResource
{
public:
	FTexture2D();
	virtual ~FTexture2D();

	static FTexture2DRef CreateTexture(const std::string &InFilename);
	void LoadFromFile(const std::string &InFilename);

	void InitRHI() override;
	void ReleaseRHI() override;

	FOpenGLTexture2DRef GetRHITexture() { return Tex2D; }

protected:
	void SafeReleaseData();

protected:
	int Width, Height;
	unsigned char* ImageData;
	FOpenGLTexture2DRef Tex2D;
};


// Material
class FMaterial : public FRenderResource
{
public:
	FMaterial()
	{}

	virtual ~FMaterial()
	{}

	void InitRHI() override;
	void ReleaseRHI() override;

public:
	FTexture2DRef		TexDiffuse;
	FTexture2DRef		TexSpecular;
	FTexture2DRef		TexNormal;
};

typedef TRefCountPtr<FMaterial>		FMaterialRef;


#endif // __JETX_SCENE_RENDERRESOURCE_H__
