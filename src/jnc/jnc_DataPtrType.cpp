#include "pch.h"
#include "jnc_DataPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
getDataPtrTypeKindString (DataPtrTypeKind ptrTypeKind)
{
	static const char* stringTable [DataPtrTypeKind__Count] = 
	{
		"normal", // EDataPtrType_Normal = 0,
		"lean",   // EDataPtrType_Lean,
		"thin",   // EDataPtrType_Thin,
	};
		
	return (size_t) ptrTypeKind < DataPtrTypeKind__Count ? 
		stringTable [ptrTypeKind] : 
		"undefined-data-ptr-kind";
}

//.............................................................................

DataPtrType::DataPtrType ()
{
	m_typeKind = TypeKind_DataPtr;
	m_ptrTypeKind = DataPtrTypeKind_Normal;
	m_targetType = NULL;
	m_anchorNamespace = NULL;
	m_size = sizeof (DataPtr);
}

bool
DataPtrType::isConstPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Const) != 0 || 
		(m_flags & PtrTypeFlag_ReadOnly) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

rtl::String
DataPtrType::createSignature (
	Type* baseType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	rtl::String signature = typeKind == TypeKind_DataRef ? "RD" : "PD";

	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Lean:
		signature += 'l';
		break;

	case DataPtrTypeKind_Thin:
		signature += 't';
		break;
	}

	signature += getPtrTypeFlagSignature (flags);
	signature += baseType->getSignature ();
	return signature;
}

void
DataPtrType::prepareTypeString ()
{
	m_typeString = m_targetType->getTypeString ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		m_typeString += ' ';
		m_typeString += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != DataPtrTypeKind_Normal)
	{
		m_typeString += ' ';
		m_typeString += getDataPtrTypeKindString (m_ptrTypeKind);
	}

	m_typeString += m_typeKind == TypeKind_DataRef ? "&" : "*";
}

void
DataPtrType::prepareLlvmType ()
{
	m_llvmType = 
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType (StdType_DataPtrStruct)->getLlvmType () : 
		m_targetType->getTypeKind () != TypeKind_Void ? llvm::PointerType::get (m_targetType->getLlvmType (), 0) :
		m_module->m_typeMgr.getStdType (StdType_BytePtr)->getLlvmType ();
}

void
DataPtrType::prepareLlvmDiType ()
{
	m_llvmDiType = 
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType (StdType_DataPtrStruct)->getLlvmDiType () :
		m_targetType->getTypeKind () != TypeKind_Void ? m_module->m_llvmDiBuilder.createPointerType (m_targetType) :
		m_module->m_typeMgr.getStdType (StdType_BytePtr)->getLlvmDiType ();
}

void
DataPtrType::gcMark (
	Runtime* runtime,
	void* p
	)
{
	if (m_ptrTypeKind == DataPtrTypeKind_Normal)
	{
		DataPtr* ptr = (DataPtr*) p;		
		Box* object = ptr->m_object;
		if (!object)
			return;

		object->gcMarkData (runtime);
	}
	else
	{
		#pragma AXL_TODO ("mark thin data pointer roots -- added by 'heap new'")
	}
}

//.............................................................................

DataPtr
strDup (
	const char* p,
	size_t length
	)
{
	if (length == -1)
		length = p ? strlen (p) : 0;

	if (!length)
		return g_nullPtr;

	char* dst = (char*) AXL_MEM_ALLOC (length + 1);
	memcpy (dst, p, length);
	dst [length] = 0;

	jnc::DataPtr ptr;
	ptr.m_p = dst;
	ptr.m_rangeBegin = dst;
	ptr.m_rangeEnd = dst + length + 1;
	ptr.m_object = jnc::getStaticBox ();
	return ptr;
}

//.............................................................................

} // namespace jnc {
