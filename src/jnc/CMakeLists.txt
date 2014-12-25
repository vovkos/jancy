# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# callconv folder
#

set (
	CALLCONV_H_LIST
	${JANCY_INC_DIR}/jnc_CallConv.h
	${JANCY_INC_DIR}/jnc_CallConv_gcc32.h
	${JANCY_INC_DIR}/jnc_CallConv_msc32.h
	${JANCY_INC_DIR}/jnc_CdeclCallConv_gcc64.h
	${JANCY_INC_DIR}/jnc_CdeclCallConv_msc64.h
	${JANCY_INC_DIR}/jnc_JnccallCallConv.h
	)

set (
	CALLCONV_CPP_LIST
	jnc_CallConv.cpp
	jnc_CallConv_gcc32.cpp
	jnc_CallConv_msc32.cpp
	jnc_CdeclCallConv_gcc64.cpp
	jnc_CdeclCallConv_msc64.cpp
	jnc_JnccallCallConv.cpp
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
	${JANCY_INC_DIR}/jnc_BasicBlock.h
	${JANCY_INC_DIR}/jnc_ControlFlowMgr.h
	)

set (
	CONTROLFLOW_CPP_LIST
	jnc_BasicBlock.cpp
	jnc_ControlFlowMgr.cpp
	jnc_ControlFlowMgr_Stmt.cpp
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
	${JANCY_INC_DIR}/jnc_CallFunction.h
	${JANCY_INC_DIR}/jnc_Disassembler.h
	${JANCY_INC_DIR}/jnc_Function.h
	${JANCY_INC_DIR}/jnc_FunctionMgr.h
	${JANCY_INC_DIR}/jnc_Property.h
	${JANCY_INC_DIR}/jnc_PropertyTemplate.h
	${JANCY_INC_DIR}/jnc_PropertyVerifier.h
	${JANCY_INC_DIR}/jnc_ScheduleLauncherFunction.h
	${JANCY_INC_DIR}/jnc_ThunkFunction.h
	${JANCY_INC_DIR}/jnc_ThunkProperty.h
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
	${JANCY_INC_DIR}/jnc_AttributeBlock.h
	${JANCY_INC_DIR}/jnc_AttributeMgr.h
	${JANCY_INC_DIR}/jnc_JitMemoryMgr.h
	${JANCY_INC_DIR}/jnc_LlvmDiBuilder.h
	${JANCY_INC_DIR}/jnc_LlvmIrBuilder.h
	${JANCY_INC_DIR}/jnc_Module.h
	${JANCY_INC_DIR}/jnc_ModuleItem.h
	${JANCY_INC_DIR}/jnc_UnitMgr.h
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
	${JANCY_INC_DIR}/jnc_Namespace.h
	${JANCY_INC_DIR}/jnc_NamespaceMgr.h
	${JANCY_INC_DIR}/jnc_Orphan.h
	${JANCY_INC_DIR}/jnc_QualifiedName.h
	${JANCY_INC_DIR}/jnc_Scope.h
	${JANCY_INC_DIR}/jnc_ScopeLevelStack.h
	${JANCY_INC_DIR}/jnc_UsingSet.h
	)

