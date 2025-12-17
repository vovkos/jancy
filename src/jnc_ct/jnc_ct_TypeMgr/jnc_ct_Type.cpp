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
	stream.flush();
	return sl::String(string.data(), string.length());
}

//..............................................................................

const char*
getPtrTypeFlagString(PtrTypeFlag flag) {
	static const char* stringTable[] = {
		"safe",       // PtrTypeFlag_Safe       = 0x0010000
		"const",      // PtrTypeFlag_Const      = 0x0020000
		"readonly",   // PtrTypeFlag_ReadOnly   = 0x0040000
		"cmut",       // PtrTypeFlag_CMut       = 0x0080000
		"volatile",   // PtrTypeFlag_Volatile   = 0x0100000
		"event",      // PtrTypeFlag_Event      = 0x0200000
		"dualevent",  // PtrTypeFlag_DualEvent  = 0x0400000
		"bindable",   // PtrTypeFlag_Bindable   = 0x0800000
		"autoget",    // PtrTypeFlag_AutoGet    = 0x1000000
		"dualtarget", // PtrTypeFlag_DualTarget = 0x2000000, // data ptr only
		"bigendian",  // PtrTypeFlag_BigEndian  = 0x4000000, // data ptr only
		"bitfield",   // PtrTypeFlag_BitField   = 0x8000000, // data ptr only
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
		"t",  // PtrTypeFlag_DualTarget = 0x2000000, // data ptr only
		"n",  // PtrTypeFlag_BigEndian  = 0x4000000, // data ptr only
		"f",  // PtrTypeFlag_BitField   = 0x8000000, // data ptr only
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

	if (modifiers & TypeModifier_BigEndian)
		flags |= PtrTypeFlag_BigEndian;

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

bool
Type::prepareImports() {
	ASSERT(!(m_flags & (TypeFlag_LayoutReady | TypeFlag_NoImports)));

	bool result = resolveImports();
	if (!result)
		return false;

	m_flags |= TypeFlag_NoImports;
	return true;
}

bool
Type::prepareLayout() {
	ASSERT(!(m_flags & TypeFlag_LayoutReady));

	if (m_flags & TypeFlag_InCalcLayout) {
		err::setFormatStringError("can't calculate layout of '%s' due to recursion", getItemName().sz());
		return false;
	}

	m_flags |= TypeFlag_InCalcLayout;
	bool result = calcLayout();
	if (!result) {
		m_flags &= ~TypeFlag_InCalcLayout;
		return false;
	}

	m_flags |= TypeFlag_LayoutReady; // no need to clear TypeFlag_InCalcLayout
	return true;
}

sl::StringRef
Type::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
		break;

	case TypeStringKind_TypeName: {
		sl::StringRef prefix = getTypeStringPrefix();
		sl::StringRef suffix = getTypeStringSuffix();
		return suffix.isEmpty() ? prefix : prefix + ' ' + suffix;
		}

	case ModuleItemStringKind_Synopsis:
		return getTypeString();

	case TypeStringKind_DoxyLinkedTextPrefix:
		return getTypeStringPrefix();

	case TypeStringKind_DoxyLinkedTextSuffix:
		return getTypeStringSuffix();

	case TypeStringKind_DoxyTypeString: {
		sl::String  string = "<type>";
		string += getDoxyLinkedTextPrefix();
		string += "</type>\n";

		AXL_TODO("add compile-option for whether to use doxy-linked-text instead of plain-text")

		sl::String suffix = getDoxyLinkedTextSuffix();
		if (!suffix.isEmpty()) { // suffix should be ready by now
			string += "<argsstring>";
			string += suffix;
			string += "</argsstring>\n";
		}

		return string;
		}

	default:
		return sl::StringRef();
	}

	static sl::StringRef stringTable[TypeKind__PrimitiveTypeCount] = {
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
		"float",
		"double",
	};

	ASSERT(m_typeKind < TypeKind__PrimitiveTypeCount);
	return stringTable[m_typeKind];
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
		getLlvmType_float,   // TypeKind_Float
		getLlvmType_double,  // TypeKind_Double
	};

	m_llvmType = getLlvmTypeFuncTable[(size_t)m_typeKind](m_module);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
