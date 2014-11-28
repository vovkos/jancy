# This file is part of Jancy project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# base samples folder
#

set (
	BASE_JNC_LIST
	00_HelloWorld.jnc
	01_Classes.jnc
	02_Inheritance.jnc
	03_Storage.jnc
	04_RAII.jnc
	05_WeakClassPtr.jnc	
	)

source_group (
	0_base
	FILES
	${BASE_JNC_LIST}
	)
	
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# data ptr samples folder
#

set (
	DATA_PTR_JNC_LIST
	10_DataPtrRange.jnc
	11_DataPtrScope.jnc
	12_ConstCorrectness.jnc
	13_DynamicCast.jnc
	14_DynamicSizeof.jnc
	)

source_group (
	1_data_ptr
	FILES
	${DATA_PTR_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# function ptr samples folder
#

set (
	FUNCTION_PTR_JNC_LIST
	20_FunctionPtr.jnc
	21_WeakFunctionPtr.jnc
	22_ScheduleOperator.jnc
	23_Multicasts.jnc
	24_WeakMulticasts.jnc	
	)

source_group (
	2_function_ptr
	FILES
	${FUNCTION_PTR_JNC_LIST}
	)
	
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# property samples folder
#

set (
	PROPERTY_JNC_LIST
	30_SimplePropertyDecl.jnc
	31_FullPropertyDecl.jnc
	32_IndexedProperties.jnc
	33_AutoGetProperties.jnc
	34_BindableProperties.jnc
	35_PropertyPtr.jnc
	36_WeakPropertyPtr.jnc
	)

source_group (
	3_property
	FILES
	${PROPERTY_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# reactive samples folder
#

set (
	REACTIVE_JNC_LIST
	40_Reactors.jnc
	41_OnEventStmt.jnc
	)

source_group (
	4_reactive
	FILES
	${REACTIVE_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# exception samples folder
#

set (
	EXCEPTION_JNC_LIST
	50_Exceptions.jnc
	51_Errors.jnc
	52_Finally.jnc
	)

source_group (
	5_exception
	FILES
	${EXCEPTION_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# literal samples folder
#

set (
	LITERAL_JNC_LIST
	60_HexLiterals.jnc
	61_FormattingLiterals.jnc
	)

source_group (
	6_literal
	FILES
	${LITERAL_JNC_LIST}
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# misc samples folder
#

set (
	MISC_JNC_LIST
	70_DualModifiers.jnc
	71_BitflagEnums.jnc
	72_Once.jnc
	73_BreakN.jnc
	74_CurlyInitializers.jnc
	75_ScopesInSwitch.jnc
	76_ModuleConstructors.jnc
	77_ExtensionNamespaces.jnc
	)

source_group (
	7_misc
	FILES
	${MISC_JNC_LIST}
	)

#..............................................................................
#
# jnc_samples
#

set (
	FULL_JNC_LIST
	${BASE_JNC_LIST}
	${DATA_PTR_JNC_LIST}
	${FUNCTION_PTR_JNC_LIST}
	${PROPERTY_JNC_LIST}
	${REACTIVE_JNC_LIST}
	${EXCEPTION_JNC_LIST}
	${LITERAL_JNC_LIST}
	${MISC_JNC_LIST}
	)

add_custom_target (
	jnc_samples
	SOURCES
	${FULL_JNC_LIST}
	)

set_target_properties (
	jnc_samples
	PROPERTIES
	FOLDER samples
	)

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

set (
	JANCY_SAMPLE_LIST
	${FULL_JNC_LIST}
	PARENT_SCOPE
	)

set (
	JANCY_SAMPLE_DIR
	${CMAKE_CURRENT_LIST_DIR}
	PARENT_SCOPE
	)
	
#..............................................................................
