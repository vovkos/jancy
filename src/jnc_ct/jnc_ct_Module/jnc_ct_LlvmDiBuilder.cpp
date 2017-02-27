//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_LlvmDiBuilder.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

LlvmDiBuilder::LlvmDiBuilder ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_llvmDiBuilder = NULL;
}

void
LlvmDiBuilder::create ()
{
	clear ();

	llvm::Module* llvmModule = m_module->getLlvmModule ();
	ASSERT (llvmModule);

	m_llvmDiBuilder = new llvm::DIBuilder (*llvmModule);

	m_llvmDiBuilder->createCompileUnit (
		llvm::dwarf::DW_LANG_C99,
		m_module->getName ().sz (),
		io::getCurrentDir ().sz (),
		"jnc-1.0.0",
		false, "", 1
		);
}

void
LlvmDiBuilder::clear ()
{
	if (!m_llvmDiBuilder)
		return;

	delete m_llvmDiBuilder;
	m_llvmDiBuilder = NULL;
}

llvm::DebugLoc
LlvmDiBuilder::getEmptyDebugLoc ()
{
	// llvm magic: unfortunately, simple llvm::DebugLoc () doesn't quit cut it

	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	return unit ?
		llvm::DebugLoc::get (0, 0, unit->getLlvmDiFile ()) :
		llvm::DebugLoc ();
}