set (
	NAMESPACE_CPP_LIST
	jnc_Namespace.cpp
	jnc_NamespaceMgr.cpp
	jnc_Orphan.cpp
	jnc_QualifiedName.cpp
	jnc_Scope.cpp
	jnc_ScopeLevelStack.cpp
	jnc_UsingSet.cpp
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
	${JANCY_INC_DIR}/jnc_BinOp.h
	${JANCY_INC_DIR}/jnc_BinOp_Arithmetic.h
	${JANCY_INC_DIR}/jnc_BinOp_Assign.h
	${JANCY_INC_DIR}/jnc_BinOp_At.h
	${JANCY_INC_DIR}/jnc_BinOp_Cmp.h
	${JANCY_INC_DIR}/jnc_BinOp_Idx.h
	${JANCY_INC_DIR}/jnc_BinOp_Logic.h
	${JANCY_INC_DIR}/jnc_CastOp.h
	${JANCY_INC_DIR}/jnc_CastOp_Array.h
	${JANCY_INC_DIR}/jnc_CastOp_Bool.h
	${JANCY_INC_DIR}/jnc_CastOp_ClassPtr.h
	${JANCY_INC_DIR}/jnc_CastOp_DataPtr.h
	${JANCY_INC_DIR}/jnc_CastOp_Fp.h
	${JANCY_INC_DIR}/jnc_CastOp_FunctionPtr.h
	${JANCY_INC_DIR}/jnc_CastOp_Int.h
	${JANCY_INC_DIR}/jnc_CastOp_PropertyPtr.h
	${JANCY_INC_DIR}/jnc_CastOp_Struct.h
	${JANCY_INC_DIR}/jnc_OperatorMgr.h
	${JANCY_INC_DIR}/jnc_UnOp.h
	${JANCY_INC_DIR}/jnc_UnOp_Arithmetic.h
	${JANCY_INC_DIR}/jnc_UnOp_Inc.h
	${JANCY_INC_DIR}/jnc_UnOp_LogNot.h
	${JANCY_INC_DIR}/jnc_UnOp_Ptr.h
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
	${JANCY_INC_DIR}/jnc_Lexer.h
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
	${JANCY_INC_DIR}/jnc_ArrayType.h
	${JANCY_INC_DIR}/jnc_BitFieldType.h
	${JANCY_INC_DIR}/jnc_ClassPtrType.h
	${JANCY_INC_DIR}/jnc_ClassType.h
	${JANCY_INC_DIR}/jnc_ClosureClassType.h
	${JANCY_INC_DIR}/jnc_DataPtrType.h
	${JANCY_INC_DIR}/jnc_DerivableType.h
	${JANCY_INC_DIR}/jnc_EnumType.h
	${JANCY_INC_DIR}/jnc_FunctionArg.h
	${JANCY_INC_DIR}/jnc_FunctionPtrType.h
	${JANCY_INC_DIR}/jnc_FunctionType.h
	${JANCY_INC_DIR}/jnc_FunctionTypeOverload.h
	${JANCY_INC_DIR}/jnc_ImportType.h
	${JANCY_INC_DIR}/jnc_McSnapshotClassType.h
	${JANCY_INC_DIR}/jnc_MulticastClassType.h
	${JANCY_INC_DIR}/jnc_NamedTypeBlock.h
	${JANCY_INC_DIR}/jnc_PropertyPtrType.h
	${JANCY_INC_DIR}/jnc_PropertyType.h
	${JANCY_INC_DIR}/jnc_ReactorClassType.h
	${JANCY_INC_DIR}/jnc_StructType.h
	${JANCY_INC_DIR}/jnc_Type.h
	${JANCY_INC_DIR}/jnc_TypeMgr.h
	${JANCY_INC_DIR}/jnc_UnionType.h
	${JANCY_INC_DIR}/jnc_VariantType.h
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
	jnc_NamedTypeBlock.cpp
	jnc_PropertyPtrType.cpp
	jnc_PropertyType.cpp
	jnc_ReactorClassType.cpp
	jnc_StructType.cpp
	jnc_Type.cpp
	jnc_TypeMgr.cpp
	jnc_UnionType.cpp
	)

