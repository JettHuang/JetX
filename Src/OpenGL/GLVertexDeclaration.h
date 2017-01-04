// \brief
//		GL Vertex Declaration is encapsulation for vertex attributes inputs
//

#ifndef __JETX_GL_VERTEX_DECLARATION_H__
#define __JETX_GL_VERTEX_DECLARATION_H__

#include <GL/glew.h>
#include <vector>
#include <Common/RefCounting.h>


enum EVertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_PackedNormal,	// FPackedNormal
	VET_UByte4,
	VET_UByte4N,
	VET_Color,
	VET_Short2,
	VET_Short4,
	VET_Short2N,		// 16 bit word normalized to (value/32767.0,value/32767.0,0,0,1)
	VET_Half2,			// 16 bit float using 1 bit sign, 5 bit exponent, 10 bit mantissa 
	VET_Half4,
	VET_Short4N,		// 4 X 16 bit word, normalized 
	VET_UShort2,
	VET_UShort4,
	VET_UShort2N,		// 16 bit word normalized to (value/65535.0,value/65535.0,0,0,1)
	VET_UShort4N,		// 4 X 16 bit word unsigned, normalized 
	VET_URGB10A2N,		// 10 bit r, g, b and 2 bit a normalized to (value/1023.0f, value/1023.0f, value/1023.0f, value/3.0f)
	VET_Int1,
	VET_Int2,
	VET_Int3,
	VET_Int4,
	VET_MAX
};

// vertex element
struct FVertexElement
{
	unsigned short 		StreamIndex;
	unsigned short 		AttributeIndex;
	unsigned short 		Offset;
	unsigned short 		Stride;
	EVertexElementType	DataType;
	
	FVertexElement() {}
	FVertexElement(unsigned short InStreamIndex, unsigned short InAttriIndex, unsigned short InOffset, unsigned short InStride, EVertexElementType InType)
		: StreamIndex(InStreamIndex)
		, AttributeIndex(InAttriIndex)
		, Offset(InOffset)
		, Stride(InStride)
		, DataType(InType)
	{
	}
};

typedef std::vector<FVertexElement>		FVertexElementsList;


// GL vertex element
struct FOpenGLVertexElement
{
	GLuint		StreamIndex;
	GLuint		AttributeIndex;
	GLint		Size;
	GLenum		Type;
	GLsizei		Stride;
	GLuint		Offset;
	GLboolean	Normalized;
	GLboolean	ShouldConvertToFloat;

	FOpenGLVertexElement() {}

	FOpenGLVertexElement(GLuint InStreamIndex, GLuint InAttributeIndex, GLint InSize, GLenum InType, GLsizei InStride, GLuint InOffset, GLboolean InNormalized,
		GLboolean InShouldConvertToFloat)
		: StreamIndex(InStreamIndex)
		, AttributeIndex(InAttributeIndex)
		, Size(InSize)
		, Type(InType)
		, Stride(InStride)
		, Offset(InOffset)
		, Normalized(InNormalized)
		, ShouldConvertToFloat(InShouldConvertToFloat)
	{}

	void SetValue(GLuint InStreamIndex, GLuint InAttributeIndex, GLint InSize, GLenum InType, GLsizei InStride, GLuint InOffset, GLboolean InNormalized, GLboolean InShouldConvertToFloat)
	{
		StreamIndex = InStreamIndex;
		AttributeIndex = InAttributeIndex;
		Size = InSize;
		Type = InType;
		Stride = InStride;
		Offset = InOffset;
		Normalized = InNormalized;
		ShouldConvertToFloat = InShouldConvertToFloat;
	}
};

typedef std::vector<FOpenGLVertexElement>  FOpenGLVertexElementsList;

// vertex declaration
class FOpenGLVertexDeclaration : public FRefCountedObject
{
public:
	FOpenGLVertexDeclaration(const FVertexElementsList &InVertexElements);

	FOpenGLVertexElementsList	GLVertexElements;
};

typedef TRefCountPtr<FOpenGLVertexDeclaration>		FOpenGLVertexDeclarationRef;


#endif // __JETX_GL_VERTEX_DECLARATION_H__
