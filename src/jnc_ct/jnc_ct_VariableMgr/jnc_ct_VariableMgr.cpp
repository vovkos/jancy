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
#include "jnc_ct_VariableMgr.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

VariableMgr::VariableMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_currentLiftedStackVariable = NULL;
	m_tlsStructType = NULL;
	m_extraStackPtrFlags = 0;

	memset(m_stdVariableArray, 0, sizeof(m_stdVariableArray));
}

void
VariableMgr::clear() {
	m_variableList.clear();
	m_staticGcRootArray.clear();
	m_staticVariableArray.clear();
	m_globalVariablePrimeArray.clear();
	m_globalVariableInitializeArray.clear();
	m_liftedStackVariableArray.clear();
	m_argVariableArray.clear();
	m_tlsVariableArray.clear();
	m_currentLiftedStackVariable = NULL;
	m_tlsStructType = NULL;
	m_extraStackPtrFlags = 0;

	memset(m_stdVariableArray, 0, sizeof(m_stdVariableArray));
}

void
VariableMgr::createStdVariables() {
	// these variables are required even if not used (so the layout of TLS remains the same)

	getStdVariable(StdVariable_SjljFrame);
	getStdVariable(StdVariable_GcShadowStackTop);
	getStdVariable(StdVariable_AsyncScheduler);
}

Variable*
VariableMgr::getStdVariable(StdVariable stdVariable) {
	ASSERT((size_t)stdVariable < StdVariable__Count);

	if (m_stdVariableArray[stdVariable])
		return m_stdVariableArray[stdVariable];

	bool result;
	Variable* variable;

	switch (stdVariable) {
	case StdVariable_SjljFrame:
		variable = createVariable(
			StorageKind_Tls,
			"g_sjljFrame",
			"jnc.g_sjljFrame",
			m_module->m_typeMgr.getStdType(StdType_SjljFrame)->getDataPtrType_c()
		);
		break;

	case StdVariable_GcShadowStackTop:
		variable = createVariable(
			StorageKind_Tls,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_module->m_typeMgr.getStdType(StdType_GcShadowStackFrame)->getDataPtrType_c()
		);
		break;

	case StdVariable_GcSafePointTrigger:
		variable = createVariable(
			StorageKind_Static,
			"g_gcSafePointTrigger",
			"jnc.g_gcSafePointTrigger",
			m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr)->getDataPtrType_c()
		);
		break;

	case StdVariable_NullPtrCheckSink:
		variable = createVariable(
			StorageKind_Static,
			"g_nullPtrCheckSink",
			"jnc.g_nullPtrCheckSink",
			m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)
		);
		break;

	case StdVariable_AsyncScheduler:
		variable = createVariable(
			StorageKind_Tls,
			"g_asyncScheduler",
			"jnc.g_asyncScheduler",
			m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr)
		);
		break;

	default:
		ASSERT(false);
		return NULL;
	}

	result = allocateVariable(variable);
	ASSERT(result);

	variable->m_stdVariable = stdVariable;
	m_stdVariableArray[stdVariable] = variable;
	return variable;
}

Variable*
VariableMgr::createVariable(
	StorageKind storageKind,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	Type* type,
	uint_t ptrTypeFlags,
	sl::List<Token>* constructor,
	sl::List<Token>* initializer
) {
	if (storageKind == StorageKind_Stack)
		ptrTypeFlags |= m_extraStackPtrFlags;

	Variable* variable = AXL_MEM_NEW(Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = storageKind;
	variable->m_ptrTypeFlags = ptrTypeFlags;
	variable->m_scope = m_module->m_namespaceMgr.getCurrentScope();

	if (constructor)
		sl::takeOver(&variable->m_constructor, constructor);

	if (initializer)
		sl::takeOver(&variable->m_initializer, initializer);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&variable->m_type);

	m_variableList.insertTail(variable);
	return variable;
}

bool
VariableMgr::allocateNamespaceVariables(const sl::ConstIterator<Variable>& prevIt) {
	bool result;

	sl::Iterator<Variable> it = prevIt ? (Variable*)prevIt.getNext().p() : m_variableList.getHead();
	for (; it; it++) {
		ASSERT(it->m_storageKind == StorageKind_Static || it->m_storageKind == StorageKind_Tls);
		if (it->m_flags & VariableFlag_Allocated) // already
			continue;

		result = allocateVariable(*it);
		if (!result)
			return false;
	}

	return true;
}

