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

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

class FunctionType;

//..............................................................................

enum OrphanKind
{
	OrphanKind_Undefined,
	OrphanKind_Function,
	OrphanKind_Reactor
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Orphan:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public FunctionName
{
	friend class NamespaceMgr;
	friend class Namespace;
	friend class Parser;

protected:
	OrphanKind m_orphanKind;
	QualifiedName m_declaratorName;
	FunctionType* m_functionType;
	ModuleItem* m_origin;
	UsingSet m_usingSet;

public:
	Orphan();

	OrphanKind
	getOrphanKind()
	{
		return m_orphanKind;
	}

	const QualifiedName&
	getDeclaratorName()
	{
		return m_declaratorName;
	}

	FunctionType*
	getFunctionType()
	{
		return m_functionType;
	}

	ModuleItem*
	getOrigin() // after adopted
	{
		return m_origin;
	}

	void
	addUsingSet(Namespace* anchorNamespace);

	bool
	adopt(ModuleItem* item);

	ModuleItem*
	resolveForCodeAssist()
	{
		return resolveForCodeAssist(m_parentNamespace);
	}

protected:
	ModuleItem*
	resolveForCodeAssist(Namespace* nspace);

	bool
	adoptOrphanFunction(ModuleItem* item);

	bool
	adoptOrphanReactor(ModuleItem* item);

	bool
	verifyStorageKind(ModuleItemDecl* targetDecl);

	void
	copySrcPos(ModuleItemDecl* targetDecl);

	bool
	copyArgNames(FunctionType* targetFunctionType);

	OverloadableFunction
	getItemUnnamedMethod(ModuleItem* item);
};

//..............................................................................

} // namespace ct
} // namespace jnc
