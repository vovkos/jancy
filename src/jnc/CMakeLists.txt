# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

project_virtual_folder (jnc)

#..............................................................................
#
# callconv folder
#

set (
	CALLCONV_H_LIST
	${JNC_INC_DIR}/jnc_CallConv.h
	${JNC_INC_DIR}/jnc_CallConv_gcc32.h
	${JNC_INC_DIR}/jnc_CallConv_msc.h
	${JNC_INC_DIR}/jnc_CdeclCallConv_gcc64.h
	${JNC_INC_DIR}/jnc_JnccallCallConv.h
	)

set (
	CALLCONV_CPP_LIST
	jnc_CallConv.cpp
	jnc_CallConv_gcc32.cpp
	jnc_CallConv_msc.cpp
	jnc_CdeclCallConv_gcc64.cpp
	jnc_JnccallCallConv.cpp
	)

source_virtual_folder (
	callconv
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	)

#..............................................................................
#
# controlflow folder
#

set (
	CONTROLFLOW_H_LIST
	${JNC_INC_DIR}/jnc_BasicBlock.h
	${JNC_INC_DIR}/jnc_ControlFlowMgr.h
	)

set (
	CONTROLFLOW_CPP_LIST
	jnc_BasicBlock.cpp
	jnc_ControlFlowMgr.cpp
	jnc_ControlFlowMgr_Stmt.cpp
	)

