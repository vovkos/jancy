#include "pch.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

void
primeIface (
	ClassType* type,
	IfaceHdr* self,
	void* vtable,
	Box* object,
	Box* root,
	uintptr_t flags
	)
{
	self->m_vtable = vtable;
	self->m_box = object;

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
			object,
			root,
			flags
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
			root,
			flags
			);
	}
}

void
prime (
	ClassType* type,
	void* vtable,
	Box* box,
	Box* root,
	uintptr_t flags
	)
{
	memset (box, 0, type->getSize ());

	box->m_root = root;
	box->m_type = type;
	box->m_flags = flags;
	box->m_elementCount = 1;

	primeIface (type, (IfaceHdr*) (box + 1), vtable, box, root, flags);
}

//.............................................................................

} // namespace jnc {