source_group (
	type
	FILES
	${TYPE_H_LIST}
	${TYPE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# value folder
#

set (
	VALUE_H_LIST
	${JANCY_INC_DIR}/jnc_Closure.h
	${JANCY_INC_DIR}/jnc_ConstMgr.h
	${JANCY_INC_DIR}/jnc_ObjHdr.h
	${JANCY_INC_DIR}/jnc_LeanDataPtrValidator.h
	${JANCY_INC_DIR}/jnc_Value.h
	)

set (
	VALUE_CPP_LIST
	jnc_Closure.cpp
	jnc_ConstMgr.cpp
	jnc_ObjHdr.cpp
	jnc_LeanDataPtrValidator.cpp
	jnc_Value.cpp
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
	${JANCY_INC_DIR}/jnc_Alias.h
	${JANCY_INC_DIR}/jnc_DestructList.h
	${JANCY_INC_DIR}/jnc_Variable.h
	${JANCY_INC_DIR}/jnc_VariableMgr.h
	)

set (
	VARIABLE_CPP_LIST
	jnc_Alias.cpp
	jnc_DestructList.cpp
	jnc_Variable.cpp
	jnc_VariableMgr.cpp
	)

source_group (
	variable
	FILES
	${VARIABLE_H_LIST}
	${VARIABLE_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# runtime folder
#

set (
	RUNTIME_H_LIST
	${JANCY_INC_DIR}/jnc_Runtime.h
	${JANCY_INC_DIR}/jnc_TlsMgr.h
	jnc_Multicast.h
	)

set (
	RUNTIME_CPP_LIST
	jnc_Runtime.cpp
	jnc_TlsMgr.cpp
	)

source_group (
	runtime
	FILES
	${RUNTIME_H_LIST}
	${RUNTIME_CPP_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# stdlib folder
#

set (
	STDLIB_H_LIST
	${JANCY_INC_DIR}/jnc_Api.h
	${JANCY_INC_DIR}/jnc_MulticastLib.h
	${JANCY_INC_DIR}/jnc_Error.h
	${JANCY_INC_DIR}/jnc_Buffer.h
	${JANCY_INC_DIR}/jnc_String.h
	${JANCY_INC_DIR}/jnc_StdLibApiSlots.h
	${JANCY_INC_DIR}/jnc_StdLib.h
	jnc_Multicast.h
	)

set (
	STDLIB_CPP_LIST
	jnc_Api.cpp
	jnc_Multicast.cpp
	jnc_MulticastLib.cpp
	jnc_Error.cpp
	jnc_Buffer.cpp
	jnc_String.cpp
	jnc_StdLib.cpp
	)

set (
	STDLIB_JNC_LIST
	jnc_StdTypes.jnc
	jnc_StdFunctions.jnc
	jnc_Buffer.jnc
	jnc_String.jnc
	)

set (
	STDLIB_PL_LIST
	jnc2cpp.pl
	)
	
source_group (
	stdlib
	FILES
	${STDLIB_H_LIST}
	${STDLIB_CPP_LIST}
	${STDLIB_JNC_LIST}
	${STDLIB_PL_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set (GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file (MAKE_DIRECTORY ${GEN_DIR})

push_and_set (CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

add_perl_step (
	jnc_StdTypes.jnc.cpp
	jnc_StdTypes.jnc
	jnc2cpp.pl
	)

add_perl_step (
	jnc_StdFunctions.jnc.cpp
	jnc_StdFunctions.jnc
	jnc2cpp.pl
	)

add_perl_step (
	jnc_Buffer.jnc.cpp
	jnc_Buffer.jnc
	jnc2cpp.pl
	)

add_perl_step (
	jnc_String.jnc.cpp
	jnc_String.jnc
	jnc2cpp.pl
	)
	
add_ragel_step (
	jnc_Lexer.rl.cpp
	jnc_Lexer.rl
	)

add_bulldozer_double_step (
	jnc_Parser.llk.h
	jnc_Parser.llk.cpp
	CppParser.h.in
	CppParser.cpp.in
	jnc_Parser.llk

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
	GEN_PL_CPP_LIST
	${GEN_DIR}/jnc_StdTypes.jnc.cpp
	${GEN_DIR}/jnc_StdFunctions.jnc.cpp
	${GEN_DIR}/jnc_Buffer.jnc.cpp
	${GEN_DIR}/jnc_String.jnc.cpp
	)

set (
	GEN_RL_CPP_LIST
	${GEN_DIR}/jnc_Lexer.rl.cpp
	)
	
set (
	GEN_LLK_H_LIST
	${GEN_DIR}/jnc_Parser.llk.h
	)

set (
	GEN_LLK_CPP_LIST
	${GEN_DIR}/jnc_Parser.llk.cpp
	)

exclude_from_build (${GEN_PL_CPP_LIST})  # include "*.jnc.cpp" manually
exclude_from_build (${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually
exclude_from_build (${GEN_LLK_CPP_LIST}) # include "*.llk.cpp" manually

source_group (
	gen
	FILES
	${GEN_PL_CPP_LIST}
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
set (JNC_PCH_H ${JANCY_INC_DIR}/jnc_Pch.h)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	${JNC_PCH_H}
	)

#..............................................................................
#
# jnc library
#

include_directories (
	${LLVM_INC_DIR}
	${LLVM_INC_DIR_2}
	${AXL_INC_DIR}
	${BULLDOZER_INC_DIR}
	${JANCY_INC_DIR}
	${GEN_DIR}
	${CMAKE_CURRENT_LIST_DIR}
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
	${STDLIB_H_LIST}
	${STDLIB_CPP_LIST}
	${STDLIB_JNC_LIST}
	${STDLIB_PL_LIST}
	${GEN_PL_CPP_LIST}
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	# ${GEN_LLK_CPP_LIST} # currently, cmake can't handle double-file output
	)

set_pch (
	jnc
	${PCH_H}
	${PCH_CPP}
	)

#..............................................................................
