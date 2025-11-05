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
#include "jnc_rtl_Type.h"
#include "jnc_Construct.h"
#include "jnc_ct_Variable.h"
#include "jnc_ct_DerivableType.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Type,
	"jnc.Type",
	sl::g_nullGuid,
	-1,
	Type,
	&Type::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Type)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Type, ct::Type*>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Type>)
	JNC_MAP_CONST_PROPERTY("m_typeKind", &Type::getTypeKind)
	JNC_MAP_CONST_PROPERTY("m_typeKindFlags", &Type::getTypeKindFlags)
	JNC_MAP_CONST_PROPERTY("m_size", &Type::getSize)
	JNC_MAP_CONST_PROPERTY("m_alignment", &Type::getAlignment)
	JNC_MAP_CONST_PROPERTY("m_signature", &Type::getSignature)
	JNC_MAP_CONST_PROPERTY("m_typeString", &Type::getTypeString)
	JNC_MAP_CONST_PROPERTY("m_typeStringPrefix", &Type::getTypeStringPrefix)
	JNC_MAP_CONST_PROPERTY("m_typeStringSuffix", &Type::getTypeStringSuffix)
	JNC_MAP_FUNCTION("cmp", &Type::cmp)
	JNC_MAP_FUNCTION("getArrayType", &Type::getArrayType)
	JNC_MAP_FUNCTION("getDataPtrType", &Type::getDataPtrType_0)
	JNC_MAP_OVERLOAD(&Type::getDataPtrType_1)
	JNC_MAP_FUNCTION("getValueString", &Type::getValueString_0)
	JNC_MAP_OVERLOAD(&Type::getValueString_1)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	NamedType,
	"jnc.NamedType",
	sl::g_nullGuid,
	-1,
	NamedType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(NamedType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<NamedType, ct::NamedType*>))
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DataPtrType,
	"jnc.DataPtrType",
	sl::g_nullGuid,
	-1,
	DataPtrType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DataPtrType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<DataPtrType, ct::DataPtrType*>))
	JNC_MAP_CONST_PROPERTY("m_ptrTypeKind", &DataPtrType::getPtrTypeKind)
	JNC_MAP_CONST_PROPERTY("m_targetType", &DataPtrType::getTargetType)
	JNC_MAP_CONST_PROPERTY("m_bitOffset", &DataPtrType::getBitOffset)
	JNC_MAP_CONST_PROPERTY("m_bitCount", &DataPtrType::getBitCount)
	JNC_MAP_FUNCTION("getTargetValueString", &DataPtrType::getTargetValueString)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Typedef,
	"jnc.Typedef",
	sl::g_nullGuid,
	-1,
	Typedef,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Typedef)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Typedef, ct::Typedef*>))
	JNC_MAP_CONST_PROPERTY("m_type", &Typedef::getType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
Type::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	Cache* cache = m_cache.get();
	if (!cache)
		return;

	gcHeap->markString(cache->m_signature);
	gcHeap->markString(cache->m_typeString);
	gcHeap->markString(cache->m_typeStringPrefix);
	gcHeap->markString(cache->m_typeStringSuffix);
}

String
JNC_CDECL
Type::getSignature(Type* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_signature.m_length)
		cache->m_signature = createForeignString(self->m_item->getSignature(), false);

	return cache->m_signature;
}

String
JNC_CDECL
Type::getTypeString(Type* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_typeString.m_length)
		cache->m_typeString = createForeignString(self->m_item->getTypeString(), false);

	return cache->m_typeString;
}

String
JNC_CDECL
Type::getTypeStringPrefix(Type* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_typeStringPrefix.m_length)
		cache->m_typeStringPrefix= createForeignString(self->m_item->getTypeStringPrefix(), false);

	return cache->m_typeStringPrefix;
}

String
JNC_CDECL
Type::getTypeStringSuffix(Type* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_typeStringSuffix.m_length)
		cache->m_typeStringSuffix= createForeignString(self->m_item->getTypeStringSuffix(), false);

	return cache->m_typeStringSuffix;
}

String
JNC_CDECL
Type::getValueString_0(
	Type* self,
	DataPtr valuePtr,
	String formatSpec
) {
	if (!valuePtr.m_p)
		return g_nullString;

	if (self->m_item->getTypeKindFlags() & TypeKindFlag_Derivable) {
		String string;
		bool result = tryGetStringableValueString(&string, (ct::DerivableType*)self->m_item, valuePtr);
		if (result)
			return string;
	}

	return allocateString(self->m_item->getValueString(valuePtr.m_p, (formatSpec >> toAxl).szn()));
}

