#include "pch.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

void
primeIface (
	Box* box,
	Box* root,
	IfaceHdr* iface,
	ClassType* type,
	void* vtable
	)
{
	iface->m_vtable = vtable;
	iface->m_box = box;

	// prime all the base types

	rtl::Array <BaseTypeSlot*> baseTypePrimeArray = type->getBaseTypePrimeArray ();
	size_t count = baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = baseTypePrimeArray [i];
		ASSERT (slot->getType ()->getTypeKind () == TypeKind_Class);
		
		primeIface (
			box,
			root,
			(IfaceHdr*) ((char*) iface + slot->getOffset ()),
			(ClassType*) slot->getType (),
			(void**) vtable + slot->getVTableIndex ()
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
		Box* fieldBox = (Box*) ((char*) iface + field->getOffset ());

		prime (
			fieldBox,
			root,
			fieldType
			);
	}
}

void
prime (
	Box* box,
	Box* root,
	ClassType* type,
	void* vtable
	)
{
	ASSERT (root <= box);

	if (!vtable)
		vtable = type->getVTableVariable ()->getStaticData ();

	memset (box, 0, type->getSize ());

	box->m_type = type;
	box->m_flags = BoxFlag_StrongMark | BoxFlag_WeakMark;
	box->m_rootOffset = (char*) box - (char*) root;

	primeIface (box, root, (IfaceHdr*) (box + 1), type, vtable);
}

//.............................................................................

} // namespace jnc {
