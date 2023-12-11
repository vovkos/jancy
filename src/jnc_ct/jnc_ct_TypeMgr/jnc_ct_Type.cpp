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
#include "jnc_ct_Type.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"
#include "jnc_Variant.h"
#include "jnc_String.h"
#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace ct {

//..............................................................................

TypeKind
getInt32TypeKind(int32_t integer) {
	return
		integer >= INT8_MIN && integer <= INT8_MAX ? TypeKind_Int8 :
		(uint32_t)integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer >= INT16_MIN && integer <= INT16_MAX ? TypeKind_Int16 :
		(uint32_t)integer <= UINT16_MAX ? TypeKind_Int16_u : TypeKind_Int32;
}

TypeKind
getInt32TypeKind_u(uint32_t integer) {
	return
		integer <= INT8_MAX ? TypeKind_Int8 :
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= INT16_MAX ? TypeKind_Int16 :
		integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer <= INT32_MAX ? TypeKind_Int32 : TypeKind_Int32_u;
}

TypeKind
getInt64TypeKind(int64_t integer) {
	return
		integer >= INT8_MIN && integer <= INT8_MAX ? TypeKind_Int8 :
		(uint64_t)integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer >= INT16_MIN && integer <= INT16_MAX ? TypeKind_Int16 :
		(uint64_t)integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer >= INT32_MIN && integer <= INT32_MAX ? TypeKind_Int32 :
		(uint64_t)integer <= UINT32_MAX ? TypeKind_Int32_u : TypeKind_Int64;
}

TypeKind
getInt64TypeKind_u(uint64_t integer) {
	return
		integer <= INT8_MAX ? TypeKind_Int8 :
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= INT16_MAX ? TypeKind_Int16 :
		integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer <= INT32_MAX ? TypeKind_Int32 :
		integer <= UINT32_MAX ? TypeKind_Int32_u :
		integer <= INT64_MAX ? TypeKind_Int64 : TypeKind_Int64_u;
}

//..............................................................................

sl::String
getLlvmTypeString(llvm::Type* llvmType) {
	std::string string;
	llvm::raw_string_ostream stream(string);
	llvmType->print(stream);
	return sl::String(string.data(), string.length());
}

//..............................................................................

const char*
getPtrTypeFlagString(PtrTypeFlag flag) {
	static const char* stringTable[] = {
		"safe",      // PtrTypeFlag_Safe      = 0x0010000
		"const",     // PtrTypeFlag_Const     = 0x0020000
		"readonly",  // PtrTypeFlag_ReadOnly  = 0x0040000
		"cmut",      // PtrTypeFlag_CMut      = 0x0080000
		"volatile",  // PtrTypeFlag_Volatile  = 0x0100000
		"event",     // PtrTypeFlag_Event     = 0x0200000
		"dualevent", // PtrTypeFlag_DualEvent = 0x0400000
		"bindable",  // PtrTypeFlag_Bindable  = 0x0800000
		"autoget",   // PtrTypeFlag_AutoGet   = 0x1000000
	};

	size_t i = sl::getLoBitIdx16((uint16_t)(flag >> 16));
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-ptr-type-flag";
}

sl::StringRef
getPtrTypeFlagString(uint_t flags) {
	flags &= PtrTypeFlag__All;
	if (!flags)
		return sl::StringRef();

	PtrTypeFlag flag = getFirstFlag<PtrTypeFlag>(flags);
	sl::StringRef string0 = getPtrTypeFlagString(flag);
	flags &= ~flag;
	if (!flags)
		return string0;

	sl::String string = string0;
	while (flags) {
		flag = getFirstFlag<PtrTypeFlag>(flags);
		string += ' ';
		string += getPtrTypeFlagString(flag);
		flags &= ~flag;
	}

	return string;
}