String
JNC_CDECL
Type::getValueString_1(
	Type* self,
	Variant value,
	String formatSpec
) {
	char buffer[256];
	sl::Array<char> valueBuffer(rc::BufKind_Stack, buffer, sizeof(buffer));
	valueBuffer.setCount(self->m_item->getSize());

	bool result = value.cast(self->m_item, valueBuffer.p());
	return result ?
		allocateString(self->m_item->getValueString(valueBuffer, (formatSpec >> toAxl).szn())) :
		g_nullString;
}

bool
Type::tryGetStringableValueString(
	String* string,
	ct::DerivableType* type,
	DataPtr valuePtr
) {
	ct::MemberCoord coord;
	FindModuleItemResult findResult = type->findDirectChildItemTraverse("toString", &coord);
	if (!findResult.m_item || findResult.m_item->getItemKind() != ModuleItemKind_Function)
		return false;

	ct::Function* function = ((ct::Function*)findResult.m_item);
	ct::FunctionType* functionType = function->getType();
	void* p = function->getMachineCode();

	if (!p ||
		!functionType->isMemberMethodType() ||
		functionType->getReturnType()->getTypeKind() != TypeKind_String ||
		functionType->getArgArray().getCount() != 1
	)
		return false;

	*string = jnc::callFunctionImpl_u<String>(p, valuePtr);
	return true;
}

//..............................................................................

String
JNC_CDECL
DataPtrType::getTargetValueString(
	DataPtrType* self,
	DataPtr valuePtr,
	String formatSpec
) {
	return valuePtr.m_p ?
		allocateString(self->m_item->getTargetValueString(valuePtr.m_p, (formatSpec >> toAxl).szn())) :
		g_nullString;
}

//..............................................................................

Type*
JNC_CDECL
getType(ct::Type* type) {
	if (type->hasTypeVariable()) {
		ct::Variable* typeVar = type->getTypeVariable();
		return (Type*)((Box*)typeVar->getStaticData() + 1);
	}

	static StdType stdTypeTable[TypeKind__Count] = {
		StdType_Type,              // TypeKind_Void
		StdType_Type,              // TypeKind_Variant
		StdType_Type,              // TypeKind_String
		StdType_Type,              // TypeKind_Bool
		StdType_Type,              // TypeKind_Int8
		StdType_Type,              // TypeKind_Int8_u
		StdType_Type,              // TypeKind_Int16
		StdType_Type,              // TypeKind_Int16_u
		StdType_Type,              // TypeKind_Int32
		StdType_Type,              // TypeKind_Int32_u
		StdType_Type,              // TypeKind_Int64
		StdType_Type,              // TypeKind_Int64_u
		StdType_Type,              // TypeKind_Float
		StdType_Type,              // TypeKind_Double

		StdType_ArrayType,         // TypeKind_Array
		StdType_EnumType,          // TypeKind_Enum
		StdType_StructType,        // TypeKind_Struct
		StdType_UnionType,         // TypeKind_Union
		StdType_ClassType,         // TypeKind_Class
		StdType_FunctionType,      // TypeKind_Function
		StdType_PropertyType,      // TypeKind_Property
		StdType_DataPtrType,       // TypeKind_DataPtr
		StdType_DataPtrType,       // TypeKind_DataRef
		StdType_ClassPtrType,      // TypeKind_ClassPtr
		StdType_ClassPtrType,      // TypeKind_ClassRef
		StdType_FunctionPtrType,   // TypeKind_FunctionPtr
		StdType_FunctionPtrType,   // TypeKind_FunctionRef
		StdType_PropertyPtrType,   // TypeKind_PropertyPtr
		StdType_PropertyPtrType,   // TypeKind_PropertyRef

		// shouldn't happen at runtime

		StdType_Type,              // TypeKind_NamedImport,
		StdType_Type,              // TypeKind_ImportPtr,
		StdType_Type,              // TypeKind_ImportIntMod,
		StdType_Type,              // TypeKind_TypedefShadow,
		StdType_Type,              // TypeKind_TemplateArg,
		StdType_Type,              // TypeKind_TemplatePtr,
		StdType_Type,              // TypeKind_TemplateIntMod,
		StdType_Type,              // TypeKind_TemplateDecl,
	};

	TypeKind typeKind = type->getTypeKind();
	ASSERT((size_t)typeKind < countof(stdTypeTable));

	return (Type*)getIntrospectionClass(type, stdTypeTable[typeKind]);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