bool
VariableMgr::allocateVariable(Variable* variable) {
	ASSERT(!(variable->m_flags & VariableFlag_Allocated) && !variable->m_llvmValue);

	bool result = variable->m_type->ensureLayout();
	if (!result)
		return false;

	bool isClassType = variable->m_type->getTypeKind() == TypeKind_Class;
	if (isClassType) {
		result = ((ClassType*)variable->m_type)->ensureCreatable();
		if (!result)
			return false;
	}

	if (!m_module->hasCodeGen()) {
		variable->m_flags |= VariableFlag_Allocated;
		return true;
	}

	Value ptrValue;
	switch (variable->m_storageKind) {
	case StorageKind_Static:
		ASSERT(!variable->m_llvmGlobalVariable);
		variable->m_llvmGlobalVariable = createLlvmGlobalVariable(variable->m_type, variable->m_qualifiedName);

		variable->m_llvmValue = isClassType ?
			(llvm::Value*)m_module->m_llvmIrBuilder.createGep2(variable->m_llvmGlobalVariable, 1, NULL, &ptrValue) :
			(llvm::Value*)variable->m_llvmGlobalVariable;

		if (variable->m_type->getFlags() & TypeFlag_GcRoot)
			m_staticGcRootArray.append(variable);

		m_staticVariableArray.append(variable);

		if (variable->m_parentNamespace &&
			variable->m_parentNamespace->getNamespaceKind() == NamespaceKind_Global) {
			if (isClassType)
				m_globalVariablePrimeArray.append(variable);

			if (!variable->m_initializer.isEmpty() ||
				isConstructibleType(variable->m_type))
				m_globalVariableInitializeArray.append(variable);
		}

		break;

	case StorageKind_Tls:
		m_tlsVariableArray.append(variable);
		break;

	case StorageKind_Stack:
		variable->m_llvmValue = m_module->m_llvmIrBuilder.createAlloca(variable->m_type, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.saveInsertPoint(&variable->m_liftInsertPoint);

		ASSERT(variable->m_scope);
		if (!variable->m_scope->m_firstStackVariable)
			variable->m_scope->m_firstStackVariable = variable;

		break;

	case StorageKind_Heap:
		result = allocateHeapVariable(variable);
		if (!result)
			return false;

		break;

	default:
		ASSERT(false);
	}

	variable->m_flags |= VariableFlag_Allocated;
	return true;
}

Variable*
VariableMgr::createSimpleStackVariable(
	const sl::StringRef& name,
	Type* type,
	uint_t ptrTypeFlags
) {
	Variable* variable = createVariable(StorageKind_Stack, name, name, type, ptrTypeFlags);
	bool result = allocateVariable(variable);
	ASSERT(result);
	return variable;
}

Variable*
VariableMgr::createSimpleStaticVariable(
	const sl::StringRef& name,
	Type* type,
	const Value& value,
	uint_t ptrTypeFlags
) {
	ASSERT(type->getTypeKind() != TypeKind_Class);

	Variable* variable = createVariable(StorageKind_Static, name, type, ptrTypeFlags);
	variable->m_llvmGlobalVariable = createLlvmGlobalVariable(type, name, value);
	variable->m_llvmValue = variable->m_llvmGlobalVariable;
	variable->m_flags |= VariableFlag_Allocated;

	if (type->getFlags() & TypeFlag_GcRoot)
		m_staticGcRootArray.append(variable);

	return variable;
}

bool
VariableMgr::initializeVariable(Variable* variable) {
	if (variable->m_type->getFlags() & TypeFlag_Dynamic) {
		err::setFormatStringError(
			"'%s' uses dynamic type '%s'",
			variable->getQualifiedName().sz(),
			variable->m_type->getTypeString().sz()
		);

		variable->pushSrcPosError();
		return false;
	}

	if (m_module->hasCodeGen())
		switch (variable->m_storageKind) {
		case StorageKind_Static:
			// only local class statics need to be primed (globals are primed in module constructor,
			// type/property member static variables are primed in static constructor)

			if (variable->m_type->getTypeKind() == TypeKind_Class &&
				variable->m_parentNamespace->getNamespaceKind() == NamespaceKind_Scope)
				primeStaticClassVariable(variable);
			break;

		case StorageKind_Tls:
		case StorageKind_Heap:
			break;

		case StorageKind_Stack:
			if (variable->m_type->getFlags() & TypeFlag_GcRoot) {
				m_module->m_operatorMgr.zeroInitialize(variable);
				m_module->m_gcShadowStackMgr.markGcRoot(variable, variable->m_type);
			} else if ((variable->m_type->getTypeKindFlags() & TypeKindFlag_Aggregate) || variable->m_initializer.isEmpty()) {
				m_module->m_operatorMgr.zeroInitialize(variable);
			}

			break;

		default:
			ASSERT(false);
		};

	Unit* prevUnit = variable->m_parentUnit ? m_module->m_unitMgr.setCurrentUnit(variable->m_parentUnit) : NULL;

	bool result = m_module->m_operatorMgr.parseInitializer(
		variable,
		&variable->m_constructor,
		&variable->m_initializer
	);

	if (!result)
		return false;

	if (prevUnit)
		m_module->m_unitMgr.setCurrentUnit(prevUnit);

	return true;
}

bool
VariableMgr::finalizeDisposableVariable(Variable* variable) {
	ASSERT(variable->m_scope && (variable->m_scope->getFlags() & ScopeFlag_Disposable));

	bool result;

	// we have to save the pointer in entry block -- otherwise, variable's
	// llvmValue definition may not be seen from the dispose block

	Type* ptrType = variable->m_type->getTypeKind() == TypeKind_Class ?
		(Type*)((ClassType*)variable->m_type)->getClassPtrType() :
		(variable->m_type->getTypeKindFlags() & TypeKindFlag_Ptr) ?
			variable->m_type->getDataPtrType_c() :
			variable->m_type->getDataPtrType();

	Variable* ptrVariable = createSimpleStackVariable("disposable_variable_ptr", ptrType);

	Value ptrValue;
	result =
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, variable, &ptrValue) &&
		m_module->m_operatorMgr.storeDataRef(ptrVariable, ptrValue);

	if (!result)
		return false;

	size_t count = variable->m_scope->addDisposableVariable(ptrVariable);

	if (m_module->hasCodeGen()) {
		Variable* disposeLevelVariable = variable->m_scope->getDisposeLevelVariable();

		m_module->m_llvmIrBuilder.createStore(
			Value(&count, disposeLevelVariable->m_type),
			disposeLevelVariable
		);
	}

	return true;
}

