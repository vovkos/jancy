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

#include "jnc_ct_Namespace.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_LlvmIrInsertPoint.h"
#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

class BasicBlock;
class Variable;
class GcShadowStackFrameMap;

//..............................................................................

struct ScopeExtension {
	virtual ~ScopeExtension() {}
};

struct TryExpr: ScopeExtension {
	TryExpr* m_prev;
	BasicBlock* m_catchBlock;
	size_t m_sjljFrameIdx;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DynamicLayoutStmt: ScopeExtension {
	Value m_layoutValue; // jnc.DynamicLayout
	StructType* m_structType;
	BasicBlock* m_structBlock;
	sl::Array<Value> m_structSectionValueArray;
	size_t m_fieldAlignment;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ScopeFlag {
	ScopeFlag_Function       = 0x00000100,
	ScopeFlag_Unsafe         = 0x00000200,
	ScopeFlag_Nested         = 0x00000400,
	ScopeFlag_Else           = 0x00000800,
	ScopeFlag_Try            = 0x00001000,
	ScopeFlag_Catch          = 0x00002000,
	ScopeFlag_Finally        = 0x00004000,
	ScopeFlag_CatchAhead     = 0x00020000,
	ScopeFlag_FinallyAhead   = 0x00040000,
	ScopeFlag_Finalizable    = 0x00100000, // scope or one of its parents has finally
	ScopeFlag_Disposable     = 0x00200000, // this scope contains disposable variables
	ScopeFlag_HasCatch       = 0x00400000, // this scope or some of its parents have catch
	ScopeFlag_FrameMapCached = 0x00800000, // this scope or some of its parents have catch
	ScopeFlag_HasLandingPads = 0x10000000, // this function has landing pads
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Scope:
	public ModuleItem,
	public Namespace {
	friend class NamespaceMgr;
	friend class ControlFlowMgr;
	friend class FunctionMgr;
	friend class Parser;

protected:
	Function* m_function;
	Variable* m_disposeLevelVariable;
	sl::Array<Variable*> m_disposableVariableArray;
	llvm::DIScope_vn m_llvmDiScope;

public:
	BasicBlock* m_breakBlock;
	BasicBlock* m_continueBlock;
	BasicBlock* m_catchBlock;
	BasicBlock* m_finallyBlock;
	TryExpr* m_tryExpr;
	DynamicLayoutStmt* m_dynamicLayoutStmt;
	size_t m_sjljFrameIdx;

	LlvmIrInsertPoint m_gcShadowStackFrameMapInsertPoint;
	GcShadowStackFrameMap* m_gcShadowStackFrameMap;
	Variable* m_firstStackVariable; // we have to set frame map BEFORE the very first stack variable lift point
	Variable* m_regexMatchVariable; // for getting regex groups via $0, $1, etc

public:
	Scope();

	Function*
	getFunction() {
		return m_function;
	}

	Scope*
	getParentScope() {
		return m_parentNamespace && m_parentNamespace->getNamespaceKind() == NamespaceKind_Scope ? (Scope*)m_parentNamespace : NULL;
	}

	GcShadowStackFrameMap*
	findGcShadowStackFrameMap();

	Variable*
	getDisposeLevelVariable() {
		return m_disposeLevelVariable;
	}

	sl::Array<Variable*>
	getDisposableVariableArray() {
		return m_disposableVariableArray;
	}

	size_t
	addDisposableVariable(Variable* variable) {
		m_disposableVariableArray.append(variable);
		return m_disposableVariableArray.getCount();
	}

	llvm::DIScope_vn
	getLlvmDiScope() {
		return m_llvmDiScope;
	}

	bool
	canCatch() {
		return m_tryExpr != NULL || (m_flags & ScopeFlag_HasCatch);
	}

	bool
	canStaticThrow() {
		return canCatch() || (m_function->getType()->getFlags() & FunctionTypeFlag_ErrorCode);
	}

	BasicBlock*
	getCatchBlock() {
		return m_tryExpr ? m_tryExpr->m_catchBlock : m_catchBlock;
	}

	virtual
	sl::StringRef
	createQualifiedName(const sl::StringRef& name) {
		return name;
	}

protected:
	virtual
	bool
	parseBody() {
		ASSERT(false);
		return true;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
