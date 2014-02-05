// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DerivableType.h"
#include "jnc_BitFieldType.h"

namespace jnc {

class CStructType;
class CUnionType;

//.............................................................................

enum EStructFieldFlag
{
	EStructFieldFlag_WeakMasked = 0x010000,
};

//.............................................................................

class CStructField: public CUserModuleItem
{
	friend class CTypeMgr;
	friend class CDerivableType;
	friend class CProperty;
	friend class CStructType;
	friend class CUnionType;
	friend class CClassType;

protected:
	CType* m_pType;
	CImportType* m_pType_i;
	uint_t m_PtrTypeFlags;
	rtl::CBoxListT <CToken> m_Constructor;
	rtl::CBoxListT <CToken> m_Initializer;

	CType* m_pBitFieldBaseType;
	size_t m_BitCount;
	size_t m_Offset;
	uint_t m_LlvmIndex;

public:
	CStructField ();

	CType*
	GetType ()
	{
		return m_pType;
	}

	CImportType*
	GetType_i ()
	{
		return m_pType_i;
	}

	int
	GetPtrTypeFlags ()
	{
		return m_PtrTypeFlags;
	}

	rtl::CConstBoxListT <CToken>
	GetConstructor ()
	{
		return m_Constructor;
	}

	rtl::CConstBoxListT <CToken>
	GetInitializer ()
	{
		return m_Initializer;
	}

	size_t
	GetOffset ()
	{
		return m_Offset;
	}

	uint_t
	GetLlvmIndex ()
	{
		return m_LlvmIndex;
	}
};

//.............................................................................

enum EStructType
{
	EStructType_Normal,
	EStructType_IfaceStruct,
	EStructType_ClassStruct,
	EStructType_UnionStruct,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CStructType: public CDerivableType
{
	friend class CTypeMgr;
	friend class CClassType;
	friend class CUnionType;
	friend class CProperty;

protected:
	EStructType m_StructTypeKind;

	size_t m_PackFactor;
	size_t m_FieldActualSize;
	size_t m_FieldAlignedSize;

	rtl::CStdListT <CStructField> m_FieldList;
	rtl::CArrayT <CStructField*> m_FieldArray;
	rtl::CArrayT <CStructField*> m_InitializedFieldArray;
	rtl::CArrayT <llvm::Type*> m_LlvmFieldTypeArray;
	CBitFieldType* m_pLastBitFieldType;
	size_t m_LastBitFieldOffset;

public:
	CStructType ();

	EStructType
	GetStructTypeKind ()
	{
		return m_StructTypeKind;
	}

	size_t
	GetPackFactor ()
	{
		return m_PackFactor;
	}

	size_t
	GetFieldActualSize ()
	{
		return m_FieldActualSize;
	}

	size_t
	GetFieldAlignedSize ()
	{
		return m_FieldAlignedSize;
	}

	rtl::CConstListT <CStructField>
	GetFieldList ()
	{
		return m_FieldList;
	}

	virtual
	CStructField*
	GetFieldByIndex (size_t Index)
	{
		return GetFieldByIndexImpl (Index, false);
	}

	rtl::CArrayT <CStructField*>
	GetInitializedFieldArray ()
	{
		return m_InitializedFieldArray;
	}

	bool
	Append (CStructType* pType);

	bool
	InitializeFields (const CValue& ThisValue);

	virtual
	bool
	Compile ();

	virtual
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		);

protected:
	virtual
	CStructField*
	CreateFieldImpl (
		const rtl::CString& Name,
		CType* pType,
		size_t BitCount = 0,
		uint_t PtrTypeFlags = 0,
		rtl::CBoxListT <CToken>* pConstructor = NULL,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

	virtual
	void
	PrepareTypeString ()
	{
		m_TypeString.Format ("struct %s", m_Tag.cc ()); // thanks a lot gcc
	}

	virtual
	void
	PrepareLlvmType ();

	virtual
	void
	PrepareLlvmDiType ();

	virtual
	bool
	CalcLayout ();

	bool
	CompileDefaultPreConstructor ();

	CStructField*
	GetFieldByIndexImpl (
		size_t Index,
		bool IgnoreBaseTypes
		);

	bool
	LayoutField (
		llvm::Type* pLlvmType,
		size_t Size,
		size_t AlignFactor,
		size_t* pOffset,
		uint_t* pLlvmIndex
		);

	bool
	LayoutField (
		CType* pType,
		size_t* pOffset,
		uint_t* pLlvmIndex
		)
	{
		return
			pType->EnsureLayout () &&
			LayoutField (
				pType->GetLlvmType (),
				pType->GetSize (),
				pType->GetAlignFactor (),
				pOffset,
				pLlvmIndex
				);
	}

	bool
	LayoutBitField (
		CType* pBaseType,
		size_t BitCount,
		CType** ppType,
		size_t* pOffset,
		uint_t* pLlvmIndex
		);

	size_t
	GetFieldOffset (size_t AlignFactor);

	size_t
	GetBitFieldBitOffset (
		CType* pType,
		size_t BitCount
		);

	size_t
	SetFieldActualSize (size_t Size);

	CArrayType*
	InsertPadding (size_t Size);
};

//.............................................................................

} // namespace jnc {