const char*
getPtrTypeFlagSignature(PtrTypeFlag flag) {
	// possible conflicts with primitive type signatures are OK

	static const char* stringTable[] = {
		"s",  // PtrTypeFlag_Safe      = 0x0010000
		"c",  // PtrTypeFlag_Const     = 0x0020000
		"r",  // PtrTypeFlag_ReadOnly  = 0x0040000
		"m",  // PtrTypeFlag_CMut      = 0x0080000
		"v",  // PtrTypeFlag_Volatile  = 0x0100000
		"e",  // PtrTypeFlag_Event     = 0x0200000
		"d",  // PtrTypeFlag_DualEvent = 0x0400000
		"b",  // PtrTypeFlag_Bindable  = 0x0800000
		"a",  // PtrTypeFlag_AutoGet   = 0x1000000
	};

	size_t i = sl::getLoBitIdx16((uint16_t)(flag >> 16));
	return i < countof(stringTable) ?
		stringTable[i] :
		"?";
}

sl::StringRef
getPtrTypeFlagSignature(uint_t flags) {
	flags &= PtrTypeFlag__All;
	if (!flags)
		return sl::StringRef();

	PtrTypeFlag flag = getFirstFlag<PtrTypeFlag>(flags);
	sl::StringRef string0 = sl::StringRef(getPtrTypeFlagSignature(flag), 1);
	flags &= ~flag;
	if (!flags)
		return string0;

	sl::String string = string0;
	while (flags) {
		flag = getFirstFlag<PtrTypeFlag>(flags);
		string.append(getPtrTypeFlagSignature(flag), 1);
		flags &= ~flag;
	}

	return string;
}

uint_t
getPtrTypeFlagsFromModifiers(uint_t modifiers) {
	uint_t flags = 0;

	if (modifiers & TypeModifier_Safe)
		flags |= PtrTypeFlag_Safe;

	if (modifiers & TypeModifier_Volatile)
		flags |= PtrTypeFlag_Volatile;

	if (modifiers & TypeModifier_Const)
		flags |= PtrTypeFlag_Const;
	else if (modifiers & TypeModifier_ReadOnly)
		flags |= PtrTypeFlag_ReadOnly;
	else if (modifiers & TypeModifier_CMut)
		flags |= PtrTypeFlag_CMut;

	if (modifiers & TypeModifier_Event)
		flags |= PtrTypeFlag_Event;

	return flags;
}

//..............................................................................

Type::Type() {
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = TypeKind_Void;
	m_stdType = (StdType)-1;
	m_size = 0;
	m_alignment = 0;
	m_llvmType = NULL;
	m_typeVariable = NULL;
	m_typeStringTuple = NULL;
	m_simplePropertyTypeTuple = NULL;
	m_functionArgTuple = NULL;
	m_dataPtrTypeTuple = NULL;
	m_dualTypeTuple = NULL;
}

Type::~Type() {
	if (m_typeStringTuple)
		delete m_typeStringTuple;
}

const sl::StringRef&
Type::getTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (!tuple->m_typeString.isEmpty())
		return tuple->m_typeString;

	prepareTypeString();
	if (tuple->m_typeStringPrefix.isEmpty())
		prepareTypeString();
	ASSERT(!tuple->m_typeStringPrefix.isEmpty());

	tuple->m_typeString = tuple->m_typeStringSuffix.isEmpty() ?
		tuple->m_typeStringPrefix :
		tuple->m_typeStringPrefix + ' ' + tuple->m_typeStringSuffix;

	return tuple->m_typeString;
}

const sl::StringRef&
Type::getTypeStringPrefix() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (tuple->m_typeStringPrefix.isEmpty()) {
		prepareTypeString();
		ASSERT(!tuple->m_typeStringPrefix.isEmpty());
	}

	return tuple->m_typeStringPrefix;
}

const sl::String&
Type::getTypeStringSuffix() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (tuple->m_typeStringPrefix.isEmpty()) { // this is not a typo, we still need to check prefix string!
		prepareTypeString();
		ASSERT(!tuple->m_typeStringPrefix.isEmpty());
	}

	return tuple->m_typeStringSuffix;
}

const sl::String&
Type::getDoxyTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (tuple->m_doxyTypeString.isEmpty()) {
		prepareDoxyTypeString();
		ASSERT(!tuple->m_doxyTypeString.isEmpty());
	}

	return tuple->m_doxyTypeString;
}

