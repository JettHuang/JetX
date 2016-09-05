// \brief
//		GL Vertex Declaration is encapsulation for vertex attributes inputs
//

#ifndef __JETX_GL_VERTEX_DECLARATION_H__
#define __JETX_GL_VERTEX_DECLARATION_H__

#include <GL/glew.h>
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
	VET_MAX
};

// vertex element
struct FVertexElement
{
	unsigned char 		StreamIndex;
	unsigned char 		Offset;
	unsigned short 		AttributeIndex;	
	EVertexElementType	DataType;
	unsigned int 		Stride;
	
	FVertexElement() {}
};

// vertex declaration
class FOpenGLVertexDeclaration : public FRefCountedObject
{
public:

};


#endif // __JETX_GL_VERTEX_DECLARATION_H__
