#include "pch.h"
#include "jnc_ct_LlvmDiBuilder.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

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
		m_module->getName ().cc (),
		io::getCurrentDir ().cc (),
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

llvm::DIType
LlvmDiBuilder::createSubroutineType (FunctionType* functionType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	sl::Array <FunctionArg*> argArray = functionType->getArgArray ();
	size_t count = argArray.getCount ();

	char buffer [256];
	sl::Array <llvm::Value*> argTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argTypeArray.setCount (count + 1);

	llvm::Value** dst = argTypeArray;
	*dst = functionType->getReturnType ()->getLlvmDiType ();
	dst++;

	for (size_t i = 0; i < count; i++, dst++)
		*dst = argArray [i]->getType ()->getLlvmDiType ();

	llvm::DIArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType (unit->getLlvmDiFile (), llvmDiArray);
}

llvm::DIType
LlvmDiBuilder::createEmptyStructType (StructType* structType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	return m_llvmDiBuilder->createStructType (
		unit->getLlvmDiFile (),
		structType->m_tag.cc (),
		unit->getLlvmDiFile (),
		structType->getItemDecl ()->getPos ()->m_line + 1,
		structType->getSize () * 8,
		structType->getAlignment () * 8,
		0,
		llvm::DIType (), // derived from
		llvm::DIArray () // elements -- set body later
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
	sl::Array <llvm::Value*> fieldTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	fieldTypeArray.setCount (count);

	size_t i = 0;

	sl::Iterator <BaseTypeSlot> baseTypeIt = baseTypeList.getHead ();
	for (; baseTypeIt; i++, baseTypeIt++)
	{
		BaseTypeSlot* baseType = *baseTypeIt;
		sl::String name = baseType->getType ()->getQualifiedName ();

		fieldTypeArray [i] = m_llvmDiBuilder->createMemberType (
			unit->getLlvmDiFile (),
			!name.isEmpty () ? name.cc () : "UnnamedBaseType",
			unit->getLlvmDiFile (),
			baseType->getItemDecl ()->getPos ()->m_line + 1,
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
			!name.isEmpty () ? name.cc () : "m_unnamedField",
			unit->getLlvmDiFile (),
			field->getItemDecl ()->getPos ()->m_line + 1,
			field->getType ()->getSize () * 8,
			field->getType ()->getAlignment () * 8,
			field->getOffset () * 8,
			0,
			field->getType ()->getLlvmDiType ()
			);
	}

	llvm::DIArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (fieldTypeArray, count));

	llvm::DICompositeType llvmDiType (structType->getLlvmDiType ());
	ASSERT (llvmDiType);

	llvmDiType.setTypeArray (llvmDiArray);
}

llvm::DIType
LlvmDiBuilder::createEmptyUnionType (UnionType* unionType)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	return m_llvmDiBuilder->createUnionType (
		unit->getLlvmDiFile (),
		unionType->m_tag.cc (),
		unit->getLlvmDiFile (),
		unionType->getItemDecl ()->getPos ()->m_line + 1,
		unionType->getSize () * 8,
		unionType->getAlignment () * 8,
		0,
		llvm::DIArray () // elements -- set body later
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
	sl::Array <llvm::Value*> fieldTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	fieldTypeArray.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		StructField* field = fieldArray [i];
		sl::String name = field->getName ();

		fieldTypeArray [i] = m_llvmDiBuilder->createMemberType (
			unit->getLlvmDiFile (),
			!name.isEmpty () ? name.cc () : "m_unnamedField",
			unit->getLlvmDiFile (),
			field->getItemDecl ()->getPos ()->m_line + 1,
			field->getType ()->getSize () * 8,
			field->getType ()->getAlignment () * 8,
			field->getOffset () * 8,
			0,
			field->getType ()->getLlvmDiType ()
			);
	}

	llvm::DIArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (fieldTypeArray, count));

	llvm::DICompositeType llvmDiType (unionType->getLlvmDiType ());
	ASSERT (llvmDiType);

	llvmDiType.setTypeArray (llvmDiArray);
}

