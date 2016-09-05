// GLBuffer implementation
//
//

#include <cassert>
#include "OpenGLDrv.h"
#include "GLBuffer.h"


FOpenGLBuffer::FOpenGLBuffer(FOpenGLDrv& InOwner, GLenum InType, GLsizeiptr InSize, const GLvoid *InData, GLenum InUsage)
	: Owner(InOwner)
	, Name(0)
	, Type(InType)
	, SizeBytes(InSize)
	, Usage(InUsage)
	, bIsLocked(false)
{
	glGenBuffers(1, &Name);
	Bind();
	glBufferData(Type, SizeBytes, InData, Usage);
	Owner.CheckError(__FILE__, __LINE__);
}

FOpenGLBuffer::~FOpenGLBuffer()
{
	if (Name != 0)
	{
		Owner.OnDeleteBuffer(Type, Name);
		glDeleteBuffers(1, &Name);
	}
}

bool FOpenGLBuffer::IsValid() const
{
	return glIsBuffer(Name) == GL_TRUE;
}

void FOpenGLBuffer::Bind()
{
	Owner.CachedBindBuffer(Type, Name);
}

// Lock Buffer
// bDiscard:  discard the mapping data-content for write-only
void* FOpenGLBuffer::Lock(GLintptr InOffset, GLsizeiptr InLength, bool bReadOnly, bool bDiscard)
{
	GLbitfield Access;
	void* Data = nullptr;

	assert(!bIsLocked);

	bIsLocked = true;
	Bind();
	Access = bReadOnly ? GL_MAP_READ_BIT :
		(bDiscard ? GL_MAP_WRITE_BIT : GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

	Data = glMapBufferRange(Type, InOffset, InLength, Access);
	Owner.CheckError(__FILE__, __LINE__);
	return Data;
}

void FOpenGLBuffer::UnLock()
{
	assert(bIsLocked);
	bIsLocked = false;
	Bind();
	glUnmapBuffer(Type);
	Owner.CheckError(__FILE__, __LINE__);
}

