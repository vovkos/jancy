#include "pch.h"
#include "jnc_ObjHdr.h"
#include "jnc_ClosureClassType.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

void 
ObjHdr::gcMarkData (Runtime* runtime)
{
	m_root->m_flags |= ObjHdrFlagKind_GcWeakMark;

	if (m_flags & ObjHdrFlagKind_GcRootsAdded)
		return;

	m_flags |= ObjHdrFlagKind_GcRootsAdded;

	if (!(m_type->getFlags () & TypeFlagKind_GcRoot))
		return;

	if (!(m_flags & ObjHdrFlagKind_DynamicArray))
	{
		if (m_type->getTypeKind () == TypeKind_Class)
			runtime->addGcRoot (this, m_type);
		else
			runtime->addGcRoot (this + 1, m_type);
	}
	else
	{
		ASSERT (m_type->getTypeKind () != TypeKind_Class);

		char* p = (char*) (this + 1);		
		size_t count = *((size_t*) this - 1);
		for (size_t i = 0; i < count; i++)
		{
			runtime->addGcRoot (p, m_type);
			p += m_type->getSize  ();
		}
	}
}

void 
ObjHdr::gcMarkObject (Runtime* runtime)
{
	m_root->m_flags |= ObjHdrFlagKind_GcWeakMark;
	m_flags |= ObjHdrFlagKind_GcMark;

	if (m_flags & ObjHdrFlagKind_GcRootsAdded)
		return;

	m_flags |= ObjHdrFlagKind_GcRootsAdded;

	if (!(m_type->getFlags () & TypeFlagKind_GcRoot))
		return;

	runtime->addGcRoot (this, m_type);
}

void
ObjHdr::gcWeakMarkClosureObject (Runtime* runtime)
{
	m_root->m_flags |= ObjHdrFlagKind_GcWeakMark;
	m_flags |= ObjHdrFlagKind_GcMark;

	if (m_flags & (ObjHdrFlagKind_GcWeakMark_c | ObjHdrFlagKind_GcRootsAdded))
		return;

	m_flags |= ObjHdrFlagKind_GcWeakMark_c;

	ClosureClassType* closureClassType = (ClosureClassType*) m_classType;
	if (!closureClassType->getWeakMask ())
	{
		gcMarkObject (runtime);
		return;
	}

	char* p = (char*) (this + 1);

	rtl::Array <StructField*> gcRootMemberFieldArray = closureClassType->getGcRootMemberFieldArray ();
	size_t count = gcRootMemberFieldArray.getCount ();

	for (size_t i = 0; i < count; i++)
	{
		StructField* field = gcRootMemberFieldArray [i];
		Type* type = field->getType ();
		ASSERT (type->getFlags () & TypeFlagKind_GcRoot);		

		if (field->getFlags () & StructFieldFlagKind_WeakMasked)
			type = getWeakPtrType (type);

		type->gcMark (runtime, p + field->getOffset ());
	}
}

//.............................................................................

} // namespace jnc {
