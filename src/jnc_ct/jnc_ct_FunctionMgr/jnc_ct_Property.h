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

#include "jnc_ct_PropertyType.h"
#include "jnc_ct_PropertyVerifier.h"
#include "jnc_ct_Function.h"
#include "jnc_ct_MemberBlock.h"
#include "jnc_Property.h"

namespace jnc {
namespace ct {

//..............................................................................

class Property:
	public ModuleItem,
	public Namespace,
	public MemberBlock {
	friend class TypeMgr;
	friend class MemberBlock;
	friend class DerivableType;
	friend class ClassType;
	friend class ExtensionNamespace;
	friend class FunctionMgr;
	friend class Parser;

protected:
	class DefaultStaticConstructor: public CompilableFunction {
	public:
		DefaultStaticConstructor() {
			m_functionKind = FunctionKind_StaticConstructor;
			m_storageKind = StorageKind_Static;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileDefaultStaticConstructor();
		}
	};

	class DefaultConstructor: public CompilableFunction {
	public:
		DefaultConstructor() {
			m_functionKind = FunctionKind_Constructor;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileDefaultConstructor();
		}
	};

	class DefaultDestructor: public CompilableFunction {
	public:
		DefaultDestructor() {
			m_functionKind = FunctionKind_Destructor;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileDefaultDestructor();
		}
	};

	class AutoGetter: public CompilableFunction {
	public:
		AutoGetter() {
			m_functionKind = FunctionKind_Getter;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileAutoGetter();
		}
	};

	class AutoSetter: public CompilableFunction {
	public:
		AutoSetter() {
			m_functionKind = FunctionKind_Setter;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileAutoSetter();
		}
	};

	class Binder: public CompilableFunction {
	public:
		Binder() {
			m_functionKind = FunctionKind_Binder;
		}

		virtual
		bool
		compile() {
			return ((Property*)m_parentNamespace)->compileBinder();
		}
	};

protected:
	PropertyKind m_propertyKind;

	PropertyType* m_type;

	Function* m_getter;
	OverloadableFunction m_setter;
	Function* m_binder;

	// member data is Field or Variable

	ModuleItem* m_onChanged;
	ModuleItem* m_autoGetValue;

	// parent type

	DerivableType* m_parentType;
	size_t m_parentClassVtableIndex;
	sl::Array<Function*> m_vtable;
	Variable* m_vtableVariable;

	ExtensionNamespace* m_extensionNamespace;

	PropertyVerifier m_verifier;

public:
	Property();

	PropertyKind
	getPropertyKind() {
		return m_propertyKind;
	}

	PropertyType*
	getType() {
		return m_type;
	}

	Function*
	getGetter() {
		return m_getter;
	}

	OverloadableFunction
	getSetter() {
		return m_setter;
	}

	Function*
	getBinder() {
		return m_binder;
	}

	ModuleItem*
	getOnChanged() {
		return m_onChanged;
	}

	bool
	setOnChanged(
		ModuleItem* item,
		bool isForced = false
	);

	bool
	createOnChanged();

	ModuleItem*
	getAutoGetValue() {
		return m_autoGetValue;
	}

	bool
	setAutoGetValue(
		ModuleItem* item,
		bool isForced = false
	); // struct-field or variable

	bool
	createAutoGetValue(Type* type);

	DerivableType*
	getParentType() {
		return m_parentType;
	}

	bool
	isMember() {
		return m_parentType != NULL;
	}

	bool
	isVirtual() {
		return m_storageKind >= StorageKind_Abstract && m_storageKind <= StorageKind_Override;
	}

	size_t
	getParentClassVtableIndex() {
		return m_parentClassVtableIndex;
	}

	bool
	create(PropertyType* type);

	PropertyType*
	createType();

	virtual
	bool
	addMethod(Function* function);

	virtual
	bool
	addProperty(Property* prop);

	bool
	ensureVtable() {
		return m_vtable.isEmpty() ? prepareVtable() : true;
	}

	Variable*
	getVtableVariable() {
		return m_vtableVariable || createVtableVariable() ? m_vtableVariable : NULL;
	}

	bool
	finalize(); // resolves autoget/bindable aliases, lays out vtable, creates default constructors

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

protected:
	StorageKind
	getAccessorStorageKind() {
		return
			m_storageKind == StorageKind_Abstract ? StorageKind_Virtual :
			m_storageKind == StorageKind_Reactor ? StorageKind_Member :
			m_storageKind;
	}

	virtual
	Function*
	createAccessor(
		FunctionKind functionKind,
		FunctionType* type
	);

	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount,
		uint_t ptrTypeFlags,
		sl::List<Token>* constructor,
		sl::List<Token>* initializer
	);

	bool
	appendVtableMethod(Function* function) {
		return function->getType()->ensureLayout() && m_vtable.append(function) != -1;
	}

	bool
	prepareVtable();

	bool
	createVtableVariable();

	Value
	getAutoAccessorPropertyValue();

	bool
	compileDefaultStaticConstructor();

	bool
	compileDefaultConstructor();

	bool
	compileDefaultDestructor();

	bool
	compileAutoGetter();

	bool
	compileAutoSetter();

	bool
	compileBinder();
};

//..............................................................................

} // namespace ct
} // namespace jnc
