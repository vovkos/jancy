#include "pch.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

void
primeIface (
	ClassType* type,
	IfaceHdr* self,
	void* vtable,
	Box* box,
	Box* root
	)
{
	self->m_vtable = vtable;
	self->m_box = box;

	// prime all the base types

	rtl::Array <BaseTypeSlot*> baseTypePrimeArray = type->getBaseTypePrimeArray ();
	size_t count = baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = baseTypePrimeArray [i];
		ASSERT (slot->getType ()->getTypeKind () == TypeKind_Class);

		primeIface (
			(ClassType*) slot->getType (),
			(IfaceHdr*) ((char*) self + slot->getOffset ()),
			vtable ? (void**) vtable + slot->getVTableIndex () : NULL,
			box,
			root
			);
	}

	// prime all the class fields

	rtl::Array <StructField*> fieldPrimeArray = type->getClassMemberFieldArray ();
	count = fieldPrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = fieldPrimeArray [i];
		ASSERT (field->getType ()->getTypeKind () == TypeKind_Class);

		ClassType* fieldType = (ClassType*) field->getType ();
		Box* fieldBox = (Box*) ((char*) self + field->getOffset ());
		void* fieldVTable = NULL; // pFieldType->GetVTablePtrValue ()

		prime (
			fieldType, 
			fieldVTable,
			fieldBox,
			root
			);
	}
}

void
prime (
	ClassType* type,
	void* vtable,
	Box* box,
	Box* root
	)
{
	ASSERT (root <= box);

	memset (box, 0, type->getSize ());

	box->m_type = type;
	box->m_flags = BoxFlag_StrongMark | BoxFlag_WeakMark;
	box->m_rootOffset = (char*) root - (char*) box;

	primeIface (type, (IfaceHdr*) (box + 1), vtable, box, root);
}

//.............................................................................

} // namespace jnc {