llvm::GlobalVariable*
VariableMgr::createLlvmGlobalVariable(
	Type* type,
	const sl::StringRef& name,
	const Value& initValue
) {
	llvm::Constant* llvmInitConstant = initValue ?
		(llvm::Constant*)initValue.getLlvmValue() :
		(llvm::Constant*)type->getZeroValue().getLlvmValue();

	sl::String llvmName;
#if (_JNC_JIT != _JNC_JIT_LLVM_LEGACY)
	llvmName = "?"; // as to avoid linking conflicts
	llvmName += name;
#else
	llvmName = name;
#endif

	return new llvm::GlobalVariable(
		*m_module->getLlvmModule(),
		type->getLlvmType(),
		false,
		llvm::GlobalVariable::ExternalLinkage, //InternalLinkage,
		llvmInitConstant,
		llvmName >> toLlvm
	);
}

void
VariableMgr::primeStaticClassVariable(Variable* variable) {
	ASSERT(variable->m_storageKind == StorageKind_Static && variable->m_type->getTypeKind() == TypeKind_Class);

	Function* primeStaticClass = m_module->m_functionMgr.getStdFunction(StdFunc_PrimeStaticClass);

	Value argValueArray[2];
	m_module->m_llvmIrBuilder.createBitCast(
		variable->m_llvmGlobalVariable,
		m_module->m_typeMgr.getStdType(StdType_BoxPtr),
		&argValueArray[0]
	);

	argValueArray[1].createConst(
		&variable->m_type,
		m_module->m_typeMgr.getStdType(StdType_BytePtr)
	);

	m_module->m_llvmIrBuilder.createCall(
		primeStaticClass,
		primeStaticClass->getType(),
		argValueArray,
		2,
		NULL
	);

	Function* destructor = ((ClassType*)variable->m_type)->getDestructor();
	if (destructor) {
		Function* addDestructor = m_module->m_functionMgr.getStdFunction(StdFunc_AddStaticClassDestructor);

		Value argValueArray[2];

		m_module->m_llvmIrBuilder.createBitCast(destructor, m_module->m_typeMgr.getStdType(StdType_BytePtr), &argValueArray[0]);
		m_module->m_llvmIrBuilder.createBitCast(variable, m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr), &argValueArray[1]);
		m_module->m_llvmIrBuilder.createCall(addDestructor, addDestructor->getType(), argValueArray, countof(argValueArray), NULL);
	}
}