source_virtual_folder (
	controlflow
	${CONTROLFLOW_H_LIST}
	${CONTROLFLOW_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# function folder
#

set (
	FUNCTION_H_LIST
	${JNC_INC_DIR}/jnc_CallFunction.h
	${JNC_INC_DIR}/jnc_Disassembler.h
	${JNC_INC_DIR}/jnc_Function.h
	${JNC_INC_DIR}/jnc_FunctionMgr.h
	${JNC_INC_DIR}/jnc_Property.h
	${JNC_INC_DIR}/jnc_PropertyTemplate.h
	${JNC_INC_DIR}/jnc_PropertyVerifier.h
	${JNC_INC_DIR}/jnc_ScheduleLauncherFunction.h
	${JNC_INC_DIR}/jnc_ThunkFunction.h
	${JNC_INC_DIR}/jnc_ThunkProperty.h
	jnc_GcShadowStack.h
	)

set (
	FUNCTION_CPP_LIST
	jnc_Disassembler.cpp
	jnc_Function.cpp
	jnc_FunctionMgr.cpp
	jnc_Property.cpp
	jnc_PropertyTemplate.cpp
	jnc_PropertyVerifier.cpp
	jnc_ScheduleLauncherFunction.cpp
	jnc_ThunkFunction.cpp
	jnc_ThunkProperty.cpp
	jnc_GcShadowStack.cpp
	)

source_virtual_folder (
	function
	${FUNCTION_H_LIST}
	${FUNCTION_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# module folder
#

set (
	MODULE_H_LIST
	${JNC_INC_DIR}/jnc_AttributeBlock.h
	${JNC_INC_DIR}/jnc_AttributeMgr.h
	${JNC_INC_DIR}/jnc_JitMemoryMgr.h
	${JNC_INC_DIR}/jnc_LlvmDiBuilder.h
	${JNC_INC_DIR}/jnc_LlvmIrBuilder.h
	${JNC_INC_DIR}/jnc_Module.h
	${JNC_INC_DIR}/jnc_ModuleItem.h
	${JNC_INC_DIR}/jnc_UnitMgr.h
	)

set (
	MODULE_CPP_LIST
	jnc_AttributeBlock.cpp
	jnc_AttributeMgr.cpp
	jnc_JitMemoryMgr.cpp
	jnc_LlvmDiBuilder.cpp
	jnc_LlvmIrBuilder.cpp
	jnc_Module.cpp
	jnc_ModuleItem.cpp
	jnc_UnitMgr.cpp
	)

source_virtual_folder (
	module
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# namespace folder
#

set (
	NAMESPACE_H_LIST
	${JNC_INC_DIR}/jnc_Namespace.h
	${JNC_INC_DIR}/jnc_NamespaceMgr.h
	${JNC_INC_DIR}/jnc_Orphan.h
	${JNC_INC_DIR}/jnc_QualifiedName.h
	${JNC_INC_DIR}/jnc_Scope.h
	${JNC_INC_DIR}/jnc_ScopeLevelStack.h
	)

set (
	NAMESPACE_CPP_LIST
	jnc_Namespace.cpp
	jnc_NamespaceMgr.cpp
	jnc_Orphan.cpp
	jnc_QualifiedName.cpp
	jnc_Scope.cpp
	jnc_ScopeLevelStack.cpp
	)

source_virtual_folder (
	namespace
	${NAMESPACE_H_LIST}
	${NAMESPACE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# operator folder
#

set (
	OPERATOR_H_LIST
	${JNC_INC_DIR}/jnc_BinOp.h
	${JNC_INC_DIR}/jnc_BinOp_Arithmetic.h
	${JNC_INC_DIR}/jnc_BinOp_Assign.h
	${JNC_INC_DIR}/jnc_BinOp_At.h
	${JNC_INC_DIR}/jnc_BinOp_Cmp.h
	${JNC_INC_DIR}/jnc_BinOp_Idx.h
	${JNC_INC_DIR}/jnc_BinOp_Logic.h
	${JNC_INC_DIR}/jnc_CastOp.h
	${JNC_INC_DIR}/jnc_CastOp_Array.h
	${JNC_INC_DIR}/jnc_CastOp_Bool.h
	${JNC_INC_DIR}/jnc_CastOp_ClassPtr.h
	${JNC_INC_DIR}/jnc_CastOp_DataPtr.h
	${JNC_INC_DIR}/jnc_CastOp_Fp.h
	${JNC_INC_DIR}/jnc_CastOp_FunctionPtr.h
	${JNC_INC_DIR}/jnc_CastOp_Int.h
	${JNC_INC_DIR}/jnc_CastOp_PropertyPtr.h
	${JNC_INC_DIR}/jnc_CastOp_Struct.h
	${JNC_INC_DIR}/jnc_OperatorMgr.h
	${JNC_INC_DIR}/jnc_UnOp.h
	${JNC_INC_DIR}/jnc_UnOp_Arithmetic.h
	${JNC_INC_DIR}/jnc_UnOp_Inc.h
	${JNC_INC_DIR}/jnc_UnOp_LogNot.h
	${JNC_INC_DIR}/jnc_UnOp_Ptr.h
	)

set (
	OPERATOR_CPP_LIST
	jnc_BinOp.cpp
	jnc_BinOp_Arithmetic.cpp
	jnc_BinOp_Assign.cpp
	jnc_BinOp_At.cpp
	jnc_BinOp_Cmp.cpp
	jnc_BinOp_Idx.cpp
	jnc_BinOp_Logic.cpp
	jnc_CastOp.cpp
	jnc_CastOp_Array.cpp
	jnc_CastOp_Bool.cpp
	jnc_CastOp_ClassPtr.cpp
	jnc_CastOp_DataPtr.cpp
	jnc_CastOp_Fp.cpp
	jnc_CastOp_FunctionPtr.cpp
	jnc_CastOp_Int.cpp
	jnc_CastOp_PropertyPtr.cpp
	jnc_CastOp_Struct.cpp
	jnc_OperatorMgr.cpp
	jnc_OperatorMgr_Call.cpp
	jnc_OperatorMgr_CheckPtr.cpp
	jnc_OperatorMgr_Closure.cpp
	jnc_OperatorMgr_DataRef.cpp
	jnc_OperatorMgr_Field.cpp
	jnc_OperatorMgr_Logic.cpp
	jnc_OperatorMgr_Member.cpp
	jnc_OperatorMgr_New.cpp
	jnc_OperatorMgr_Property.cpp
	jnc_OperatorMgr_Virtual.cpp
	jnc_UnOp.cpp
	jnc_UnOp_Arithmetic.cpp
	jnc_UnOp_Inc.cpp
	jnc_UnOp_LogNot.cpp
	jnc_UnOp_Ptr.cpp
	)

source_virtual_folder (
	operator
	${OPERATOR_H_LIST}
	${OPERATOR_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# parser folder
#

set (
	PARSER_H_LIST
	${JNC_INC_DIR}/jnc_Lexer.h
	jnc_Decl.h
	jnc_DeclTypeCalc.h
	)

set (
	PARSER_CPP_LIST
	jnc_Decl.cpp
	jnc_DeclTypeCalc.cpp
	jnc_Lexer.cpp
	jnc_Parser.cpp
	)

set (
	PARSER_RL_LIST
	jnc_Lexer.rl
	)

set (
	PARSER_LLK_LIST
	jnc_Decl.llk
	jnc_DeclarationSpecifier.llk
	jnc_Declarator.llk
	jnc_Expr.llk
	jnc_Expr_0.llk
	jnc_NamedTypeSpecifier.llk
	jnc_Parser.llk
	jnc_Reactor.llk
	jnc_Stmt.llk
	)

source_virtual_folder (
	parser
	${PARSER_H_LIST}
	${PARSER_CPP_LIST}
	${PARSER_RL_LIST}
	${PARSER_LLK_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# type folder
#

set (
	TYPE_H_LIST
	${JNC_INC_DIR}/jnc_ArrayType.h
	${JNC_INC_DIR}/jnc_BitFieldType.h
	${JNC_INC_DIR}/jnc_ClassPtrType.h
	${JNC_INC_DIR}/jnc_ClassType.h
	${JNC_INC_DIR}/jnc_ClosureClassType.h
	${JNC_INC_DIR}/jnc_DataPtrType.h
	${JNC_INC_DIR}/jnc_DerivableType.h
	${JNC_INC_DIR}/jnc_EnumType.h
	${JNC_INC_DIR}/jnc_FunctionArg.h
	${JNC_INC_DIR}/jnc_FunctionPtrType.h
	${JNC_INC_DIR}/jnc_FunctionType.h
	${JNC_INC_DIR}/jnc_FunctionTypeOverload.h
	${JNC_INC_DIR}/jnc_ImportType.h
	${JNC_INC_DIR}/jnc_McSnapshotClassType.h
	${JNC_INC_DIR}/jnc_MulticastClassType.h
	${JNC_INC_DIR}/jnc_NamedType.h
	${JNC_INC_DIR}/jnc_PropertyPtrType.h
	${JNC_INC_DIR}/jnc_PropertyType.h
	${JNC_INC_DIR}/jnc_ReactorClassType.h
	${JNC_INC_DIR}/jnc_StructType.h
	${JNC_INC_DIR}/jnc_Type.h
	${JNC_INC_DIR}/jnc_TypeMgr.h
	${JNC_INC_DIR}/jnc_UnionType.h
	${JNC_INC_DIR}/jnc_VariantType.h
	)

set (
	TYPE_CPP_LIST
	jnc_ArrayType.cpp
	jnc_BitFieldType.cpp
	jnc_ClassPtrType.cpp
	jnc_ClassType.cpp
	jnc_ClosureClassType.cpp
	jnc_DataPtrType.cpp
	jnc_DerivableType.cpp
	jnc_EnumType.cpp
	jnc_FunctionArg.cpp
	jnc_FunctionPtrType.cpp
	jnc_FunctionType.cpp
	jnc_FunctionTypeOverload.cpp
	jnc_ImportType.cpp
	jnc_McSnapshotClassType.cpp
	jnc_MulticastClassType.cpp
	jnc_NamedType.cpp
	jnc_PropertyPtrType.cpp
	jnc_PropertyType.cpp
	jnc_ReactorClassType.cpp
	jnc_StructType.cpp
	jnc_Type.cpp
	jnc_TypeMgr.cpp
	jnc_UnionType.cpp
	)

source_virtual_folder (
	type
	${TYPE_H_LIST}
	${TYPE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# value folder
#

set (
	VALUE_H_LIST
	${JNC_INC_DIR}/jnc_Closure.h
	${JNC_INC_DIR}/jnc_ConstMgr.h
	${JNC_INC_DIR}/jnc_ObjHdr.h
	${JNC_INC_DIR}/jnc_LeanDataPtrValidator.h
	${JNC_INC_DIR}/jnc_Value.h
	)

set (
	VALUE_CPP_LIST
	jnc_Closure.cpp
	jnc_ConstMgr.cpp
	jnc_ObjHdr.cpp
	jnc_LeanDataPtrValidator.cpp
	jnc_Value.cpp
	)

source_virtual_folder (
	value
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# variable folder
#

set (
	VARIABLE_H_LIST
	${JNC_INC_DIR}/jnc_Alias.h
	${JNC_INC_DIR}/jnc_DestructList.h
	${JNC_INC_DIR}/jnc_Variable.h
	${JNC_INC_DIR}/jnc_VariableMgr.h
	)

set (
	VARIABLE_CPP_LIST
	jnc_Alias.cpp
	jnc_DestructList.cpp
	jnc_Variable.cpp
	jnc_VariableMgr.cpp
	)

source_virtual_folder (
	variable
	${VARIABLE_H_LIST}
	${VARIABLE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# runtime folder
#

set (
	RUNTIME_H_LIST
	${JNC_INC_DIR}/jnc_Api.h
	${JNC_INC_DIR}/jnc_MulticastLib.h
	${JNC_INC_DIR}/jnc_Runtime.h
	${JNC_INC_DIR}/jnc_StdLib.h
	${JNC_INC_DIR}/jnc_TlsMgr.h
	jnc_Multicast.h
	)

set (
	RUNTIME_CPP_LIST
	jnc_Api.cpp
	jnc_MulticastLib.cpp
	jnc_Runtime.cpp
	jnc_StdLib.cpp
	jnc_TlsMgr.cpp
	jnc_Multicast.cpp
	)

source_virtual_folder (
	runtime
	${RUNTIME_H_LIST}
	${RUNTIME_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (JNC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)

push_and_set (CMAKE_CURRENT_BINARY_DIR ${JNC_GEN_DIR})

add_ragel_step (
	jnc_Lexer.rl.cpp
	jnc_Lexer.rl
	)

add_gepard_step (
	jnc_Parser.llk.h
	jnc_Parser.llk
	"${PATH_GEPARD_FRAME_DIR}/llk_Parser.h.frame"

	# dependencies:

	jnc_Decl.llk
	jnc_DeclarationSpecifier.llk
	jnc_Declarator.llk
	jnc_Expr.llk
	jnc_Expr_0.llk
	jnc_NamedTypeSpecifier.llk
	jnc_Reactor.llk
	jnc_Stmt.llk
	)

pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_RL_CPP_LIST
	${JNC_GEN_DIR}/jnc_Lexer.rl.cpp
	)

set (
	GEN_LLK_H_LIST
	${JNC_GEN_DIR}/jnc_Parser.llk.h
	)

exclude_from_build (${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually

source_virtual_folder (
	gen
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)
set (JNC_PCH_H ${JNC_INC_DIR}/jnc_Pch.h)

source_virtual_folder (
	pch
	${PCH_H}
	${PCH_CPP}
	${JNC_PCH_H}
	)

#..............................................................................
#
# jnc library
#

include_directories (
	${PATH_AXL_INC_DIR}
	${PATH_GEPARD_INC_DIR}
	${PATH_LLVM_INC_DIR}
	${PATH_LLVM_INC_DIR_2}
	${JNC_GEN_DIR}
	)

add_library (
	jnc
	${PCH_H}
	${PCH_CPP}
	${JNC_PCH_H}
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	${CONTROLFLOW_H_LIST}
	${CONTROLFLOW_CPP_LIST}
	${FUNCTION_H_LIST}
	${FUNCTION_CPP_LIST}
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
	${NAMESPACE_H_LIST}
	${NAMESPACE_CPP_LIST}
	${OPERATOR_H_LIST}
	${OPERATOR_CPP_LIST}
	${PARSER_H_LIST}
	${PARSER_CPP_LIST}
	${PARSER_RL_LIST}
	${PARSER_LLK_LIST}
	${TYPE_H_LIST}
	${TYPE_CPP_LIST}
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
	${VARIABLE_H_LIST}
	${VARIABLE_CPP_LIST}
	${RUNTIME_H_LIST}
	${RUNTIME_CPP_LIST}
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	)

if (JNC_USE_PCH)
	set_pch (
		jnc
		${PCH_H}
		${PCH_CPP}
		)
endif ()

#..............................................................................

