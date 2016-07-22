# This file is part of Jancy project 
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# callconv folder
#

set (
	CALLCONV_H_LIST
	jnc_ct_CallConv.h
	jnc_ct_CallConv_gcc32.h
	jnc_ct_CallConv_msc32.h
	jnc_ct_CdeclCallConv_gcc64.h
	jnc_ct_CdeclCallConv_msc64.h
	jnc_ct_JnccallCallConv.h
	)

set (
	CALLCONV_CPP_LIST
	jnc_ct_CallConv.cpp
	jnc_ct_CallConv_gcc32.cpp
	jnc_ct_CallConv_msc32.cpp
	jnc_ct_CdeclCallConv_gcc64.cpp
	jnc_ct_CdeclCallConv_msc64.cpp
	jnc_ct_JnccallCallConv.cpp
	)

source_group (
	callconv
	FILES
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	)

#..............................................................................
#
# controlflow folder
#

set (
	CONTROLFLOW_H_LIST
	jnc_ct_BasicBlock.h
	jnc_ct_ControlFlowMgr.h
	)

set (
	CONTROLFLOW_CPP_LIST
	jnc_ct_BasicBlock.cpp
	jnc_ct_ControlFlowMgr.cpp
	jnc_ct_ControlFlowMgr_Stmt.cpp
	jnc_ct_ControlFlowMgr_Eh.cpp
	)