Variable*
VariableMgr::createOnceFlagVariable(StorageKind storageKind) {
	Variable* variable = createVariable(
		storageKind,
		"onceFlag",
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32),
		storageKind == StorageKind_Static ? PtrTypeFlag_Volatile : 0
	);

	bool result = allocateVariable(variable);
	ASSERT(result);
	return variable;
}

LeanDataPtrValidator*
VariableMgr::createStaticDataPtrValidator(Variable* variable) {
	ASSERT(variable->m_storageKind == StorageKind_Static);

	// create detached static data box

	StructType* boxType = (StructType*)m_module->m_typeMgr.getStdType(StdType_DetachedDataBox);
	StructType* validatorType = (StructType*)m_module->m_typeMgr.getStdType(StdType_DataPtrValidator);

	sl::String boxName = variable->getQualifiedName() + ".box";

	llvm::GlobalVariable* llvmBoxVariable = new llvm::GlobalVariable(
		*m_module->getLlvmModule(),
		boxType->getLlvmType(),
		false,
		llvm::GlobalVariable::InternalLinkage,
		NULL,
		boxName >> toLlvm
	);

	Value variablePtrValue;
	Value variableEndPtrValue;

	m_module->m_llvmIrBuilder.createBitCast(
		variable,
		m_module->m_typeMgr.getStdType(StdType_BytePtr),
		&variablePtrValue
	);

	m_module->m_llvmIrBuilder.createGep(
		variablePtrValue,
		variable->m_type->getSize(),
		m_module->m_typeMgr.getStdType(StdType_BytePtr),
		&variableEndPtrValue
	);

	ASSERT(llvm::isa<llvm::Constant> (variablePtrValue.getLlvmValue()));
	ASSERT(llvm::isa<llvm::Constant> (variableEndPtrValue.getLlvmValue()));

	// validator initializer

	llvm::Constant* llvmMemberArray[4]; // this buffer is used twice

	Value boxPtrValue;
	m_module->m_llvmIrBuilder.createBitCast(
		llvmBoxVariable,
		m_module->m_typeMgr.getStdType(StdType_BoxPtr),
		&boxPtrValue
	);

	ASSERT(llvm::isa<llvm::Constant> (boxPtrValue.getLlvmValue()));

	llvmMemberArray[0] = (llvm::Constant*)boxPtrValue.getLlvmValue();
	llvmMemberArray[1] = (llvm::Constant*)boxPtrValue.getLlvmValue();
	llvmMemberArray[2] = (llvm::Constant*)variablePtrValue.getLlvmValue();
	llvmMemberArray[3] = (llvm::Constant*)variableEndPtrValue.getLlvmValue();

	llvm::Constant* llvmValidatorConst = llvm::ConstantStruct::get(
		(llvm::StructType*)validatorType->getLlvmType(),
		llvm::ArrayRef<llvm::Constant*> (llvmMemberArray, 4)
	);

	// box initializer

	uintptr_t flags = BoxFlag_Detached | BoxFlag_Static | BoxFlag_DataMark | BoxFlag_WeakMark;

	llvmMemberArray[0] = Value::getLlvmConst(m_module->m_typeMgr.getStdType(StdType_BytePtr), &variable->m_type);
	llvmMemberArray[1] = Value::getLlvmConst(m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr_u), &flags);
	llvmMemberArray[2] = llvmValidatorConst;
	llvmMemberArray[3] = (llvm::Constant*)variablePtrValue.getLlvmValue();

	llvm::Constant* llvmBoxConst = llvm::ConstantStruct::get(
		(llvm::StructType*)boxType->getLlvmType(),
		llvm::ArrayRef<llvm::Constant*> (llvmMemberArray, 4)
	);

	llvmBoxVariable->setInitializer(llvmBoxConst);

	Value validatorPtrValue;
	m_module->m_llvmIrBuilder.createGep2(
		llvmBoxVariable,
		2,
		m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr),
		&validatorPtrValue
	);

	LeanDataPtrValidator* validator = variable->getLeanDataPtrValidator();
	validator->m_validatorValue = validatorPtrValue;
	return validator;
}

