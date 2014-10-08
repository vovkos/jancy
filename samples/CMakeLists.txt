# This file is part of AXL (R) Library
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................
#
# samples folder
#

set (
	SAMPLE_JNC_LIST
	00_HelloWorld.jnc
	01_Classes.jnc
	02_Inheritance.jnc
	10_Storage.jnc
	11_RAII.jnc
	20_ConstCorrectness.jnc
	21_SafeDataPtr.jnc
	22_WeakClassPtr.jnc
	30_SimplePropertyDecl.jnc
	31_FullPropertyDecl.jnc
	32_IndexedProperties.jnc
	33_AutoGetProperties.jnc
	34_BindableProperties.jnc
	40_FunctionPtr.jnc
	42_WeakFunctionPtr.jnc
	43_ScheduleOperator.jnc
	44_PropertyPtr.jnc
	45_WeakPropertyPtr.jnc
	46_String.jnc
	47_Array.jnc
	50_Multicasts.jnc
	51_WeakMulticasts.jnc
	60_Reactors.jnc
	61_OnEventStmt.jnc
	70_DualModifiers.jnc
	71_Once.jnc
	72_CurlyInitializers.jnc
	73_Literals.jnc
	74_BreakN.jnc
	75_ScopesInSwitch.jnc
	76_ModuleConstructors.jnc
	77_ExtensionNamespaces.jnc
	78_Enums.jnc
	80_Exceptions.jnc
	)

source_group (
	samples
	FILES
	${SAMPLE_JNC_LIST}
	)

#..............................................................................
#
# jnc_samples library
#

add_library (
	jnc_samples
	EXCLUDE_FROM_ALL
	${SAMPLE_JNC_LIST}
	)

set_target_properties (
	jnc_samples
	PROPERTIES
	FOLDER samples
	LINKER_LANGUAGE C
	)

set (
	JANCY_SAMPLE_LIST
	${SAMPLE_JNC_LIST}
	PARENT_SCOPE
	)

set (
	JANCY_SAMPLE_DIR
	${CMAKE_CURRENT_LIST_DIR}
	PARENT_SCOPE
	)

#..............................................................................
