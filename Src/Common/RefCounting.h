// \brief
//		base class for reference counting object
//

#ifndef __JETX_REFCOUNTING_H__
#define __JETX_REFCOUNTING_H__

#include <cassert>


class FRefCountedObject
{
public:
	FRefCountedObject() :
		nRefCount(0)
	{
	}

	virtual ~FRefCountedObject()
	{
		assert(nRefCount == 0);
	}

	int Retain()
	{
		return ++nRefCount;
	}

	int Release()
	{
		assert(nRefCount > 0);
		int NewRef = --nRefCount;
		if (NewRef == 0)
		{
			delete this;
		}

		return NewRef;
	}

	int RetainCount()
	{
		return nRefCount;
	}

protected:
	int		nRefCount;
};

// smart pointer base on ref-counting
template<typename ReferencedType>
class TRefCountPtr
{
public:
	TRefCountPtr() : Reference(nullptr)
	{
	}

	TRefCountPtr(ReferencedType *InReference, bool bRetain=true)
	{
		Reference = InReference;
		if (Reference && bRetain)
		{
			Reference->Retain();
		}
	}

	TRefCountPtr(const TRefCountPtr &InRefCountPtr)
	{
		Reference = InRefCountPtr.Reference;
		if (Reference)
		{
			Reference->Retain();
		}
	}

	~TRefCountPtr()
	{
		if (Reference)
		{
			Reference->Release();
		}
	}

	TRefCountPtr& operator =(ReferencedType* InRefObject)
	{
		ReferencedType* OldRefObject = Reference;
		
		Reference = InRefObject;
		if (Reference)
		{
			Reference->Retain();
		}
		if (OldRefObject)
		{
			OldRefObject->Release();
		}

		return *this;
	}

	TRefCountPtr& operator =(const TRefCountPtr &rhs)
	{
		return (*this = rhs.Reference);
	}

	bool operator==(const TRefCountPtr &rhs)
	{
		return (Reference == rhs.Reference);
	}

	ReferencedType* operator->() const
	{
		return Reference;
	}

	operator ReferencedType*() const
	{
		return Reference;
	}

	void SafeRelease()
	{
		*this = nullptr;
	}

	int GetReferenceCount()
	{
		if (Reference)
		{
			Reference->Retain();
			return Reference->Release();
		}

		return 0;
	}

	friend bool IsValidRef(const TRefCountPtr &InRef)
	{
		return (InRef.Reference != nullptr);
	}

private:
	ReferencedType	*Reference;
};



#endif // __JETX_REFCOUNTING_H__
