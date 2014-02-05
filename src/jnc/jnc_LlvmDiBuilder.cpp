#include "pch.h"
#include "jnc_LlvmDiBuilder.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CLlvmDiBuilder::CLlvmDiBuilder ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_pLlvmDiBuilder = NULL;
}

void
CLlvmDiBuilder::Create ()
{
	Clear ();

	llvm::Module* pLlvmModule = m_pModule->GetLlvmModule ();
	ASSERT (pLlvmModule);

	m_pLlvmDiBuilder = new llvm::DIBuilder (*pLlvmModule);

	m_pLlvmDiBuilder->createCompileUnit (
		llvm::dwarf::DW_LANG_C99,
		m_pModule->GetName ().cc (),
		io::GetCurrentDir ().cc (),
		"jnc-1.0.0",
		false, "", 1
		);
}

void
CLlvmDiBuilder::Clear ()
{
	if (!m_pLlvmDiBuilder)
		return;

	delete m_pLlvmDiBuilder;
	m_pLlvmDiBuilder = NULL;
}

llvm::DebugLoc
CLlvmDiBuilder::GetEmptyDebugLoc ()
{
	// llvm magic: unfortunately, simple llvm::DebugLoc () doesn't quit cut it

	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	return pUnit ?
		llvm::DebugLoc::get (0, 0, pUnit->GetLlvmDiFile ()) :
		llvm::DebugLoc ();
}

llvm::DIType
CLlvmDiBuilder::CreateSubroutineType (CFunctionType* pFunctionType)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
	size_t Count = ArgArray.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Value*> ArgTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgTypeArray.SetCount (Count + 1);

	llvm::Value** ppDst = ArgTypeArray;
	*ppDst = pFunctionType->GetReturnType ()->GetLlvmDiType ();
	ppDst++;

	for (size_t i = 0; i < Count; i++, ppDst++)
		*ppDst = ArgArray [i]->GetType ()->GetLlvmDiType ();

	llvm::DIArray LlvmDiArray = m_pLlvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (ArgTypeArray, Count + 1));
	return m_pLlvmDiBuilder->createSubroutineType (pUnit->GetLlvmDiFile (), LlvmDiArray);
}

llvm::DIType
CLlvmDiBuilder::CreateEmptyStructType (CStructType* pStructType)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	return m_pLlvmDiBuilder->createStructType (
		pUnit->GetLlvmDiFile (),
		pStructType->m_Tag.cc (),
		pUnit->GetLlvmDiFile (),
		pStructType->GetItemDecl ()->GetPos ()->m_Line + 1,
		pStructType->GetSize () * 8,
		pStructType->GetAlignFactor () * 8,
		0,
		llvm::DIType (), // derived from
		llvm::DIArray () // elements -- set body later
		);
}

void
CLlvmDiBuilder::SetStructTypeBody (CStructType* pStructType)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	rtl::CConstListT <CBaseTypeSlot> BaseTypeList = pStructType->GetBaseTypeList ();
	rtl::CConstListT <CStructField> FieldList = pStructType->GetFieldList ();

	size_t Count = BaseTypeList.GetCount () + FieldList.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Value*> FieldTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	FieldTypeArray.SetCount (Count);

	size_t i = 0;

	rtl::CIteratorT <CBaseTypeSlot> BaseType = BaseTypeList.GetHead ();
	for (; BaseType; i++, BaseType++)
	{
		CBaseTypeSlot* pBaseType = *BaseType;
		rtl::CString Name = pBaseType->GetType ()->GetQualifiedName ();

		FieldTypeArray [i] = m_pLlvmDiBuilder->createMemberType (
			pUnit->GetLlvmDiFile (),
			!Name.IsEmpty () ? Name.cc () : "UnnamedBaseType",
			pUnit->GetLlvmDiFile (),
			pBaseType->GetItemDecl ()->GetPos ()->m_Line + 1,
			pBaseType->GetType ()->GetSize () * 8,
			pBaseType->GetType ()->GetAlignFactor () * 8,
			pBaseType->GetOffset () * 8,
			0,
			pBaseType->GetType ()->GetLlvmDiType ()
			);
	}

	rtl::CIteratorT <CStructField> Field = FieldList.GetHead ();
	for (; Field; i++, Field++)
	{
		CStructField* pField = *Field;
		rtl::CString Name = pField->GetName ();

		FieldTypeArray [i] = m_pLlvmDiBuilder->createMemberType (
			pUnit->GetLlvmDiFile (),
			!Name.IsEmpty () ? Name.cc () : "m_unnamedField",
			pUnit->GetLlvmDiFile (),
			pField->GetItemDecl ()->GetPos ()->m_Line + 1,
			pField->GetType ()->GetSize () * 8,
			pField->GetType ()->GetAlignFactor () * 8,
			pField->GetOffset () * 8,
			0,
			pField->GetType ()->GetLlvmDiType ()
			);
	}

	llvm::DIArray LlvmDiArray = m_pLlvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (FieldTypeArray, Count));

	llvm::DICompositeType LlvmDiType (pStructType->GetLlvmDiType ());
	ASSERT (LlvmDiType);

	LlvmDiType.setTypeArray (LlvmDiArray);
}

