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
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace ct {

//..............................................................................

ArrayType::ArrayType()
{
	m_typeKind = TypeKind_Array;
	m_flags = TypeFlag_StructRet;
	m_elementType = NULL;
	m_rootType = NULL;
	m_elementCount = -1;
	m_getDynamicSizeFunction = NULL;
	m_parentUnit = NULL;
	m_parentNamespace = NULL;
}

Type*
ArrayType::getRootType()
{
	if (!m_rootType)
		m_rootType = m_elementType->getTypeKind() == TypeKind_Array ?
			((ArrayType*)m_elementType)->getRootType() :
			m_elementType;

	return m_rootType;
}

void
ArrayType::prepareTypeString()
{
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_typeStringPrefix = getRootType()->getTypeString();
	tuple->m_typeStringSuffix = createDimensionString();
}

void
ArrayType::prepareDoxyLinkedText()
{
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyLinkedTextPrefix = getRootType()->getDoxyLinkedTextPrefix();
	tuple->m_doxyLinkedTextSuffix = createDimensionString();
}

sl::String
ArrayType::createDimensionString()
{
	sl::String string;

	if (m_elementCount == -1)
		string = "[]";
	else
		string.format("[%d]", m_elementCount);

	Type* elementType = m_elementType;
	while (elementType->getTypeKind() == TypeKind_Array)
	{
		ArrayType* arrayType = (ArrayType*)elementType;
		string.appendFormat(" [%d]", arrayType->m_elementCount);
		elementType = arrayType->m_elementType;
	}

	return string;
}

bool
ArrayType::ensureDynamicLayout(
	StructType* dynamicStruct,
	StructField* dynamicField
	)
{
	bool result;

	if (m_flags & ModuleItemFlag_LayoutReady)
		return true;

	result = calcLayoutImpl(dynamicStruct, dynamicField);
	if (!result)
		return false;

	m_flags |= ModuleItemFlag_LayoutReady;
	return true;
}

bool
ArrayType::calcLayoutImpl(
	StructType* dynamicStruct,
	StructField* dynamicField
	)
{
	bool result = m_elementType->ensureLayout();
	if (!result)
		return false;

	if (m_elementType->getTypeKind() == TypeKind_Class ||
		m_elementType->getFlags() & TypeFlag_Dynamic)
	{
		err::setFormatStringError("'%s' cannot be an element of an array", m_elementType->getTypeString ().sz ());
		return false;
	}

	// ensure update

	m_rootType = NULL;
	if (m_typeStringTuple)
	{
		AXL_MEM_DELETE(m_typeStringTuple);
		m_typeStringTuple = NULL;
	}

	uint_t rootTypeFlags = getRootType()->getFlags();
	if (rootTypeFlags & TypeFlag_Pod)
		m_flags |= TypeFlag_Pod;
	else if (rootTypeFlags & TypeFlag_GcRoot)
		m_flags |= TypeFlag_GcRoot;

	m_alignment = m_elementType->getAlignment();

	// calculate size

	if (!m_elementCountInitializer.isEmpty())
	{
		ASSERT(m_parentUnit && m_parentNamespace);

		if (m_parentUnit)
			m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

		m_module->m_namespaceMgr.openNamespace(m_parentNamespace);

		int64_t value = 0;
		result = m_module->m_operatorMgr.parseConstIntegerExpression(m_elementCountInitializer, &value);

		m_module->m_namespaceMgr.closeNamespace();

		if (!result)
		{
			if (!dynamicStruct)
				return false;

			sl::String qualifiedName = sl::formatString (
				"%s.%s.getDynamicSize",
				dynamicStruct->getQualifiedName().sz(),
				dynamicField->getName().sz()
				);

			Type* returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
			FunctionType* type = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);

			m_getDynamicSizeFunction = m_module->m_functionMgr.createFunction(
				FunctionKind_Internal,
				"getDynamicSize",
				qualifiedName,
				type
				);

			m_getDynamicSizeFunction->m_storageKind = StorageKind_Member;
			m_getDynamicSizeFunction->convertToMemberMethod(dynamicStruct);
			m_module->markForCompile(this);

			m_flags |= TypeFlag_Dynamic;
			m_elementCount = 0;
			m_size = 0;
			return true;
		}

		if (value <= 0)
		{
			err::setFormatStringError("invalid array size '%lld'\n", value);
			lex::pushSrcPosError(
				m_parentUnit->getFilePath(),
				m_elementCountInitializer.getHead()->m_pos
				);

			return false;
		}

#if (JNC_PTR_SIZE == 4)
		if (value >= (uint32_t) -1)
		{
			err::setFormatStringError("array size '%lld' is too big\n", value);
			lex::pushSrcPosError(
				m_parentUnit->getFilePath(),
				m_elementCountInitializer.getHead()->m_pos
				);

			return false;
		}
#endif

		m_elementCount = (size_t)value;
	}

	sl::String signature = createSignature(m_elementType, m_elementCount);
	m_module->m_typeMgr.updateTypeSignature(this, signature);

	m_size = m_elementType->getSize() * m_elementCount;
	if (m_size > TypeSizeLimit_StackAllocSize)
		m_flags |= TypeFlag_NoStack;

	return true;
}

bool
ArrayType::compile()
{
	ASSERT(m_getDynamicSizeFunction);

	bool result;

	m_module->m_functionMgr.internalPrologue(m_getDynamicSizeFunction);
	m_module->m_functionMgr.createThisValue();

	Unit* parentUnit = m_getDynamicSizeFunction->getParentUnit();
	if (parentUnit)
		m_module->m_unitMgr.setCurrentUnit(parentUnit);

	m_module->m_namespaceMgr.openNamespace(m_getDynamicSizeFunction->getParentNamespace());

	Value resultValue;
	result = m_module->m_operatorMgr.parseExpression(m_elementCountInitializer, &resultValue);
	if (!result)
		return false;

	size_t size = m_elementType->getSize();
	if (size != 1)
	{
		Value sizeValue(size, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
		result = m_module->m_operatorMgr.binaryOperator(
			BinOpKind_Mul,
			&resultValue,
			sizeValue
			);

		if (!result)
			return false;
	}

	result = m_module->m_controlFlowMgr.ret(resultValue);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_functionMgr.internalEpilogue();
	return true;
}

void
ArrayType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT(m_flags & TypeFlag_GcRoot);
	gcHeap->addRootArray(p, m_elementType, m_elementCount);
}

void
ArrayType::prepareLlvmDiType()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createArrayType(this);
}

//..............................................................................

} // namespace ct
} // namespace jnc