source_group (
	controlflow
	FILES
	${CONTROLFLOW_H_LIST}
	${CONTROLFLOW_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# function folder
#

set (
	FUNCTION_H_LIST
	jnc_ct_Function.h
	jnc_ct_FunctionMgr.h
	jnc_ct_Property.h
	jnc_ct_PropertyTemplate.h
	jnc_ct_PropertyVerifier.h
	jnc_ct_ScheduleLauncherFunction.h
	jnc_ct_ThunkFunction.h
	jnc_ct_ThunkProperty.h
	jnc_ct_StdFunction.h
	)

set (
	FUNCTION_CPP_LIST
	jnc_ct_Function.cpp
	jnc_ct_FunctionMgr.cpp
	jnc_ct_Property.cpp
	jnc_ct_PropertyTemplate.cpp
	jnc_ct_PropertyVerifier.cpp
	jnc_ct_ScheduleLauncherFunction.cpp
	jnc_ct_ThunkFunction.cpp
	jnc_ct_ThunkProperty.cpp
	jnc_ct_StdFunction.cpp
	)

source_group (
	function
	FILES
	${FUNCTION_H_LIST}
	${FUNCTION_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# module folder
#

set (
	MODULE_H_LIST
	jnc_ct_AttributeBlock.h
	jnc_ct_AttributeMgr.h
	jnc_ct_LlvmDiBuilder.h
	jnc_ct_LlvmIrBuilder.h
	jnc_ct_LlvmIrInsertPoint.h
	jnc_ct_Module.h
	jnc_ct_ModuleItem.h
	jnc_ct_UnitMgr.h
	jnc_ct_ImportMgr.h
	jnc_ct_GcShadowStackMgr.h
	jnc_ct_JitMemoryMgr.h
	jnc_ct_ExtensionLibMgr.h
	)

set (
	MODULE_CPP_LIST
	jnc_ct_AttributeBlock.cpp
	jnc_ct_AttributeMgr.cpp
	jnc_ct_JitMemoryMgr.cpp
	jnc_ct_LlvmDiBuilder.cpp
	jnc_ct_LlvmIrBuilder.cpp
	jnc_ct_Module.cpp
	jnc_ct_ModuleItem.cpp
	jnc_ct_UnitMgr.cpp
	jnc_ct_ImportMgr.cpp
	jnc_ct_GcShadowStackMgr.cpp
	jnc_ct_ExtensionLibMgr.cpp
	)

source_group (
	module
	FILES
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# namespace folder
#

set (
	NAMESPACE_H_LIST
	jnc_ct_Namespace.h
	jnc_ct_NamespaceMgr.h
	jnc_ct_Orphan.h
	jnc_ct_QualifiedName.h
	jnc_ct_Scope.h
	jnc_ct_UsingSet.h
	jnc_ct_StdNamespace.h
	)

set (
	NAMESPACE_CPP_LIST
	jnc_ct_Namespace.cpp
	jnc_ct_NamespaceMgr.cpp
	jnc_ct_Orphan.cpp
	jnc_ct_QualifiedName.cpp
	jnc_ct_Scope.cpp
	jnc_ct_UsingSet.cpp
	)

source_group (
	namespace
	FILES
	${NAMESPACE_H_LIST}
	${NAMESPACE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# operator folder
#

set (
	OPERATOR_H_LIST
	jnc_ct_BinOp.h
	jnc_ct_BinOp_Arithmetic.h
	jnc_ct_BinOp_Assign.h
	jnc_ct_BinOp_At.h
	jnc_ct_BinOp_Cmp.h
	jnc_ct_BinOp_Idx.h
	jnc_ct_BinOp_Logic.h
	jnc_ct_CastOp.h
	jnc_ct_CastOp_Array.h
	jnc_ct_CastOp_Bool.h
	jnc_ct_CastOp_ClassPtr.h
	jnc_ct_CastOp_DataPtr.h
	jnc_ct_CastOp_Fp.h
	jnc_ct_CastOp_FunctionPtr.h
	jnc_ct_CastOp_Int.h
	jnc_ct_CastOp_PropertyPtr.h
	jnc_ct_CastOp_Struct.h
	jnc_ct_CastOp_Variant.h
	jnc_ct_OperatorMgr.h
	jnc_ct_UnOp.h
	jnc_ct_UnOp_Arithmetic.h
	jnc_ct_UnOp_Inc.h
	jnc_ct_UnOp_LogNot.h
	jnc_ct_UnOp_Ptr.h
	)

set (
	OPERATOR_CPP_LIST
	jnc_ct_BinOp.cpp
	jnc_ct_BinOp_Arithmetic.cpp
	jnc_ct_BinOp_Assign.cpp
	jnc_ct_BinOp_At.cpp
	jnc_ct_BinOp_Cmp.cpp
	jnc_ct_BinOp_Idx.cpp
	jnc_ct_BinOp_Logic.cpp
	jnc_ct_CastOp.cpp
	jnc_ct_CastOp_Array.cpp
	jnc_ct_CastOp_Bool.cpp
	jnc_ct_CastOp_ClassPtr.cpp
	jnc_ct_CastOp_DataPtr.cpp
	jnc_ct_CastOp_Fp.cpp
	jnc_ct_CastOp_FunctionPtr.cpp
	jnc_ct_CastOp_Int.cpp
	jnc_ct_CastOp_PropertyPtr.cpp
	jnc_ct_CastOp_Struct.cpp
	jnc_ct_CastOp_Variant.cpp
	jnc_ct_OperatorMgr.cpp
	jnc_ct_OperatorMgr_Call.cpp
	jnc_ct_OperatorMgr_CheckPtr.cpp
	jnc_ct_OperatorMgr_Closure.cpp
	jnc_ct_OperatorMgr_DataRef.cpp
	jnc_ct_OperatorMgr_Field.cpp
	jnc_ct_OperatorMgr_Logic.cpp
	jnc_ct_OperatorMgr_Member.cpp
	jnc_ct_OperatorMgr_New.cpp
	jnc_ct_OperatorMgr_Property.cpp
	jnc_ct_OperatorMgr_Virtual.cpp
	jnc_ct_UnOp.cpp
	jnc_ct_UnOp_Arithmetic.cpp
	jnc_ct_UnOp_Inc.cpp
	jnc_ct_UnOp_LogNot.cpp
	jnc_ct_UnOp_Ptr.cpp
	)

source_group (
	operator
	FILES
	${OPERATOR_H_LIST}
	${OPERATOR_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# parser folder
#

set (
	PARSER_H_LIST
	jnc_ct_Lexer.h
	jnc_ct_Decl.h
	jnc_ct_DeclTypeCalc.h
	)

set (
	PARSER_CPP_LIST
	jnc_ct_Decl.cpp
	jnc_ct_DeclTypeCalc.cpp
	jnc_ct_Lexer.cpp
	jnc_ct_Parser.cpp
	)

set (
	PARSER_RL_LIST
	jnc_ct_Lexer.rl
	)

set (
	PARSER_LLK_LIST
	jnc_ct_Decl.llk
	jnc_ct_DeclarationSpecifier.llk
	jnc_ct_Declarator.llk
	jnc_ct_Expr.llk
	jnc_ct_Expr_0.llk
	jnc_ct_NamedTypeSpecifier.llk
	jnc_ct_Parser.llk
	jnc_ct_Reactor.llk
	jnc_ct_Stmt.llk
	)

source_group (
	parser
	FILES
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
	jnc_ct_ArrayType.h
	jnc_ct_BitFieldType.h
	jnc_ct_ClassPtrType.h
	jnc_ct_ClassType.h
	jnc_ct_ClosureClassType.h
	jnc_ct_DataPtrType.h
	jnc_ct_DerivableType.h
	jnc_ct_EnumType.h
	jnc_ct_FunctionArg.h
	jnc_ct_FunctionPtrType.h
	jnc_ct_FunctionType.h
	jnc_ct_FunctionTypeOverload.h
	jnc_ct_ImportType.h
	jnc_ct_McSnapshotClassType.h
	jnc_ct_MulticastClassType.h
	jnc_ct_NamedTypeBlock.h
	jnc_ct_PropertyPtrType.h
	jnc_ct_PropertyType.h
	jnc_ct_ReactorClassType.h
	jnc_ct_StructType.h
	jnc_ct_Type.h
	jnc_ct_TypeMgr.h
	jnc_ct_UnionType.h
	jnc_ct_StdType.h
	)

set (
	TYPE_CPP_LIST
	jnc_ct_ArrayType.cpp
	jnc_ct_BitFieldType.cpp
	jnc_ct_ClassPtrType.cpp
	jnc_ct_ClassType.cpp
	jnc_ct_ClosureClassType.cpp
	jnc_ct_DataPtrType.cpp
	jnc_ct_DerivableType.cpp
	jnc_ct_EnumType.cpp
	jnc_ct_FunctionArg.cpp
	jnc_ct_FunctionPtrType.cpp
	jnc_ct_FunctionType.cpp
	jnc_ct_FunctionTypeOverload.cpp
	jnc_ct_ImportType.cpp
	jnc_ct_McSnapshotClassType.cpp
	jnc_ct_MulticastClassType.cpp
	jnc_ct_NamedTypeBlock.cpp
	jnc_ct_PropertyPtrType.cpp
	jnc_ct_PropertyType.cpp
	jnc_ct_ReactorClassType.cpp
	jnc_ct_StructType.cpp
	jnc_ct_Type.cpp
	jnc_ct_TypeMgr.cpp
	jnc_ct_UnionType.cpp
	jnc_ct_StdType.cpp
	)

source_group (
	type
	FILES
	${TYPE_H_LIST}
	${TYPE_CPP_LIST}
	${TYPE_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# value folder
#

set (
	VALUE_H_LIST
	jnc_ct_Closure.h
	jnc_ct_ConstMgr.h
	jnc_ct_LeanDataPtrValidator.h
	jnc_ct_Value.h
	)

set (
	VALUE_CPP_LIST
	jnc_ct_Closure.cpp
	jnc_ct_ConstMgr.cpp
	jnc_ct_LeanDataPtrValidator.cpp
	jnc_ct_Value.cpp
	)

source_group (
	value
	FILES
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# variable folder
#

set (
	VARIABLE_H_LIST
	jnc_ct_Alias.h
	jnc_ct_Variable.h
	jnc_ct_VariableMgr.h
	)

set (
	VARIABLE_CPP_LIST
	jnc_ct_Alias.cpp
	jnc_ct_Variable.cpp
	jnc_ct_VariableMgr.cpp
	)

source_group (
	variable
	FILES
	${VARIABLE_H_LIST}
	${VARIABLE_CPP_LIST}
	)


# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set (
	JNC_LIST
	jnc/jnc_StdTypes.jnc
	jnc/jnc_StdFunctions.jnc
	jnc/jnc_DynamicLib.jnc
	jnc/jnc_Recognizer.jnc
	jnc/jnc_Scheduler.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# perl folder
#

set (
	PERL_PL_LIST
	${JNC2CPP_PL}
	)

source_group (
	perl
	FILES
	${PERL_PL_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	jnc_StdTypes.jnc.cpp
	jnc/jnc_StdTypes.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_StdFunctions.jnc.cpp
	jnc/jnc_StdFunctions.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_DynamicLib.jnc.cpp
	jnc/jnc_DynamicLib.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_Recognizer.jnc.cpp
	jnc/jnc_Recognizer.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_Scheduler.jnc.cpp
	jnc/jnc_Scheduler.jnc
	${JNC2CPP_PL}
	)

add_ragel_step (
	jnc_ct_Lexer.rl.cpp
	jnc_ct_Lexer.rl
	)

add_graco_double_step (
	jnc_ct_Parser.llk.h
	jnc_ct_Parser.llk.cpp
	CppParser.h.in
	CppParser.cpp.in
	jnc_ct_Parser.llk

	jnc_ct_Decl.llk
	jnc_ct_DeclarationSpecifier.llk
	jnc_ct_Declarator.llk
	jnc_ct_Expr.llk
	jnc_ct_Expr_0.llk
	jnc_ct_NamedTypeSpecifier.llk
	jnc_ct_Reactor.llk
	jnc_ct_Stmt.llk
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_StdTypes.jnc.cpp
	${GEN_DIR}/jnc_StdFunctions.jnc.cpp
	${GEN_DIR}/jnc_DynamicLib.jnc.cpp
	${GEN_DIR}/jnc_Recognizer.jnc.cpp
	${GEN_DIR}/jnc_Scheduler.jnc.cpp
	)

set (
	GEN_RL_CPP_LIST
	${GEN_DIR}/jnc_ct_Lexer.rl.cpp
	)
	
set (
	GEN_LLK_H_LIST
	${GEN_DIR}/jnc_ct_Parser.llk.h
	)

set (
	GEN_LLK_CPP_LIST
	${GEN_DIR}/jnc_ct_Parser.llk.cpp
	)

axl_exclude_from_build (${GEN_JNC_CPP_LIST}) # include "*.jnc.cpp" manually
axl_exclude_from_build (${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually
axl_exclude_from_build (${GEN_LLK_CPP_LIST}) # include "*.llk.cpp" manually

source_group (
	gen
	FILES
	${GEN_JNC_CPP_LIST}
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	${GEN_LLK_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

set (
	JNC_PCH_H_LIST
	jnc_ct_Pch.h
	)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	${JNC_PCH_H_LIST}
	)

#..............................................................................
#
# jnc_ct library
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${GRACO_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_rt
	${JANCY_SRC_DIR}/jnc_rtl
	${GEN_DIR}
	${CMAKE_CURRENT_LIST_DIR}
	)

add_library (
	jnc_ct
	${PCH_H}
	${PCH_CPP}
	${JNC_PCH_H_LIST}
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
	${JNC_LIST}
	${PERL_PL_LIST}
	${GEN_JNC_CPP_LIST}
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	# ${GEN_LLK_CPP_LIST} # currently, cmake can't handle double-file output
	)

axl_set_pch (
	jnc_ct
	${PCH_H}
	${PCH_CPP}
	)

if (GCC)
	axl_append_target_string_property (
		jnc_ct
		COMPILE_FLAGS
		"-fPIC"
		)
endif ()

#..............................................................................