bool
VariableMgr::allocateHeapVariable(Variable* variable) {
	Value ptrValue;

	bool result = m_module->m_operatorMgr.gcHeapAllocate(variable->m_type, &ptrValue);
	if (!result)
		return false;

	if (variable->m_type->getTypeKind() == TypeKind_Class) {
		variable->m_llvmValue = ptrValue.getLlvmValue();
	} else {
		Value variableValue;
		Value validatorValue;

		m_module->m_llvmIrBuilder.createExtractValue(ptrValue, 0, NULL, &variableValue);
		m_module->m_llvmIrBuilder.createExtractValue(ptrValue, 1, NULL, &validatorValue);
		m_module->m_llvmIrBuilder.createBitCast(variableValue, variable->m_type->getDataPtrType_c(), &variableValue);
		variable->m_llvmValue = variableValue.getLlvmValue();

		LeanDataPtrValidator* validator = variable->getLeanDataPtrValidator();
		validator->m_validatorValue = validatorValue;
	}

	// no need to mark gc root -- should have been marked in gcHeapAllocate
	return true;
}

void
VariableMgr::liftStackVariable(Variable* variable) {
	ASSERT(variable->m_storageKind == StorageKind_Stack);
	ASSERT(llvm::isa<llvm::AllocaInst>(variable->m_llvmValue));
	ASSERT(variable->m_liftInsertPoint);

	variable->m_llvmPreLiftValue = (llvm::AllocaInst*)variable->m_llvmValue;
	variable->m_storageKind = StorageKind_Heap;

	LlvmIrInsertPoint prevInsertPoint;
	bool isInsertPointChanged = m_module->m_llvmIrBuilder.restoreInsertPoint(
		variable->m_liftInsertPoint,
		&prevInsertPoint
	);

	m_currentLiftedStackVariable = variable;
	bool result = allocateHeapVariable(variable);
	ASSERT(result);
	m_currentLiftedStackVariable = NULL;

	if (isInsertPointChanged)
		m_module->m_llvmIrBuilder.restoreInsertPoint(prevInsertPoint);

	m_liftedStackVariableArray.append(variable);
}

void
VariableMgr::finalizeFunction() {
	if (m_module->hasCodeGen()) {
		size_t count = m_liftedStackVariableArray.getCount();
		for (size_t i = 0; i < count; i++) {
			Variable* variable = m_liftedStackVariableArray[i];
			ASSERT(variable->m_llvmPreLiftValue);
			variable->m_llvmPreLiftValue->replaceAllUsesWith(variable->m_llvmValue);
			variable->m_llvmPreLiftValue->eraseFromParent();
			variable->m_llvmPreLiftValue = NULL;
		}
	}

	m_liftedStackVariableArray.clear();
	m_argVariableArray.clear();
	m_extraStackPtrFlags = 0;
}

