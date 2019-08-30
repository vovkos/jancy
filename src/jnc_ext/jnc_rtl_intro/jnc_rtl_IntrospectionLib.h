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

#pragma once

namespace jnc {
namespace rtl {

//..............................................................................

class ModuleItem;
class ModuleItemDecl;
class Attribute;
class AttributeBlock;
class Namespace;
class GlobalNamespace;
class Type;
class DataPtrType;
class NamedType;
class NamedTypeBlock;
class BaseTypeSlot;
class DerivableType;
class ArrayType;
class BitFieldType;
class FunctionArg;
class FunctionType;
class FunctionPtrType;
class PropertyType;
class PropertyPtrType;
class EnumConst;
class EnumType;
class ClassType;
class ClassPtrType;
class StructField;
class StructType;
class UnionType;
class Alias;
class Variable;
class Function;
class Property;
class Module;
class Unit;

//..............................................................................

IfaceHdr*
JNC_CDECL
getIntrospectionClass(
	void* item,
	StdType stdType
	);

ModuleItem*
JNC_CDECL
getModuleItem(ct::ModuleItem* item);

inline
AttributeBlock*
getAttributeBlock(ct::AttributeBlock* block)
{
	return (AttributeBlock*)getIntrospectionClass(block, StdType_AttributeBlock);
}

inline
Attribute*
getAttribute(ct::Attribute* attribute)
{
	return (Attribute*)getIntrospectionClass(attribute, StdType_Attribute);
}

inline
BaseTypeSlot*
getBaseTypeSlot(ct::BaseTypeSlot* slot)
{
	return (BaseTypeSlot*)getIntrospectionClass(slot, StdType_BaseTypeSlot);
}

Type*
JNC_CDECL
getType(ct::Type* type);

inline
EnumConst*
getEnumConst(ct::EnumConst* cnst)
{
	return (EnumConst*)getIntrospectionClass(cnst, StdType_EnumConst);
}

inline
StructField*
JNC_CDECL
getStructField(ct::StructField* field)
{
	return (StructField*)getIntrospectionClass(field, StdType_StructField);
}

inline
Variable*
getVariable(ct::Variable* function)
{
	return (Variable*)getIntrospectionClass(function, StdType_Variable);
}

inline
Function*
getFunction(ct::Function* function)
{
	return (Function*)getIntrospectionClass(function, StdType_Function);
}

inline
FunctionArg*
getFunctionArg(ct::FunctionArg* arg)
{
	return (FunctionArg*)getIntrospectionClass(arg, StdType_FunctionArg);
}

inline
Property*
getProperty(ct::Property* property)
{
	return (Property*)getIntrospectionClass(property, StdType_Property);
}

Namespace*
getNamespace(ct::Namespace* nspace);

inline
GlobalNamespace*
getGlobalNamespace(ct::GlobalNamespace* nspace)
{
	return (GlobalNamespace*)getIntrospectionClass(nspace, StdType_GlobalNamespace);
}

inline
Module*
getModule(ct::Module* module)
{
	return (Module*)getIntrospectionClass(module, StdType_Module);
}

inline
Unit*
getUnit(ct::Unit* unit)
{
	return (Unit*)getIntrospectionClass(unit, StdType_Unit);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
