#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

#
# include folder
#

file (
	GLOB
	INC_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	include/*.h
	)

source_group (
	include
	FILES
	${INC_H_LIST}
	)


#...............................................................................
#
# jnc_ct_CallConv folder
#

file (
	GLOB
	CALLCONV_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_CallConv/*.h
	)

file (
	GLOB
	CALLCONV_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_CallConv/*.cpp
	)

source_group (
	jnc_ct_CallConv
	FILES
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	)		

#...............................................................................
#
# jnc_ct_ControlFlowMgr folder
#

file (
	GLOB
	CONTROLFLOWMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_ControlFlowMgr/*.h
	)

file (
	GLOB
	CONTROLFLOWMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_ControlFlowMgr/*.cpp
	)

source_group (
	jnc_ct_ControlFlowMgr
	FILES
	${CONTROLFLOWMGR_H_LIST}
	${CONTROLFLOWMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_DoxyMgr folder
#

file (
	GLOB
	DOXYMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_DoxyMgr/*.h
	)

file (
	GLOB
	DOXYMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_DoxyMgr/*.cpp
	)

file (
	GLOB
	DOXYMGR_RL_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_DoxyMgr/*.rl
	)

source_group (
	jnc_ct_DoxyMgr
	FILES
	${DOXYMGR_H_LIST}
	${DOXYMGR_CPP_LIST}
	${DOXYMGR_RL_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_FunctionMgr folder
#

file (
	GLOB
	FUNCTIONMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_FunctionMgr/*.h
	)

file (
	GLOB
	FUNCTIONMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_FunctionMgr/*.cpp
	)

source_group (
	jnc_ct_FunctionMgr
	FILES
	${FUNCTIONMGR_H_LIST}
	${FUNCTIONMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_NamespaceMgr folder
#

file (
	GLOB
	NAMESPACEMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_NamespaceMgr/*.h
	)

file (
	GLOB
	NAMESPACEMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_NamespaceMgr/*.cpp
	)

source_group (
	jnc_ct_NamespaceMgr
	FILES
	${NAMESPACEMGR_H_LIST}
	${NAMESPACEMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_OperatorMgr folder
#

file (
	GLOB
	OPERATORMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_OperatorMgr/*.h
	)

file (
	GLOB
	OPERATORMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_OperatorMgr/*.cpp
	)

source_group (
	jnc_ct_OperatorMgr
	FILES
	${OPERATORMGR_H_LIST}
	${OPERATORMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_TypeMgr folder
#

file (
	GLOB
	TYPEMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_TypeMgr/*.h
	)

file (
	GLOB
	TYPEMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_TypeMgr/*.cpp
	)

source_group (
	jnc_ct_TypeMgr
	FILES
	${TYPEMGR_H_LIST}
	${TYPEMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_VariableMgr folder
#

file (
	GLOB
	VARIABLEMGR_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_VariableMgr/*.h
	)

file (
	GLOB
	VARIABLEMGR_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_VariableMgr/*.cpp
	)

source_group (
	jnc_ct_VariableMgr
	FILES
	${VARIABLEMGR_H_LIST}
	${VARIABLEMGR_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Value folder
#

file (
	GLOB
	VALUE_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Value/*.h
	)

file (
	GLOB
	VALUE_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Value/*.cpp
	)

source_group (
	jnc_ct_Value
	FILES
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Module folder
#

file (
	GLOB
	MODULE_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Module/*.h
	)

file (
	GLOB
	MODULE_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Module/*.cpp
	)

source_group (
	jnc_ct_Module
	FILES
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Parser folder
#

file (
	GLOB
	PARSER_H_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Parser/*.h
	)

file (
	GLOB
	PARSER_CPP_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Parser/*.cpp
	)

file (
	GLOB
	PARSER_RL_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Parser/*.rl
	)

file (
	GLOB
	PARSER_LLK_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc_ct_Parser/*.llk
	)

source_group (
	jnc_ct_Parser
	FILES
	${PARSER_H_LIST}
	${PARSER_CPP_LIST}
	${PARSER_RL_LIST}
	${PARSER_LLK_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

file (
	GLOB
	JNC_LIST
	RELATIVE ${CMAKE_CURRENT_LIST_DIR}
	jnc/*.jnc
	)

source_group (
	jnc
	FILES
	${JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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
	jnc_Regex.jnc.cpp
	jnc/jnc_Regex.jnc
	${JNC2CPP_PL}
	)

add_perl_step (
	jnc_Scheduler.jnc.cpp
	jnc/jnc_Scheduler.jnc
	${JNC2CPP_PL}
	)

add_ragel_step (
	jnc_ct_DoxyLexer.rl.cpp
	jnc_ct_DoxyMgr/jnc_ct_DoxyLexer.rl
	)

add_ragel_step (
	jnc_ct_Lexer.rl.cpp
	jnc_ct_Parser/jnc_ct_Lexer.rl
	)

add_graco_double_step (
	jnc_ct_Parser.llk.h
	jnc_ct_Parser.llk.cpp
	CppParser.h.in
	CppParser.cpp.in
	jnc_ct_Parser/jnc_ct_Parser.llk

	jnc_ct_Parser/jnc_ct_Decl.llk
	jnc_ct_Parser/jnc_ct_DeclarationSpecifier.llk
	jnc_ct_Parser/jnc_ct_Declarator.llk
	jnc_ct_Parser/jnc_ct_Expr.llk
	jnc_ct_Parser/jnc_ct_Expr_0.llk
	jnc_ct_Parser/jnc_ct_NamedTypeSpecifier.llk
	jnc_ct_Parser/jnc_ct_Reactor.llk
	jnc_ct_Parser/jnc_ct_Stmt.llk
	)

axl_pop (CMAKE_CURRENT_BINARY_DIR)

set (
	GEN_JNC_CPP_LIST
	${GEN_DIR}/jnc_StdTypes.jnc.cpp
	${GEN_DIR}/jnc_StdFunctions.jnc.cpp
	${GEN_DIR}/jnc_DynamicLib.jnc.cpp
	${GEN_DIR}/jnc_Scheduler.jnc.cpp
	${GEN_DIR}/jnc_Regex.jnc.cpp
	)

set (
	GEN_RL_CPP_LIST
	${GEN_DIR}/jnc_ct_Lexer.rl.cpp
	${GEN_DIR}/jnc_ct_DoxyLexer.rl.cpp
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

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set (PCH_H     pch.h)
set (PCH_CPP   pch.cpp)

source_group (
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_ct library
#

include_directories (
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${GRACO_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_rt
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_ext/jnc_rtl
	${GEN_DIR}
	)

add_library (
	jnc_ct
	${PCH_H}
	${PCH_CPP}
	${INC_H_LIST}
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	${CONTROLFLOWMGR_H_LIST}
	${CONTROLFLOWMGR_CPP_LIST}
	${DOXYMGR_H_LIST}
	${DOXYMGR_CPP_LIST}
	${DOXYMGR_RL_LIST}
	${FUNCTIONMGR_H_LIST}
	${FUNCTIONMGR_CPP_LIST}
	${NAMESPACEMGR_H_LIST}
	${NAMESPACEMGR_CPP_LIST}
	${OPERATORMGR_H_LIST}
	${OPERATORMGR_CPP_LIST}
	${TYPEMGR_H_LIST}
	${TYPEMGR_CPP_LIST}
	${VARIABLEMGR_H_LIST}
	${VARIABLEMGR_CPP_LIST}
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
	${PARSER_H_LIST}
	${PARSER_CPP_LIST}
	${PARSER_RL_LIST}
	${PARSER_LLK_LIST}
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

#...............................................................................