Type::prepareLlvmDiType() {
		ASSERT(m_typeKind && m_typeKind < TypeKind__PrimitiveTypeCount);

	struct DiTypeInfo {
		const char* m_name;
		uint_t m_dwarfCode;
		size_t m_size;
	};

	static const DiTypeInfo typeInfoTable[] = {
		{
			"void",                     // TypeKind_Void
			llvm::dwarf::DW_ATE_boolean,
			1,
		},
		{ 0 },                        // TypeKind_Variant
		{ 0 },                        // TypeKind_String
		{                             // TypeKind_Bool
			"bool",
			llvm::dwarf::DW_ATE_boolean,
			1,
		},
		{
			"char",                     // TypeKind_Int8
			llvm::dwarf::DW_ATE_signed_char,
			1
		},
		{
			"unsigned char",            // TypeKind_Int8_u
			llvm::dwarf::DW_ATE_unsigned_char,
			1
		},
		{
			"short",                    // TypeKind_Int16
			llvm::dwarf::DW_ATE_signed,
			2
		},
		{
			"unsigned short",           // TypeKind_Int16_u
			llvm::dwarf::DW_ATE_unsigned,
			2
		},
		{
			"int",                      // TypeKind_Int32
			llvm::dwarf::DW_ATE_signed,
			4
		},
		{
			"unsigned int",             // TypeKind_Int32_u
			llvm::dwarf::DW_ATE_unsigned,
			4
		},
		{
			"long",                     // TypeKind_Int64
			llvm::dwarf::DW_ATE_signed,
			8
		},
		{
			"unsigned long",            // TypeKind_Int64_u
			llvm::dwarf::DW_ATE_unsigned,
			8
		},
		{
			"float",                    // TypeKind_Float
			llvm::dwarf::DW_ATE_float,
			4
		},
		{
			"double",                   // TypeKind_Double
			llvm::dwarf::DW_ATE_float,
			8
		},
	};

	switch (m_typeKind) {
		case TypeKind_Variant:
			m_llvmDiType = m_module->m_typeMgr.getStdType(StdType_VariantStruct)->getLlvmDiType();
			break;

		case TypeKind_String:
			m_llvmDiType = m_module->m_typeMgr.getStdType(StdType_StringStruct)->getLlvmDiType();
			break;

		default:
			const DiTypeInfo& diTypeInfo = typeInfoTable[m_typeKind];
			m_llvmDiType = m_module->m_llvmDiBuilder.createBasicType(
				diTypeInfo.m_name,
				diTypeInfo.m_size,
				diTypeInfo.m_size,
				diTypeInfo.m_dwarfCode
			);
	};
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

bool
NamedType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (!m_templateInstance)
		return true; // can't deduce from a non-templated class

	if (referenceType->getTypeKind() != m_typeKind) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	TemplateInstance* referenceInstance = ((DerivableType*)referenceType)->getTemplateInstance();
	if (!referenceInstance ||
		referenceInstance->m_template != m_templateInstance->m_template
	) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	bool result = true;
	size_t argCount = m_templateInstance->m_argArray.getCount();
	for (size_t i = 0; i < argCount; i++)
		result = m_templateInstance->m_argArray[i]->deduceTemplateArgs(
			templateArgTypeArray,
			referenceInstance->m_argArray[i]
		) && result;

	return true;
}

sl::StringRef
NamedType::createLinkId() {
	if (!m_templateInstance)
		return createLinkIdImpl(m_module);

	sl::String linkId = createLinkIdImpl(m_module);
	m_templateInstance->appendArgLinkId(&linkId);
	return linkId;
}

sl::StringRef
NamedType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix: {
		if (!m_templateInstance)
			return createQualifiedNameImpl(m_module);

		sl::String string = createQualifiedNameImpl(m_module);
		m_templateInstance->appendArgString(&string);
		return string;
		}

	case ModuleItemStringKind_Synopsis:
		break;

	case TypeStringKind_DoxyLinkedTextPrefix:
		if (m_parentUnit && !m_parentUnit->getLib()) // don't reference imported libraries
			return sl::formatString(
				"<ref refid=\"%s\">%s</ref>",
				m_module->m_doxyHost.getItemBlock(this)->getRefId().sz(),
				getItemName().sz()
			);

		// else fall through

	default:
		return Type::createItemString(index);
	}

	static const sl::StringRef typeKindStringTable[] = {
		"enum",            // TypeKind_Enum,
		"struct",          // TypeKind_Struct,
		"union",           // TypeKind_Union,
		"class",           // TypeKind_Class,
	};

	sl::String synopsis;

	switch (m_typeKind) {
	case TypeKind_Enum:
		synopsis = getEnumTypeFlagString(m_flags);
		if (!synopsis.isEmpty())
			synopsis += ' ';
		break;

	case TypeKind_Class:
		if (m_flags & ClassTypeFlag_Opaque)
			synopsis = "opaque ";
		break;
	}

	size_t i = m_typeKind - TypeKind_Enum;
	ASSERT(i < countof(typeKindStringTable));

	synopsis += typeKindStringTable[i];
	synopsis += ' ';
	synopsis += getItemName();
	return synopsis;
}


void
NamedType::prepareSignature() {
	static const char prefixTable[] = {
		'E', // TypeKind_Enum
		'S', // TypeKind_Struct
		'U', // TypeKind_Union
		'C', // TypeKind_Class
	};

	size_t i = m_typeKind - TypeKind_Enum;
	ASSERT(i < countof(prefixTable));

	const sl::StringRef& linkId = getLinkId();
	sl::String signature;
	signature.reserve(linkId.getLength() + 2);
	signature = prefixTable[i];
	signature += linkId;
	signature += '$';
	m_signature = std::move(signature);
	m_flags |= TypeFlag_SignatureFinal;
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
		isStringableType((DerivableType*)type);
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