Variable*
VariableMgr::createArgVariable(
	FunctionArg* arg,
	size_t argIdx
) {
	Variable* variable = createSimpleStackVariable(
		arg->getName(),
		arg->getType(),
		arg->getPtrTypeFlags()
	);

	variable->m_parentUnit = arg->getParentUnit();
	variable->m_pos = arg->getPos();
	variable->m_flags |= ModuleItemFlag_User | VariableFlag_Arg;

	if ((m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo) &&
		(variable->getFlags() & ModuleItemFlag_User)) {
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createParameterVariable(variable, argIdx);
		m_module->m_llvmDiBuilder.createDeclare(variable);
	}

	// arg variables are not initialized (stored to directly), so mark gc root manually

	if (m_module->hasCodeGen() && (variable->m_type->getFlags() & TypeFlag_GcRoot))
		m_module->m_gcShadowStackMgr.markGcRoot(variable, variable->m_type);

	m_argVariableArray.append(variable);
	return variable;
}

Variable*
VariableMgr::createAsyncArgVariable(
	const sl::StringRef& name,
	Type* type,
	const Value& value
) {
	ASSERT(value.getLlvmValue());
	Variable* variable = createVariable(StorageKind_Member, name, type);
	variable->m_flags |= ModuleItemFlag_User | VariableFlag_Arg;
	variable->m_llvmValue = value.getLlvmValue();
	return variable;
}

Variable*
VariableMgr::createStaticRegexVariable(
	const sl::StringRef& name,
	const re::Regex* regex
) {
	// serialize regex into arrray

	sl::Array<char> regexStorage = regex->save();

	Value storageSizeValue;
	storageSizeValue.setConstSizeT(regexStorage.getCount(), m_module);

	Value storageValue;
	storageValue.setCharArray(regexStorage, regexStorage.getCount(), m_module);
	storageValue = m_module->m_constMgr.saveValue(storageValue);

	// create the regex variable

	ClassType* regexType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_Regex);
	Variable* variable = m_module->m_variableMgr.createVariable(StorageKind_Static, name, regexType);
	variable->m_parentNamespace = m_module->m_namespaceMgr.getCurrentScope();

	// initialize and load the regex variable inside a once-block

	lex::LineCol pos = m_module->m_namespaceMgr.getSourcePos();

	OnceStmt onceStmt;
	m_module->m_controlFlowMgr.onceStmt_Create(&onceStmt, pos);
	m_module->m_controlFlowMgr.onceStmt_PreBody(&onceStmt, pos);

	Value loadValue;

	bool result =
		allocateVariable(variable) &&
		initializeVariable(variable) &&
		m_module->m_operatorMgr.memberOperator(variable, "load", &loadValue) &&
		m_module->m_operatorMgr.callOperator(loadValue, storageValue, storageSizeValue);

	if (!result)
		return NULL;

	m_module->m_controlFlowMgr.onceStmt_PostBody(&onceStmt);
	return variable;
}

bool
VariableMgr::createTlsStructType() {
	bool result;

	StructType* type = m_module->m_typeMgr.createInternalStructType("jnc.Tls");

	size_t count = m_tlsVariableArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_tlsVariableArray[i];
		result = variable->m_type->ensureLayout();
		if (!result)
			return false;

		if (variable->m_type->getTypeKindFlags() & TypeKindFlag_Aggregate) {
			err::setFormatStringError("'threadlocal' variables cannot have aggregate type '%s'",  variable->m_type->getTypeString().sz());
			return false;
		}

		variable->m_tlsField = type->createField(variable->m_type);
	}

	result = type->ensureLayout();
	if (!result)
		return false;

	m_tlsStructType = type;
	return true;
}

void
VariableMgr::primeGlobalVariables() {
	size_t count = m_globalVariablePrimeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_globalVariablePrimeArray[i];
		ASSERT(variable->m_storageKind == StorageKind_Static && variable->m_type->getTypeKind() == TypeKind_Class);

		primeStaticClassVariable(variable);
	}

	m_globalVariablePrimeArray.clear();
}

bool
VariableMgr::initializeGlobalVariables() {
	bool finalResult = true;

	size_t count = m_globalVariableInitializeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_globalVariableInitializeArray[i];
		ASSERT(variable->m_storageKind == StorageKind_Static);

		m_module->m_namespaceMgr.openNamespace(variable->m_parentNamespace);

		bool result = initializeVariable(variable);
		if (!result)
			finalResult = false;

		m_module->m_namespaceMgr.closeNamespace();
	}

	m_globalVariableInitializeArray.clear();
	return finalResult;
}

//..............................................................................

} // namespace ct
} // namespace jnc