const sl::String&
Type::getDoxyLinkedTextPrefix() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (tuple->m_doxyLinkedTextPrefix.isEmpty()) {
		prepareDoxyLinkedText();
		ASSERT(!tuple->m_doxyLinkedTextPrefix.isEmpty());
	}

	return tuple->m_doxyLinkedTextPrefix;
}

const sl::String&
Type::getDoxyLinkedTextSuffix() {
	TypeStringTuple* tuple = getTypeStringTuple();
	if (tuple->m_doxyLinkedTextPrefix.isEmpty()) { // this is not a typo, we still need to check prefix string!
		prepareDoxyLinkedText();
		ASSERT(!tuple->m_doxyLinkedTextPrefix.isEmpty());
	}

	return tuple->m_doxyLinkedTextSuffix;
}

Value
Type::getUndefValue() {
	llvm::Value* llvmValue = llvm::UndefValue::get(getLlvmType());
	return Value(llvmValue, this);
}

Value
Type::getZeroValue() {
	AXL_TODO("Type::getZeroValue () probably should return ValueKind_Const")

	if (!m_module->hasCodeGen()) {
		Value value;
		value.createConst(NULL, this);
		return value;
	}

	llvm::Value* llvmValue = llvm::Constant::getNullValue(getLlvmType());
	return Value(llvmValue, this);
}

Value
Type::getErrorCodeValue() {
	uint_t typeKindFlags = getTypeKindFlags();
	ASSERT(typeKindFlags & TypeKindFlag_ErrorCode);

	if (m_typeKind == TypeKind_Bool || !(typeKindFlags & TypeKindFlag_Integer))
		return getZeroValue();

	Value errorCodeValue;
	uint64_t minusOne = -1;
	errorCodeValue.createConst(&minusOne, this);
	return errorCodeValue;
}

ArrayType*
Type::getArrayType(size_t elementCount) {
	return m_module->m_typeMgr.getArrayType(this, elementCount);
}

DataPtrType*
Type::getDataPtrType(
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getDataPtrType(this, typeKind, ptrTypeKind, flags);
}

FunctionArg*
Type::getSimpleFunctionArg(uint_t ptrTypeFlags) {
	return m_module->m_typeMgr.getSimpleFunctionArg(this, ptrTypeFlags);
}

bool
Type::prepareImports() {
	ASSERT(!(m_flags & (ModuleItemFlag_LayoutReady | TypeFlag_NoImports)));

	bool result = resolveImports();
	if (!result)
		return false;

	m_flags |= TypeFlag_NoImports;
	return true;
}

bool
Type::prepareLayout() {
	ASSERT(!(m_flags & ModuleItemFlag_LayoutReady));

	if (m_flags & ModuleItemFlag_InCalcLayout) {
		ModuleItemDecl* decl = getDecl();
		ASSERT(decl); // recursion is only possible with named types

		err::setFormatStringError("can't calculate layout of '%s' due to recursion", decl->getQualifiedName().sz());
		return false;
	}

	m_flags |= ModuleItemFlag_InCalcLayout;

	bool result = calcLayout();
	if (!result) {
		m_flags &= ~ModuleItemFlag_InCalcLayout;
		return false;
	}

	m_flags |= ModuleItemFlag_LayoutReady; // no need to clear ModuleItemFlag_InCalcLayout
	return true;
}

void
Type::prepareTypeString() {
	static const char* stringTable[TypeKind__PrimitiveTypeCount] = {
		"void",
		"variant_t",
		"string_t",
		"bool",
		"char",
		"unsigned char",
		"short",
		"unsigned short",
		"int",
		"unsigned int",
		"long",
		"unsigned long",
		"bigendian short",
		"bigendian unsigned short",
		"bigendian int",
		"bigendian unsigned int",
		"bigendian long",
		"bigendian unsigned long",
		"float",
		"double",
	};

	ASSERT(m_typeKind < TypeKind__PrimitiveTypeCount);
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_typeStringPrefix = stringTable[m_typeKind];
}