llvm::DIType
CLlvmDiBuilder::CreateEmptyUnionType (CUnionType* pUnionType)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	return m_pLlvmDiBuilder->createUnionType (
		pUnit->GetLlvmDiFile (),
		pUnionType->m_Tag.cc (),
		pUnit->GetLlvmDiFile (),
		pUnionType->GetItemDecl ()->GetPos ()->m_Line + 1,
		pUnionType->GetSize () * 8,
		pUnionType->GetAlignFactor () * 8,
		0,
		llvm::DIArray () // elements -- set body later
		);
}

void
CLlvmDiBuilder::SetUnionTypeBody (CUnionType* pUnionType)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	rtl::CConstListT <CStructField> FieldList = pUnionType->GetFieldList ();
	size_t Count = FieldList.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Value*> FieldTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	FieldTypeArray.SetCount (Count);

	rtl::CIteratorT <CStructField> Field = FieldList.GetHead ();
	for (size_t i = 0; Field; i++, Field++)
	{
		CStructField* pField = *Field;
		rtl::CString Name = pField->GetName ();

		FieldTypeArray [i] = m_pLlvmDiBuilder->createMemberType (
			pUnit->GetLlvmDiFile (),
			!Name.IsEmpty () ? Name.cc () : "m_unnamedField",
			pUnit->GetLlvmDiFile (),
			pField->GetItemDecl ()->GetPos ()->m_Line + 1,
			pField->GetType ()->GetSize () * 8,
			pField->GetType ()->GetAlignFactor () * 8,
			pField->GetOffset () * 8,
			0,
			pField->GetType ()->GetLlvmDiType ()
			);
	}

	llvm::DIArray LlvmDiArray = m_pLlvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (FieldTypeArray, Count));

	llvm::DICompositeType LlvmDiType (pUnionType->GetLlvmDiType ());
	ASSERT (LlvmDiType);

	LlvmDiType.setTypeArray (LlvmDiArray);
}

llvm::DIType
CLlvmDiBuilder::CreateArrayType (CArrayType* pArrayType)
{
	char Buffer [256];
	rtl::CArrayT <llvm::Value*> DimArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));

	CArrayType* p = pArrayType;
	for (;;)
	{
		CType* pElementType = p->GetElementType ();
		size_t ElementCount = p->GetElementCount ();
		ASSERT (ElementCount);

		#pragma AXL_TODO ("seems like a bug in LLVM DiBuilder (should be ElementCount - 1)")
		DimArray.Append (m_pLlvmDiBuilder->getOrCreateSubrange (0, ElementCount));

		if (pElementType->GetTypeKind () != EType_Array)
			break;

		p = (CArrayType*) pElementType;
	}

	llvm::DIArray LlvmDiArray = m_pLlvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (DimArray, DimArray.GetCount ()));

	return m_pLlvmDiBuilder->createArrayType (
		pArrayType->GetSize () * 8,
		pArrayType->GetAlignFactor () * 8,
		pArrayType->GetRootType ()->GetLlvmDiType (),
		LlvmDiArray
		);
}