llvm::DIType
LlvmDiBuilder::createArrayType (ArrayType* arrayType)
{
	char buffer [256];
	sl::Array <llvm::Value*> dimArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	ArrayType* p = arrayType;
	for (;;)
	{
		Type* elementType = p->getElementType ();
		size_t elementCount = p->getElementCount ();
		ASSERT (elementCount);

		#pragma AXL_TODO ("seems like a bug in LLVM DiBuilder (should be ElementCount - 1)")
		dimArray.append (m_llvmDiBuilder->getOrCreateSubrange (0, elementCount));

		if (elementType->getTypeKind () != TypeKind_Array)
			break;

		p = (ArrayType*) elementType;
	}

	llvm::DIArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray (llvm::ArrayRef <llvm::Value*> (dimArray, dimArray.getCount ()));

	return m_llvmDiBuilder->createArrayType (
		arrayType->getSize () * 8,
		arrayType->getAlignment () * 8,
		arrayType->getRootType ()->getLlvmDiType (),
		llvmDiArray
		);
}

llvm::DIType
LlvmDiBuilder::createPointerType (Type* type)
{
	return m_llvmDiBuilder->createPointerType (
		type->getLlvmDiType (),
		type->getSize () * 8,
		type->getAlignment () * 8,
		type->getTypeString ().cc ()
		);
}

llvm::DIGlobalVariable
LlvmDiBuilder::createGlobalVariable (Variable* variable)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	return m_llvmDiBuilder->createGlobalVariable (
		variable->getQualifiedName ().cc (),
		unit->getLlvmDiFile (),
		variable->getItemDecl ()->getPos ()->m_line + 1,
		variable->getType ()->getLlvmDiType (),
		true,
		variable->getLlvmValue ()
		);
}

llvm::DIVariable
LlvmDiBuilder::createLocalVariable (
	Variable* variable,
	uint_t tag
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (unit && scope);

	return m_llvmDiBuilder->createLocalVariable (
		tag,
		scope->getLlvmDiScope (),
		variable->getName ().cc (),
		unit->getLlvmDiFile (),
		variable->getItemDecl ()->getPos ()->m_line + 1,
		variable->getType ()->getLlvmDiType (),
		true,
		0, 0
		);
}

llvm::Instruction*
LlvmDiBuilder::createDeclare (Variable* variable)
{
	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock ();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	ASSERT (block && scope);

	llvm::Instruction* llvmInstruction = m_llvmDiBuilder->insertDeclare (
		variable->getLlvmValue (),
		(llvm::DIVariable) variable->getLlvmDiDescriptor (),
		block->getLlvmBlock ()
		);

	llvm::DebugLoc llvmDebugLoc = llvm::DebugLoc::get (
		variable->getItemDecl ()->getPos ()->m_line + 1, 0,
		scope->getLlvmDiScope ()
		);

	llvmInstruction->setDebugLoc (llvmDebugLoc);
	return llvmInstruction;
}

llvm::DISubprogram
LlvmDiBuilder::createFunction (Function* function)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	Token::Pos declPos = *function->getItemDecl ()->getPos ();
	Token::Pos scopePos = function->hasBody () ? function->getBody ().getHead ()->m_pos : declPos;

	return m_llvmDiBuilder->createFunction (
		unit->getLlvmDiFile (),
		function->m_tag.cc (),
		function->m_tag.cc (), // linkage name
		unit->getLlvmDiFile (),
		declPos.m_line + 1,
		llvm::DICompositeType (function->getType ()->getLlvmDiType ()),
		false,
		true,
		scopePos.m_line + 1,
		0, // llvm::DIDescriptor::FlagPrototyped,
		false,
		function->getLlvmFunction ()
		);
}

llvm::DILexicalBlock
LlvmDiBuilder::createLexicalBlock (
	Scope* parentScope,
	const Token::Pos& pos
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	llvm::DIDescriptor llvmParentBlock;
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

//.............................................................................

} // namespace ct
} // namespace jnc