llvm::DIType_vn
LlvmDiBuilder::createSubroutineType (FunctionType* functionType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	sl::Array <FunctionArg*> argArray = functionType->getArgArray ();
	size_t count = argArray.getCount ();

	char buffer [256];
	sl::Array <llvm::Metadata*> argTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argTypeArray.setCount (count + 1);

	llvm::Metadata** dst = argTypeArray;
	*dst = functionType->getReturnType ()->getLlvmDiType ();
	dst++;

	for (size_t i = 0; i < count; i++, dst++)
		*dst = argArray [i]->getType ()->getLlvmDiType ();

#if (LLVM_VERSION < 0x0309)
	llvm::DIArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType (unit->getLlvmDiFile (), llvmDiTypeArray);
#else
	llvm::DITypeRefArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateTypeArray (llvm::ArrayRef <llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType (llvmDiTypeArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createEmptyStructType (StructType* structType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	return m_llvmDiBuilder->createStructType (
		unit->getLlvmDiFile (),
		structType->m_tag.sz (),
		unit->getLlvmDiFile (),
		structType->getPos ()->m_line + 1,
		structType->getSize () * 8,
		structType->getAlignment () * 8,
		0,
#if (LLVM_VERSION < 0x0309)
		llvm::DIType (), // derived from
		llvm::DIArray () // elements -- set body later
#else
		NULL,                // derived from
		llvm::DINodeArray () // elements -- set body later
#endif
		);
}

void
LlvmDiBuilder::setStructTypeBody (StructType* structType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	sl::ConstList <BaseTypeSlot> baseTypeList = structType->getBaseTypeList ();
	sl::Array <StructField*> fieldArray = structType->getMemberFieldArray ();

	size_t baseTypeCount = baseTypeList.getCount ();
	size_t fieldCount = fieldArray.getCount ();
	size_t count = baseTypeCount + fieldCount;

	char buffer [256];
	sl::Array <llvm::Metadata*> fieldTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	fieldTypeArray.setCount (count);

	size_t i = 0;

	sl::Iterator <BaseTypeSlot> baseTypeIt = baseTypeList.getHead ();
	for (; baseTypeIt; i++, baseTypeIt++)
	{
		BaseTypeSlot* baseType = *baseTypeIt;
		sl::String name = baseType->getType ()->getQualifiedName ();

		fieldTypeArray [i] = m_llvmDiBuilder->createMemberType (
			unit->getLlvmDiFile (),
			!name.isEmpty () ? name.sz () : "UnnamedBaseType",
			unit->getLlvmDiFile (),
			baseType->getPos ()->m_line + 1,
			baseType->getType ()->getSize () * 8,
			baseType->getType ()->getAlignment () * 8,
			baseType->getOffset () * 8,
			0,
			baseType->getType ()->getLlvmDiType ()
			);
	}

	for (size_t j = 0; j < fieldCount; i++, j++)
	{
		StructField* field = fieldArray [j];
		sl::String name = field->getName ();

		fieldTypeArray [i] = m_llvmDiBuilder->createMemberType (
			unit->getLlvmDiFile (),
			!name.isEmpty () ? name.sz () : "m_unnamedField",
			unit->getLlvmDiFile (),
			field->getPos ()->m_line + 1,
			field->getType ()->getSize () * 8,
			field->getType ()->getAlignment () * 8,
			field->getOffset () * 8,
			0,
			field->getType ()->getLlvmDiType ()
			);
	}

	llvm::DIArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Metadata*> (fieldTypeArray, count));

#if (LLVM_VERSION < 0x0309)
	llvm::DICompositeType llvmDiType (structType->getLlvmDiType ());
	ASSERT (llvmDiType);
	llvmDiType.setTypeArray (llvmDiArray);
#else
	llvm::DICompositeType* llvmDiType = (llvm::DICompositeType*) structType->getLlvmDiType ();
	ASSERT (llvm::isa <llvm::DICompositeType> (llvmDiType));
	llvmDiType->replaceElements (llvmDiArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createEmptyUnionType (UnionType* unionType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	return m_llvmDiBuilder->createUnionType (
		unit->getLlvmDiFile (),
		unionType->m_tag.sz (),
		unit->getLlvmDiFile (),
		unionType->getPos ()->m_line + 1,
		unionType->getSize () * 8,
		unionType->getAlignment () * 8,
		0,
		llvm::DINodeArray () // elements -- set body later
		);
}

void
LlvmDiBuilder::setUnionTypeBody (UnionType* unionType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	sl::Array <StructField*> fieldArray = unionType->getMemberFieldArray ();
	size_t count = fieldArray.getCount ();

	char buffer [256];
	sl::Array <llvm::Metadata*> fieldTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	fieldTypeArray.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		StructField* field = fieldArray [i];
		sl::String name = field->getName ();

		fieldTypeArray [i] = m_llvmDiBuilder->createMemberType (
			unit->getLlvmDiFile (),
			!name.isEmpty () ? name.sz () : "m_unnamedField",
			unit->getLlvmDiFile (),
			field->getPos ()->m_line + 1,
			field->getType ()->getSize () * 8,
			field->getType ()->getAlignment () * 8,
			field->getOffset () * 8,
			0,
			field->getType ()->getLlvmDiType ()
			);
	}

	llvm::DINodeArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Metadata*> (fieldTypeArray, count));

#if (LLVM_VERSION < 0x0309)
	llvm::DICompositeType llvmDiType (unionType->getLlvmDiType ());
	ASSERT (llvmDiType);
	llvmDiType.setTypeArray (llvmDiArray);
#else
	llvm::DICompositeType* llvmDiType = (llvm::DICompositeType*) unionType->getLlvmDiType ();
	ASSERT (llvm::isa <llvm::DICompositeType> (llvmDiType));
	llvmDiType->replaceElements (llvmDiArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createArrayType (ArrayType* arrayType)
{
	char buffer [256];
	sl::Array <llvm::Metadata*> dimArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	ArrayType* p = arrayType;
	for (;;)
	{
		Type* elementType = p->getElementType ();
		size_t elementCount = p->getElementCount ();
		ASSERT (elementCount);

		AXL_TODO ("seems like a bug in LLVM DiBuilder (should be ElementCount - 1)")
		dimArray.append (m_llvmDiBuilder->getOrCreateSubrange (0, elementCount));

		if (elementType->getTypeKind () != TypeKind_Array)
			break;

		p = (ArrayType*) elementType;
	}

	llvm::DINodeArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Metadata*> (dimArray, dimArray.getCount ()));

	return m_llvmDiBuilder->createArrayType (
		arrayType->getSize () * 8,
		arrayType->getAlignment () * 8,
		arrayType->getRootType ()->getLlvmDiType (),
		llvmDiArray
		);
}

llvm::DIType_vn
LlvmDiBuilder::createPointerType (Type* type)
{
	return m_llvmDiBuilder->createPointerType (
		type->getLlvmDiType (),
		type->getSize () * 8,
		type->getAlignment () * 8,
		type->getTypeString ().sz ()
		);
}

llvm::DIGlobalVariable_vn
LlvmDiBuilder::createGlobalVariable (Variable* variable)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	llvm::GlobalVariable* llvmGlobalVariable = (llvm::GlobalVariable*) variable->getLlvmValue ();
	ASSERT (llvm::isa <llvm::GlobalVariable> (llvmGlobalVariable));

	return m_llvmDiBuilder->createGlobalVariable (
#if (LLVM_VERSION >= 0x0309)
		NULL, // DIScope* Context
#endif
		variable->getQualifiedName ().sz (), // StringRef Name
		variable->getQualifiedName ().sz (), // StringRef LinkageName
		unit->getLlvmDiFile (),
		variable->getPos ()->m_line + 1,
		variable->getType ()->getLlvmDiType (),
		true, // bool isLocalToUnit
		llvmGlobalVariable
		);
}

llvm::DIVariable_vn
LlvmDiBuilder::createParameterVariable (
	Variable* variable,
	size_t argumentIdx
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (unit && scope);

#if (LLVM_VERSION < 0x0309)
	return m_llvmDiBuilder->createLocalVariable (
		llvm::dwarf::DW_TAG_arg_variable,
		scope->getLlvmDiScope (),
		variable->getName ().sz (),
		unit->getLlvmDiFile (),
		variable->getPos ()->m_line + 1,
		variable->getType ()->getLlvmDiType (),
		true, // bool AlwaysPreserve
		0     // unsigned Flags
		);
#else
	return m_llvmDiBuilder->createParameterVariable (
		scope->getLlvmDiScope (),
		variable->getName ().sz (),
		argumentIdx + 1,
		unit->getLlvmDiFile (),
		variable->getPos ()->m_line + 1,
		variable->getType ()->getLlvmDiType (),
		true, // bool AlwaysPreserve
		0     // unsigned Flags
		);
#endif
}

llvm::Instruction*
LlvmDiBuilder::createDeclare (Variable* variable)
{
	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock ();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	ASSERT (block && scope);

#if (LLVM_VERSION < 0x0309)
	llvm::Instruction* llvmInstruction = m_llvmDiBuilder->insertDeclare (
		variable->getLlvmValue (),
		variable->getLlvmDiDescriptor (),
		block->getLlvmBlock ()
		);

	llvm::DebugLoc llvmDebugLoc = llvm::DebugLoc::get (
		variable->getPos ()->m_line + 1, 0,
		scope->getLlvmDiScope ()
		);

	llvmInstruction->setDebugLoc (llvmDebugLoc);
#else
	llvm::DILocalVariable* llvmDiLocalVariable = (llvm::DILocalVariable*) variable->getLlvmDiDescriptor ();
	ASSERT (llvm::isa <llvm::DILocalVariable> (llvmDiLocalVariable));

	const Token::Pos* pos = variable->getPos ();

	llvm::Instruction* llvmInstruction = m_llvmDiBuilder->insertDeclare (
		variable->getLlvmValue (),
		llvmDiLocalVariable,
		m_llvmDiBuilder->createExpression (),
		llvm::DebugLoc::get (pos->m_line, pos->m_col, scope->getLlvmDiScope()),
		block->getLlvmBlock ()
		);
#endif

	return llvmInstruction;
}

llvm::DISubprogram_vn
LlvmDiBuilder::createFunction (Function* function)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	Token::Pos declPos = *function->getPos ();
	Token::Pos scopePos = function->hasBody () ? function->getBody ().getHead ()->m_pos : declPos;

#if (LLVM_VERSION < 0x0309)
	llvm::DICompositeType llvmDiSubroutineType (function->getType ()->getLlvmDiType ());
	ASSERT (llvmDiSubroutineType);

	return m_llvmDiBuilder->createFunction (
		unit->getLlvmDiFile (),
		function->m_tag.sz (),
		function->m_tag.sz (), // linkage name
		unit->getLlvmDiFile (),
		declPos.m_line + 1,
		llvmDiSubroutineType,
		false,
		true,
		scopePos.m_line + 1,
		0,     // llvm::DIDescriptor::FlagPrototyped,
		false, // bool isOptimized
		function->getLlvmFunction (),
		NULL,  // MDNode* TParams
		NULL   // MDNode *Decl
		);
#else
	llvm::DISubroutineType* llvmDiSubroutineType = (llvm::DISubroutineType*) function->getType ()->getLlvmDiType ();
	ASSERT (llvm::isa <llvm::DISubroutineType> (llvmDiSubroutineType));

	return m_llvmDiBuilder->createFunction (
		NULL,
		function->m_tag.sz (),
		function->m_tag.sz (), // linkage name
		unit->getLlvmDiFile (),
		declPos.m_line + 1,
		llvmDiSubroutineType,
		false,
		true,
		scopePos.m_line + 1,
		0,     // llvm::DIDescriptor::FlagPrototyped,
		false,
		NULL,  // DITemplateParameterArray TParams
		NULL   // DISubprogram *Decl
		);
#endif
}

llvm::DILexicalBlock_vn
LlvmDiBuilder::createLexicalBlock (
	Scope* parentScope,
	const Token::Pos& pos
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	llvm::DIScope_vn llvmParentBlock;
	if (parentScope)
	{
		llvmParentBlock = parentScope->getLlvmDiScope ();
	}
	else
	{
		Function* function = m_module->m_functionMgr.getCurrentFunction ();
		ASSERT (function);

		llvmParentBlock = function->getLlvmDiSubprogram ();
	}

	return m_llvmDiBuilder->createLexicalBlock (
		llvmParentBlock,
		unit->getLlvmDiFile (),
		pos.m_line + 1, 0
		);
}

//..............................................................................

} // namespace ct
} // namespace jnc