void
Type::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyLinkedTextPrefix = getTypeStringPrefix();
	tuple->m_doxyLinkedTextSuffix = getTypeStringSuffix();
}

void
Type::prepareDoxyTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyTypeString = "<type>";
	tuple->m_doxyTypeString += getDoxyLinkedTextPrefix();
	tuple->m_doxyTypeString += "</type>\n";

	AXL_TODO("add compile-option for whether to use doxy-linked-text instead of plain-text")

	sl::String suffix = getTypeStringSuffix();
	if (!suffix.isEmpty()) { // suffix should be ready by now
		tuple->m_doxyTypeString += "<argsstring>";
		tuple->m_doxyTypeString += suffix;
		tuple->m_doxyTypeString += "</argsstring>\n";
	}
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Type*
getLlvmType_void(Module* module) {
	return llvm::Type::getVoidTy(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_variant(Module* module) {
	return module->m_typeMgr.getStdType(StdType_VariantStruct)->getLlvmType();
}

llvm::Type*
getLlvmType_string(Module* module) {
	return module->m_typeMgr.getStdType(StdType_StringStruct)->getLlvmType();
}

llvm::Type*
getLlvmType_bool(Module* module) {
	return llvm::Type::getInt1Ty(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_int8(Module* module) {
	return llvm::Type::getInt8Ty(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_int16(Module* module) {
	return llvm::Type::getInt16Ty(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_int32(Module* module) {
	return llvm::Type::getInt32Ty(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_int64(Module* module) {
	return llvm::Type::getInt64Ty(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_float(Module* module) {
	return llvm::Type::getFloatTy(*module->getLlvmContext());
}

llvm::Type*
getLlvmType_double(Module* module) {
	return llvm::Type::getDoubleTy(*module->getLlvmContext());
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::DIType_vn
getLlvmDiType_void(Module* module) {
	ASSERT(false); // shouldn't ever happen
	return module->m_llvmDiBuilder.createBasicType("void", 1, 1, llvm::dwarf::DW_ATE_boolean);
}

template <StdType stdType>
llvm::DIType_vn
getLlvmDiType_struct(Module* module) {
	return module->m_typeMgr.getStdType(stdType)->getLlvmDiType();
}

template <
	const char* name,
	uint_t code,
	size_t size
>
llvm::DIType_vn
getLlvmDiType_simple(Module* module) {
	return module->m_llvmDiBuilder.createBasicType(name, size, size, code);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
Type::prepareLlvmType() {
	ASSERT(!m_llvmType && (size_t)m_typeKind < TypeKind__PrimitiveTypeCount);

	typedef llvm::Type* GetLlvmTypeFunc(Module* module);

	GetLlvmTypeFunc* getLlvmTypeFuncTable[TypeKind__PrimitiveTypeCount] = {
		getLlvmType_void,    // TypeKind_Void
		getLlvmType_variant, // TypeKind_Variant
		getLlvmType_string,  // TypeKind_String
		getLlvmType_bool,    // TypeKind_Bool
		getLlvmType_int8,    // TypeKind_Int8
		getLlvmType_int8,    // TypeKind_Int8_u
		getLlvmType_int16,   // TypeKind_Int16
		getLlvmType_int16,   // TypeKind_Int16_u
		getLlvmType_int32,   // TypeKind_Int32
		getLlvmType_int32,   // TypeKind_Int32_u
		getLlvmType_int64,   // TypeKind_Int64
		getLlvmType_int64,   // TypeKind_Int64_u
		getLlvmType_int16,   // TypeKind_Int16_be
		getLlvmType_int16,   // TypeKind_Int16_ube
		getLlvmType_int32,   // TypeKind_Int32_be
		getLlvmType_int32,   // TypeKind_Int32_ube
		getLlvmType_int64,   // TypeKind_Int64_be
		getLlvmType_int64,   // TypeKind_Int64_ube
		getLlvmType_float,   // TypeKind_Float
		getLlvmType_double,  // TypeKind_Double
	};

	m_llvmType = getLlvmTypeFuncTable[(size_t)m_typeKind](m_module);
}

void
Type::prepareLlvmDiType() {
	ASSERT(m_typeKind && m_typeKind < TypeKind__PrimitiveTypeCount);

	typedef llvm::DIType_vn GetLlvmDiTypeFunc(Module* module);

	static char name_bool[]      = "bool";
	static char name_int8[]      = "char";
	static char name_int8_u[]    = "unsigned char";
	static char name_int16[]     = "short";
	static char name_int16_u[]   = "unsigned short";
	static char name_int32[]     = "int";
	static char name_int32_u[]   = "unsigned int";
	static char name_int64[]     = "long";
	static char name_int64_u[]   = "unsigned long";
	static char name_int16_be[]  = "bigendian short";
	static char name_int16_ube[] = "bigendian unsigned short";
	static char name_int32_be[]  = "bigendian int";
	static char name_int32_ube[] = "bigendian unsigned int";
	static char name_int64_be[]  = "bigendian long";
	static char name_int64_ube[] = "bigendian unsigned long";
	static char name_float[]     = "float";
	static char name_double[]    = "double";

	GetLlvmDiTypeFunc* getLlvmDiTypeFuncTable[TypeKind__PrimitiveTypeCount] = {
		getLlvmDiType_void,                          // TypeKind_Void,
		getLlvmDiType_struct<StdType_VariantStruct>, // TypeKind_Variant,
		getLlvmDiType_struct<StdType_StringStruct>,  // TypeKind_String,

		// TypeKind_Bool,
		&getLlvmDiType_simple<
			name_bool,
			llvm::dwarf::DW_ATE_boolean,
			1
		>,

		// TypeKind_Int8,
		&getLlvmDiType_simple<
			name_int8,
			llvm::dwarf::DW_ATE_signed_char,
			1
		>,

		// TypeKind_Int8_u,
		&getLlvmDiType_simple<
			name_int8_u,
			llvm::dwarf::DW_ATE_unsigned_char,
			1
		>,

		// TypeKind_Int16,
		&getLlvmDiType_simple<
			name_int16,
			llvm::dwarf::DW_ATE_signed,
			2
		>,

		// TypeKind_Int16_u,
		&getLlvmDiType_simple<
			name_int16_u,
			llvm::dwarf::DW_ATE_unsigned,
			2
		>,

		// TypeKind_Int32,
		&getLlvmDiType_simple<
			name_int32,
			llvm::dwarf::DW_ATE_signed,
			4
		>,

		// TypeKind_Int32_u,
		&getLlvmDiType_simple<
			name_int32_u,
			llvm::dwarf::DW_ATE_unsigned,
			4
		>,

		// TypeKind_Int64,
		&getLlvmDiType_simple<
			name_int64,
			llvm::dwarf::DW_ATE_signed,
			8
		>,

		// TypeKind_Int64_u,
		&getLlvmDiType_simple<
			name_int64_u,
			llvm::dwarf::DW_ATE_unsigned,
			8
		>,

		// TypeKind_Int16_be,
		&getLlvmDiType_simple<
			name_int16_be,
			llvm::dwarf::DW_ATE_signed,
			2
		>,

		// TypeKind_Int16_ube,
		&getLlvmDiType_simple<
			name_int16_ube,
			llvm::dwarf::DW_ATE_unsigned,
			2
		>,

		// TypeKind_Int32_be,
		&getLlvmDiType_simple<
			name_int32_be,
			llvm::dwarf::DW_ATE_signed,
			4
		>,

		// TypeKind_Int32_ube,
		&getLlvmDiType_simple<
			name_int32_ube,
			llvm::dwarf::DW_ATE_unsigned,
			4
		>,

		// TypeKind_Int64_be,
		&getLlvmDiType_simple<
			name_int64_be,
			llvm::dwarf::DW_ATE_signed,
			8
		>,

		// TypeKind_Int64_ube,
		&getLlvmDiType_simple<
			name_int64_ube,
			llvm::dwarf::DW_ATE_unsigned,
			8
		>,

		// TypeKind_Float,
		&getLlvmDiType_simple<
			name_float,
			llvm::dwarf::DW_ATE_float,
			4
		>,

		// TypeKind_Double,
		&getLlvmDiType_simple<
			name_double,
			llvm::dwarf::DW_ATE_float,
			8
		>
	};

	m_llvmDiType = getLlvmDiTypeFuncTable[(size_t)m_typeKind](m_module);
}

void
Type::prepareSimpleTypeVariable(StdType stdType) {
	ASSERT(!m_typeVariable && m_module->getCompileState() < ModuleCompileState_Compiled);

	sl::String qualifiedName = "jnc.g_type_" + getSignature();
	Type* type = m_module->m_typeMgr.getStdType(stdType);

	sl::List<Token> constructor;
	Token* token = new Token;
	constructor.insertTail(token);
	token->m_token = TokenKind_Integer;
	token->m_data.m_int64_u = (intptr_t)this;

	m_typeVariable = m_module->m_variableMgr.createVariable(
		StorageKind_Static,
		sl::String(),
		qualifiedName,
		type,
		0,
		&constructor
	);

	m_typeVariable->m_parentNamespace = m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc);
	m_typeVariable->m_flags |= VariableFlag_Type;

	bool result = m_module->m_variableMgr.allocateVariable(m_typeVariable);
	ASSERT(result);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

sl::StringRef
getValueString_void(
	const void* p,
	const char* formatSpec
) {
	return "void";
}

sl::StringRef
getValueString_variant(
	const void* p,
	const char* formatSpec
) {
	return ((Variant*)p)->m_type ?
		sl::StringRef(((Variant*)p)->m_type->getValueString(p, formatSpec)) :
		sl::StringRef("null");
}

sl::StringRef
getValueString_string(
	const void* p,
	const char* formatSpec
) {
	const String* string = (String*)p;
	sl::StringRef stringRef((char*)string->m_ptr.m_p, string->m_length);
	return formatSpec ? sl::StringRef(sl::formatString(formatSpec, stringRef.sz())) : stringRef;
}

sl::StringRef
getValueString_bool(
	const void* p,
	const char* formatSpec
) {
	return
		formatSpec ? sl::StringRef(sl::formatString(formatSpec, *(bool*)p)) :
		*(bool*)p ? sl::StringRef("true") : sl::StringRef("false");
}

sl::StringRef
getValueString_int8(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%d", *(int8_t*)p);
}

sl::StringRef
getValueString_int8_u(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%u", *(uint8_t*)p);
}

sl::StringRef
getValueString_int16(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%d", *(int16_t*)p);
}

sl::StringRef
getValueString_int16_u(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%u", *(uint16_t*)p);
}

sl::StringRef
getValueString_int32(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%d", *(int32_t*)p);
}

sl::StringRef
getValueString_int32_u(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%u", *(uint32_t*)p);
}

sl::StringRef
getValueString_int64(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%lld", *(int64_t*)p);
}

sl::StringRef
getValueString_int64_u(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%llu", *(uint64_t*)p);
}

sl::StringRef
getValueString_int16_be(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%d", (int16_t)sl::swapByteOrder16(*(uint16_t*)p));
}

sl::StringRef
getValueString_int16_ube(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%u", (uint16_t)sl::swapByteOrder16(*(uint16_t*)p));
}

sl::StringRef
getValueString_int32_be(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%d", (int32_t)sl::swapByteOrder32(*(uint32_t*)p));
}

sl::StringRef
getValueString_int32_ube(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%u", (uint32_t)sl::swapByteOrder32(*(uint32_t*)p));
}

sl::StringRef
getValueString_int64_be(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%lld", (int64_t)sl::swapByteOrder64(*(uint64_t*)p));
}

sl::StringRef
getValueString_int64_ube(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%llu", (uint64_t)sl::swapByteOrder64(*(uint64_t*)p));
}

sl::StringRef
getValueString_float(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%f", *(float*)p);
}

sl::StringRef
getValueString_double(
	const void* p,
	const char* formatSpec
) {
	return sl::formatString(formatSpec ? formatSpec : "%f", *(double*)p);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

sl::StringRef
Type::getValueString(
	const void* p,
	const char* formatSpec
) {
	typedef
	sl::StringRef
	GetValueStringFunc(
		const void* p,
		const char* formatSpec
	);

	GetValueStringFunc* getValueStringFuncTable[TypeKind__PrimitiveTypeCount] = {
		getValueString_void,       // TypeKind_Void
		getValueString_variant,    // TypeKind_Variant
		getValueString_string,     // TypeKind_String
		getValueString_bool,       // TypeKind_Bool
		getValueString_int8,       // TypeKind_Int8
		getValueString_int8_u,     // TypeKind_Int8_u
		getValueString_int16,      // TypeKind_Int16
		getValueString_int16_u,    // TypeKind_Int16_u
		getValueString_int32,      // TypeKind_Int32
		getValueString_int32_u,    // TypeKind_Int32_u
		getValueString_int64,      // TypeKind_Int64
		getValueString_int64_u,    // TypeKind_Int64_u
		getValueString_int16_be,   // TypeKind_Int16_be
		getValueString_int16_ube,  // TypeKind_Int16_ube
		getValueString_int32_be,   // TypeKind_Int32_be
		getValueString_int32_ube,  // TypeKind_Int32_ube
		getValueString_int64_be,   // TypeKind_Int64_be
		getValueString_int64_ube,  // TypeKind_Int64_ube
		getValueString_float,      // TypeKind_Float
		getValueString_double,     // TypeKind_Double
	};

	return (size_t)m_typeKind < TypeKind__PrimitiveTypeCount ?
		getValueStringFuncTable[(size_t)m_typeKind](p, formatSpec) :
		"<unsupported-type>";
}

typedef
void
MarkGcRootsFunc(
	const void* p,
	rt::GcHeap* gcHeap
);

void
markGcRoots_variant(
	const void* p,
	rt::GcHeap* gcHeap
) {
	gcHeap->markVariant(*(Variant*)p);
}

void
markGcRoots_string(
	const void* p,
	rt::GcHeap* gcHeap
) {
	gcHeap->markString(*(String*)p);
}

void
Type::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	static MarkGcRootsFunc* funcTable[2] = {
		markGcRoots_variant,
		markGcRoots_string
	};

	size_t i = m_typeKind - TypeKind_Variant;
	ASSERT(i < countof(funcTable));
	funcTable[i](p, gcHeap);
}

//..............................................................................

void
NamedType::prepareDoxyLinkedText() {
	if (!m_parentUnit || m_parentUnit->getLib()) { // don't reference imported libraries
		Type::prepareDoxyLinkedText();
		return;
	}

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);
	sl::String refId = doxyBlock->getRefId();
	getTypeStringTuple()->m_doxyLinkedTextPrefix.format(
		"<ref refid=\"%s\">%s</ref>",
		refId.sz(),
		getQualifiedName().sz()
	);
}

//..............................................................................

Typedef::Typedef() {
	m_itemKind = ModuleItemKind_Typedef;
	m_type = NULL;
	m_shadowType = NULL;
}

TypedefShadowType*
Typedef::getShadowType() {
	if (!m_shadowType)
		m_shadowType = m_module->m_typeMgr.createTypedefShadowType(this);

	return m_shadowType;
}

bool
Typedef::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	bool result = m_type->ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format(
		"<memberdef kind='typedef' id='%s'>\n"
		"<name>%s</name>\n",
		doxyBlock->getRefId().sz(),
		m_name.sz()
	);

	itemXml->append(m_type->getDoxyTypeString());
	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
TypedefShadowType::calcLayout() {
	bool result = m_typedef->getType()->ensureLayout();
	if (!result)
		return false;

	Type* type = m_typedef->getType(); // fetch type *after* layout (due to potential named types fixups)
	m_flags |= (type->getFlags() & TypeFlag_Pod);
	m_size = type->getSize();
	m_alignment = type->getAlignment();
	return true;
}

void
TypedefShadowType::prepareDoxyLinkedText() {
	Unit* unit = m_typedef->getParentUnit();
	if (!unit || unit->getLib()) { // don't reference imported libraries
		Type::prepareDoxyLinkedText();
		return;
	}

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(m_typedef);
	sl::String refId = doxyBlock->getRefId();
	getTypeStringTuple()->m_doxyLinkedTextPrefix.format(
		"<ref refid=\"%s\">%s</ref>",
		refId.sz(),
		getQualifiedName().sz()
	);
}

//..............................................................................

Type*
getSimpleType(
	TypeKind typeKind,
	Module* module
) {
	return module->m_typeMgr.getPrimitiveType(typeKind);
}

Type*
getSimpleType(
	StdType stdType,
	Module* module
) {
	return module->m_typeMgr.getStdType(stdType);
}

Type*
getDirectRefType(
	Type* type,
	uint_t ptrTypeFlags
) {
	return type->getTypeKind() == TypeKind_Class ?
		(Type*)((ClassType*)type)->getClassPtrType(
			TypeKind_ClassRef,
			ClassPtrTypeKind_Normal,
			ptrTypeFlags
		) :
		(Type*)type->getDataPtrType(
			TypeKind_DataRef,
			DataPtrTypeKind_Lean,
			ptrTypeFlags
		);
}

//..............................................................................

bool
isDisposableType(Type* type) {
	if (type->getTypeKindFlags() & TypeKindFlag_ClassPtr)
		return isDisposableType((DerivableType*)((ClassPtrType*)type)->getTargetType());

	if (type->getTypeKindFlags() & TypeKindFlag_DataPtr)
		type = ((DataPtrType*)type)->getTargetType();

	return
		(type->getTypeKindFlags() & TypeKindFlag_Derivable) &&
		isDisposableType((DerivableType*)type);
}

bool
isDisposableType(DerivableType* type) {
	if (!type->ensureLayout())
		return false;

	FindModuleItemResult findResult = type->findDirectChildItemTraverse("dispose");
	if (!findResult.m_item || findResult.m_item->getItemKind() != ModuleItemKind_Function)
		return false;

	FunctionType* functionType = ((Function*)findResult.m_item)->getType();
	return
		functionType->ensureLayout() &&
		functionType->getArgArray().getCount() == 1 &&
		functionType->isMemberMethodType();
}

bool
isStringableType(Type* type) {
	if (type->getTypeKindFlags() & TypeKindFlag_ClassPtr)
		return isStringableType((DerivableType*)((ClassPtrType*)type)->getTargetType());

	if (type->getTypeKindFlags() & TypeKindFlag_DataPtr)
		type = ((DataPtrType*)type)->getTargetType();

	return
		(type->getTypeKindFlags() & TypeKindFlag_Derivable) &&
		isStringableType((DerivableType*) type);
}

bool
isStringableType(DerivableType* type) {
	if (!type->ensureLayout())
		return false;

	FindModuleItemResult findResult = type->findDirectChildItemTraverse("toString");
	if (!findResult.m_item || findResult.m_item->getItemKind() != ModuleItemKind_Function)
		return false;

	FunctionType* functionType = ((Function*)findResult.m_item)->getType();
	return
		functionType->ensureLayout() &&
		functionType->getReturnType()->getTypeKind() == TypeKind_String &&
		functionType->getArgArray().getCount() == 1 &&
		functionType->isMemberMethodType();
}

bool
isSafePtrType(Type* type) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_Ptr) &&
		(type->getFlags() & PtrTypeFlag_Safe);
}

bool
isWeakPtrType(Type* type) {
	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_ClassPtr:
		return ((ClassPtrType*)type)->getPtrTypeKind() == ClassPtrTypeKind_Weak;

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*)type)->getPtrTypeKind() == FunctionPtrTypeKind_Weak;

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*)type)->getPtrTypeKind() == PropertyPtrTypeKind_Weak;

	default:
		return false;
	}
}

Type*
getWeakPtrType(Type* type) {
	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_ClassPtr:
		return ((ClassPtrType*)type)->getWeakPtrType();

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*)type)->getWeakPtrType();

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*)type)->getWeakPtrType();

	default:
		return type;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
