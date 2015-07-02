#include "pch.h"
#include "jnc_Box.h"
#include "jnc_ClosureClassType.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

void 
Box::gcMarkData (Runtime* runtime)
{
	m_root->m_flags |= BoxFlag_GcWeakMark;

	if (m_flags & BoxFlag_GcRootsAdded)
		return;

	m_flags |= BoxFlag_GcRootsAdded;

	if (!(m_type->getFlags () & TypeFlag_GcRoot))
		return;

	if (!(m_flags & BoxFlag_DynamicArray))
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
Box::gcWeakMarkObject ()
{
	if (m_flags & BoxFlag_GcWeakMark)
		return;

	m_flags |= BoxFlag_GcWeakMark;
	m_root->m_flags |= BoxFlag_GcWeakMark;
}

void 
Box::gcMarkObject (Runtime* runtime)
{
	if (m_flags & BoxFlag_GcMark)
		return;

	m_flags |= BoxFlag_GcMark | BoxFlag_GcWeakMark;
	m_root->m_flags |= BoxFlag_GcWeakMark;

	if (m_type->getTypeKind () == TypeKind_Class)
		gcMarkClassMemberFields (runtime);

	if (m_flags & BoxFlag_GcRootsAdded)
		return;

	m_flags |= BoxFlag_GcRootsAdded;

	if (!(m_type->getFlags () & TypeFlag_GcRoot))
		return;

	runtime->addGcRoot (this, m_type);
}

void 
Box::gcMarkClassMemberFields (Runtime* runtime)
{
	ASSERT (m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) m_type;
	rtl::Array <StructField*> classMemberFieldArray = classType->getClassMemberFieldArray ();
	size_t count = classMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = classMemberFieldArray [i];
		Box* childBox = (Box*) ((char*) (this + 1) + field->getOffset ());
		ASSERT (childBox->m_type == field->getType ());

		if (childBox->m_flags & BoxFlag_GcMark)
			continue;

		childBox->m_flags |= BoxFlag_GcMark | BoxFlag_GcWeakMark;
		childBox->gcMarkClassMemberFields (runtime);
	}
}

void
Box::gcWeakMarkClosureObject (Runtime* runtime)
{
	m_root->m_flags |= BoxFlag_GcWeakMark;
	m_flags |= BoxFlag_GcMark;

	if (m_flags & (BoxFlag_GcWeakMark_c | BoxFlag_GcRootsAdded))
		return;

	m_flags |= BoxFlag_GcWeakMark_c;

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
		ASSERT (type->getFlags () & TypeFlag_GcRoot);		

		if (field->getFlags () & StructFieldFlag_WeakMasked)
			type = getWeakPtrType (type);

		type->gcMark (runtime, p + field->getOffset ());
	}
}

//.............................................................................

} // namespace jnc {