llvm::DIType
CLlvmDiBuilder::CreatePointerType (CType* pType)
{
	return m_pLlvmDiBuilder->createPointerType (
		pType->GetLlvmDiType (),
		pType->GetSize () * 8,
		pType->GetAlignFactor () * 8,
		pType->GetTypeString ().cc ()
		);
}

llvm::DIGlobalVariable
CLlvmDiBuilder::CreateGlobalVariable (CVariable* pVariable)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	return m_pLlvmDiBuilder->createGlobalVariable (
		pVariable->GetQualifiedName ().cc (),
		pUnit->GetLlvmDiFile (),
		pVariable->GetItemDecl ()->GetPos ()->m_Line + 1,
		pVariable->GetType ()->GetLlvmDiType (),
		true,
		pVariable->GetLlvmValue ()
		);
}

llvm::DIVariable
CLlvmDiBuilder::CreateLocalVariable (
	CVariable* pVariable,
	uint_t Tag
	)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	ASSERT (pUnit && pScope);

	return m_pLlvmDiBuilder->createLocalVariable (
		Tag,
		pScope->GetLlvmDiScope (),
		pVariable->GetName ().cc (),
		pUnit->GetLlvmDiFile (),
		pVariable->GetItemDecl ()->GetPos ()->m_Line + 1,
		pVariable->GetType ()->GetLlvmDiType (),
		true,
		0, 0
		);
}

llvm::Instruction*
CLlvmDiBuilder::CreateDeclare (CVariable* pVariable)
{
	CBasicBlock* pBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();

	ASSERT (pBlock && pScope);

	llvm::Instruction* pLlvmInstruction = m_pLlvmDiBuilder->insertDeclare (
		pVariable->GetLlvmAllocValue (),
		(llvm::DIVariable) pVariable->GetLlvmDiDescriptor (),
		pBlock->GetLlvmBlock ()
		);

	llvm::DebugLoc LlvmDebugLoc = llvm::DebugLoc::get (
		pVariable->GetItemDecl ()->GetPos ()->m_Line + 1, 0,
		pScope->GetLlvmDiScope ()
		);

	pLlvmInstruction->setDebugLoc (LlvmDebugLoc);
	return pLlvmInstruction;
}

llvm::DISubprogram
CLlvmDiBuilder::CreateFunction (CFunction* pFunction)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	CToken::CPos DeclPos = *pFunction->GetItemDecl ()->GetPos ();
	CToken::CPos ScopePos = pFunction->HasBody () ? pFunction->GetBody ().GetHead ()->m_Pos : DeclPos;

	return m_pLlvmDiBuilder->createFunction (
		pUnit->GetLlvmDiFile (),
		pFunction->m_Tag.cc (),
		pFunction->m_Tag.cc (), // linkage name
		pUnit->GetLlvmDiFile (),
		DeclPos.m_Line + 1,
		llvm::DICompositeType (pFunction->GetType ()->GetLlvmDiType ()),
		false,
		true,
		ScopePos.m_Line + 1,
		0, // llvm::DIDescriptor::FlagPrototyped,
		false,
		pFunction->GetLlvmFunction ()
		);
}

llvm::DILexicalBlock
CLlvmDiBuilder::CreateLexicalBlock (
	CScope* pParentScope,
	const CToken::CPos& Pos
	)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	llvm::DIDescriptor LlvmParentBlock;
	if (pParentScope)
	{
		LlvmParentBlock = pParentScope->GetLlvmDiScope ();
	}
	else
	{
		CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
		ASSERT (pFunction);

		LlvmParentBlock = pFunction->GetLlvmDiSubprogram ();
	}

	return m_pLlvmDiBuilder->createLexicalBlock (
		LlvmParentBlock,
		pUnit->GetLlvmDiFile (),
		Pos.m_Line + 1, 0
		);
}

//.............................................................................

} // namespace jnc {
