#include "pch.h"
#include "jnc_McSnapshotClassType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

McSnapshotClassType::McSnapshotClassType ()
{
	m_classTypeKind = ClassTypeKind_McSnapshot;
	m_targetType = NULL;
	memset (m_fieldArray, 0, sizeof (m_fieldArray));
	memset (m_methodArray, 0, sizeof (m_methodArray));
}

void
McSnapshotClassType::prepareTypeString ()
{
	m_typeString = m_targetType->getTypeModifierString ();
	m_typeString.appendFormat ("mcsnapshot %s", m_targetType->getTargetType ()->getArgString ().cc ());
}

bool
McSnapshotClassType::compileCallMethod ()
{
	Function* function = m_methodArray [McSnapshotMethodKind_Call];

	rtl::Array <FunctionArg*> argArray = function->getType ()->getArgArray ();
	size_t argCount = argArray.getCount ();

	char buffer [256];
	rtl::Array <Value> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argValueArray.setCount (argCount);

	m_module->m_functionMgr.internalPrologue (function, argValueArray, argCount);

	rtl::BoxList <Value> argValueList;
	for (size_t i = 1; i < argCount; i++)
		argValueList.insertTail (argValueArray [i]);

	Value countValue;
	Value ptrPfnValue;
	Value ptrPfnEndValue;
	Value ptrPfnVariable;

	Type* ptrPfnType = m_targetType->getDataPtrType_c ();
	Type* sizeType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_module->m_operatorMgr.getClassField (argValueArray [0], m_fieldArray [McSnapshotFieldKind_Count], NULL, &countValue);
	m_module->m_operatorMgr.getClassField (argValueArray [0], m_fieldArray [McSnapshotFieldKind_PtrArray], NULL, &ptrPfnValue);

	m_module->m_llvmIrBuilder.createAlloca (ptrPfnType, "ppf", NULL, &ptrPfnVariable);
	m_module->m_llvmIrBuilder.createLoad (ptrPfnValue, ptrPfnValue.getType (), &ptrPfnValue);
	m_module->m_llvmIrBuilder.createLoad (countValue, countValue.getType (), &countValue);
	m_module->m_llvmIrBuilder.createStore (ptrPfnValue, ptrPfnVariable);
	m_module->m_llvmIrBuilder.createGep (ptrPfnValue, countValue, ptrPfnType, &ptrPfnEndValue);

	BasicBlock* conditionBlock = m_module->m_controlFlowMgr.createBlock ("mccall_cond");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("mccall_loop");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock ("mccall_follow");

	m_module->m_controlFlowMgr.follow (conditionBlock);

	Value idxValue;
	Value cmpValue;
	m_module->m_llvmIrBuilder.createLoad (ptrPfnVariable, NULL, &ptrPfnValue);
	m_module->m_llvmIrBuilder.createGe_u (ptrPfnValue, ptrPfnEndValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, followBlock, bodyBlock, bodyBlock);

	Value pfnValue;
	m_module->m_llvmIrBuilder.createLoad (ptrPfnValue, m_targetType, &pfnValue);
	m_module->m_operatorMgr.callOperator (pfnValue, &argValueList);

	m_module->m_llvmIrBuilder.createGep (ptrPfnValue, 1, ptrPfnType, &ptrPfnValue);
	m_module->m_llvmIrBuilder.createStore (ptrPfnValue, ptrPfnVariable);
	m_module->m_controlFlowMgr.jump (conditionBlock, followBlock);

	m_module->m_functionMgr.internalEpilogue ();

	return true;
}

void
McSnapshotClassType::markGcRoots (
	const void* _p,
	GcHeap* gcHeap
	)
{
	Box* object = (Box*) _p;
	ASSERT (object->m_type == this);

	McSnapshot* snapshot = (McSnapshot*) (object + 1);
	if (!(m_targetType->getFlags () & TypeFlag_GcRoot) || !snapshot->m_count)
		return;

	char* p = (char*) snapshot->m_ptrArray;
	size_t size = m_targetType->getSize ();

	for (size_t i = 0; i < snapshot->m_count; i++, p += size)
		gcHeap->addRoot (p, m_targetType);
}

//.............................................................................

} // namespace jnc {
