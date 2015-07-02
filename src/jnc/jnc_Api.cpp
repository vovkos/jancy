#include "pch.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

void
primeInterface (
	ClassType* type,
	IfaceHdr* self,
	void* vtable,
	Box* object,
	Box* root,
	uintptr_t flags
	)
{
	self->m_vtable = vtable;
	self->m_object = object;

	// prime all the base types

	rtl::Array <BaseTypeSlot*> baseTypePrimeArray = type->getBaseTypePrimeArray ();
	size_t count = baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = baseTypePrimeArray [i];
		ASSERT (slot->getType ()->getTypeKind () == TypeKind_Class);

		primeInterface (
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
	Box* object,
	Box* root,
	uintptr_t flags
	)
{
//	memset (pObject, 0, pType->GetSize ());

	object->m_root = root;
	object->m_type = type;
	object->m_flags = flags;

	primeInterface (type, (IfaceHdr*) (object + 1), vtable, object, root, flags);
}

//.............................................................................

} // namespace jnc {
