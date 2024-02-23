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

set(
	INC_H_LIST
	include/jnc_ct_Alias.h
	include/jnc_ct_ArrayType.h
	include/jnc_ct_AsyncLauncherFunction.h
	include/jnc_ct_AsyncSchedLauncherFunction.h
	include/jnc_ct_AsyncSequencerFunction.h
	include/jnc_ct_AttributeBlock.h
	include/jnc_ct_AttributeMgr.h
	include/jnc_ct_BasicBlock.h
	include/jnc_ct_BinOp.h
	include/jnc_ct_BinOp_Arithmetic.h
	include/jnc_ct_BinOp_Assign.h
	include/jnc_ct_BinOp_At.h
	include/jnc_ct_BinOp_Cmp.h
	include/jnc_ct_BinOp_Idx.h
	include/jnc_ct_BinOp_Logic.h
	include/jnc_ct_CallConv.h
	include/jnc_ct_CallConv_gcc32.h
	include/jnc_ct_CallConv_msc32.h
	include/jnc_ct_CapabilityMgr.h
	include/jnc_ct_CastOp.h
	include/jnc_ct_CastOp_Array.h
	include/jnc_ct_CastOp_Bool.h
	include/jnc_ct_CastOp_ClassPtr.h
	include/jnc_ct_CastOp_DataPtr.h
	include/jnc_ct_CastOp_Fp.h
	include/jnc_ct_CastOp_FunctionPtr.h
	include/jnc_ct_CastOp_Int.h
	include/jnc_ct_CastOp_PropertyPtr.h
	include/jnc_ct_CastOp_Struct.h
	include/jnc_ct_CastOp_Variant.h
	include/jnc_ct_CdeclCallConv_arm.h
	include/jnc_ct_CdeclCallConv_gcc64.h
	include/jnc_ct_CdeclCallConv_msc64.h
	include/jnc_ct_ClassPtrType.h
	include/jnc_ct_ClassType.h
	include/jnc_ct_Closure.h
	include/jnc_ct_ClosureClassType.h
	include/jnc_ct_CodeAssistMgr.h
	include/jnc_ct_ConstMgr.h
	include/jnc_ct_ControlFlowMgr.h
	include/jnc_ct_DataPtrType.h
	include/jnc_ct_Decl.h
	include/jnc_ct_DeclTypeCalc.h
	include/jnc_ct_DerivableType.h
	include/jnc_ct_DoxyHost.h
	include/jnc_ct_DynamicLibClassType.h
	include/jnc_ct_DynamicLibNamespace.h
	include/jnc_ct_EnumType.h
	include/jnc_ct_ExtensionLibMgr.h
	include/jnc_ct_ExtensionNamespace.h
	include/jnc_ct_Field.h
	include/jnc_ct_Function.h
	include/jnc_ct_FunctionArg.h
	include/jnc_ct_FunctionMgr.h
	include/jnc_ct_FunctionName.h
	include/jnc_ct_FunctionOverload.h
	include/jnc_ct_FunctionPtrType.h
	include/jnc_ct_FunctionType.h
	include/jnc_ct_FunctionTypeOverload.h
	include/jnc_ct_GcShadowStackMgr.h
	include/jnc_ct_GlobalNamespace.h
	include/jnc_ct_ImportMgr.h
	include/jnc_ct_ImportType.h
	include/jnc_ct_Jit.h
	include/jnc_ct_JnccallCallConv.h
	include/jnc_ct_LeanDataPtrValidator.h
	include/jnc_ct_Lexer.h
	include/jnc_ct_LlvmDiBuilder.h
	include/jnc_ct_LlvmIrBuilder.h
	include/jnc_ct_LlvmIrInsertPoint.h
	include/jnc_ct_McJit.h
	include/jnc_ct_McSnapshotClassType.h
	include/jnc_ct_MemberBlock.h
	include/jnc_ct_Module.h
	include/jnc_ct_ModuleItem.h
	include/jnc_ct_MulticastClassType.h
	include/jnc_ct_Namespace.h
	include/jnc_ct_NamespaceMgr.h
	include/jnc_ct_OperatorMgr.h
	include/jnc_ct_OrcJit.h
	include/jnc_ct_Orphan.h
	include/jnc_ct_Pch.h
	include/jnc_ct_PragmaMgr.h
	include/jnc_ct_Property.h
	include/jnc_ct_PropertyPtrType.h
	include/jnc_ct_PropertyTemplate.h
	include/jnc_ct_PropertyType.h
	include/jnc_ct_PropertyVerifier.h
	include/jnc_ct_QualifiedName.h
	include/jnc_ct_ReactorClassType.h
	include/jnc_ct_SchedLauncherFunction.h
	include/jnc_ct_Scope.h
	include/jnc_ct_StdFunction.h
	include/jnc_ct_StdNamespace.h
	include/jnc_ct_StdType.h
	include/jnc_ct_StructType.h
	include/jnc_ct_ThunkFunction.h
	include/jnc_ct_ThunkProperty.h
	include/jnc_ct_Type.h
	include/jnc_ct_TypeMgr.h
	include/jnc_ct_UnionType.h
	include/jnc_ct_UnitMgr.h
	include/jnc_ct_UnOp.h
	include/jnc_ct_UnOp_Arithmetic.h
	include/jnc_ct_UnOp_Inc.h
	include/jnc_ct_UnOp_LogNot.h
	include/jnc_ct_UnOp_Ptr.h
	include/jnc_ct_UsingSet.h
	include/jnc_ct_Value.h
	include/jnc_ct_Variable.h
	include/jnc_ct_VariableMgr.h
)

source_group(
	include
	FILES
	${INC_H_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_CallConv folder
#

set(
	CALLCONV_H_LIST
	jnc_ct_CallConv/jnc_ct_CallConv.h
	jnc_ct_CallConv/jnc_ct_CallConv_gcc32.h
	jnc_ct_CallConv/jnc_ct_CallConv_msc32.h
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_arm.h
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_gcc64.h
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_msc64.h
	jnc_ct_CallConv/jnc_ct_JnccallCallConv.h
)

set(
	CALLCONV_CPP_LIST
	jnc_ct_CallConv/jnc_ct_CallConv.cpp
	jnc_ct_CallConv/jnc_ct_CallConv_gcc32.cpp
	jnc_ct_CallConv/jnc_ct_CallConv_msc32.cpp
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_arm.cpp
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_gcc64.cpp
	jnc_ct_CallConv/jnc_ct_CdeclCallConv_msc64.cpp
	jnc_ct_CallConv/jnc_ct_JnccallCallConv.cpp
)

source_group(
	jnc_ct_CallConv
	FILES
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_ControlFlowMgr folder
#

set(
	CONTROLFLOWMGR_H_LIST
	jnc_ct_ControlFlowMgr/jnc_ct_BasicBlock.h
	jnc_ct_ControlFlowMgr/jnc_ct_ControlFlowMgr.h
)

set(
	CONTROLFLOWMGR_CPP_LIST
	jnc_ct_ControlFlowMgr/jnc_ct_BasicBlock.cpp
	jnc_ct_ControlFlowMgr/jnc_ct_ControlFlowMgr.cpp
	jnc_ct_ControlFlowMgr/jnc_ct_ControlFlowMgr_Eh.cpp
	jnc_ct_ControlFlowMgr/jnc_ct_ControlFlowMgr_Stmt.cpp
)

source_group(
	jnc_ct_ControlFlowMgr
	FILES
	${CONTROLFLOWMGR_H_LIST}
	${CONTROLFLOWMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_FunctionMgr folder
#

set(
	FUNCTIONMGR_H_LIST
	jnc_ct_FunctionMgr/jnc_ct_AsyncLauncherFunction.h
	jnc_ct_FunctionMgr/jnc_ct_AsyncRegionMgr.h
	jnc_ct_FunctionMgr/jnc_ct_AsyncSchedLauncherFunction.h
	jnc_ct_FunctionMgr/jnc_ct_AsyncSequencerFunction.h
	jnc_ct_FunctionMgr/jnc_ct_Function.h
	jnc_ct_FunctionMgr/jnc_ct_FunctionMgr.h
	jnc_ct_FunctionMgr/jnc_ct_FunctionName.h
	jnc_ct_FunctionMgr/jnc_ct_FunctionOverload.h
	jnc_ct_FunctionMgr/jnc_ct_Property.h
	jnc_ct_FunctionMgr/jnc_ct_PropertyTemplate.h
	jnc_ct_FunctionMgr/jnc_ct_PropertyVerifier.h
	jnc_ct_FunctionMgr/jnc_ct_SchedLauncherFunction.h
	jnc_ct_FunctionMgr/jnc_ct_StdFunction.h
	jnc_ct_FunctionMgr/jnc_ct_ThunkFunction.h
	jnc_ct_FunctionMgr/jnc_ct_ThunkProperty.h
)

set(
	FUNCTIONMGR_CPP_LIST
	jnc_ct_FunctionMgr/jnc_ct_AsyncLauncherFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_AsyncRegionMgr.cpp
	jnc_ct_FunctionMgr/jnc_ct_AsyncSchedLauncherFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_AsyncSequencerFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_Function.cpp
	jnc_ct_FunctionMgr/jnc_ct_FunctionMgr.cpp
	jnc_ct_FunctionMgr/jnc_ct_FunctionOverload.cpp
	jnc_ct_FunctionMgr/jnc_ct_Property.cpp
	jnc_ct_FunctionMgr/jnc_ct_PropertyTemplate.cpp
	jnc_ct_FunctionMgr/jnc_ct_PropertyVerifier.cpp
	jnc_ct_FunctionMgr/jnc_ct_SchedLauncherFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_StdFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_ThunkFunction.cpp
	jnc_ct_FunctionMgr/jnc_ct_ThunkProperty.cpp
)

source_group(
	jnc_ct_FunctionMgr
	FILES
	${FUNCTIONMGR_H_LIST}
	${FUNCTIONMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_NamespaceMgr folder
#

set(
	NAMESPACEMGR_H_LIST
	jnc_ct_NamespaceMgr/jnc_ct_Alias.h
	jnc_ct_NamespaceMgr/jnc_ct_DynamicLibNamespace.h
	jnc_ct_NamespaceMgr/jnc_ct_ExtensionNamespace.h
	jnc_ct_NamespaceMgr/jnc_ct_GlobalNamespace.h
	jnc_ct_NamespaceMgr/jnc_ct_MemberBlock.h
	jnc_ct_NamespaceMgr/jnc_ct_Namespace.h
	jnc_ct_NamespaceMgr/jnc_ct_NamespaceMgr.h
	jnc_ct_NamespaceMgr/jnc_ct_Orphan.h
	jnc_ct_NamespaceMgr/jnc_ct_QualifiedName.h
	jnc_ct_NamespaceMgr/jnc_ct_Scope.h
	jnc_ct_NamespaceMgr/jnc_ct_StdNamespace.h
	jnc_ct_NamespaceMgr/jnc_ct_UsingSet.h
)

set(
	NAMESPACEMGR_CPP_LIST
	jnc_ct_NamespaceMgr/jnc_ct_Alias.cpp
	jnc_ct_NamespaceMgr/jnc_ct_DynamicLibNamespace.cpp
	jnc_ct_NamespaceMgr/jnc_ct_ExtensionNamespace.cpp
	jnc_ct_NamespaceMgr/jnc_ct_GlobalNamespace.cpp
	jnc_ct_NamespaceMgr/jnc_ct_MemberBlock.cpp
	jnc_ct_NamespaceMgr/jnc_ct_Namespace.cpp
	jnc_ct_NamespaceMgr/jnc_ct_NamespaceMgr.cpp
	jnc_ct_NamespaceMgr/jnc_ct_Orphan.cpp
	jnc_ct_NamespaceMgr/jnc_ct_QualifiedName.cpp
	jnc_ct_NamespaceMgr/jnc_ct_Scope.cpp
	jnc_ct_NamespaceMgr/jnc_ct_UsingSet.cpp
)

source_group(
	jnc_ct_NamespaceMgr
	FILES
	${NAMESPACEMGR_H_LIST}
	${NAMESPACEMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_OperatorMgr folder
#

set(
	OPERATORMGR_H_LIST
	jnc_ct_OperatorMgr/jnc_ct_BinOp.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Arithmetic.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Assign.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_At.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Cmp.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Idx.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Logic.h
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Match.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Array.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Bool.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_ClassPtr.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_DataPtr.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Fp.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_FunctionPtr.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Int.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_PropertyPtr.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_String.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Struct.h
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Variant.h
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr.h
	jnc_ct_OperatorMgr/jnc_ct_UnOp.h
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Arithmetic.h
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Inc.h
	jnc_ct_OperatorMgr/jnc_ct_UnOp_LogNot.h
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Ptr.h
)

set(
	OPERATORMGR_CPP_LIST
	jnc_ct_OperatorMgr/jnc_ct_BinOp.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Arithmetic.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Assign.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_At.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Cmp.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Idx.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Logic.cpp
	jnc_ct_OperatorMgr/jnc_ct_BinOp_Match.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Array.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Bool.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_ClassPtr.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_DataPtr.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Fp.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_FunctionPtr.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Int.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_PropertyPtr.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_String.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Struct.cpp
	jnc_ct_OperatorMgr/jnc_ct_CastOp_Variant.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Call.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_CheckPtr.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Closure.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_DataRef.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Field.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Gc.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Logic.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Member.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_New.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Property.cpp
	jnc_ct_OperatorMgr/jnc_ct_OperatorMgr_Virtual.cpp
	jnc_ct_OperatorMgr/jnc_ct_UnOp.cpp
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Arithmetic.cpp
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Inc.cpp
	jnc_ct_OperatorMgr/jnc_ct_UnOp_LogNot.cpp
	jnc_ct_OperatorMgr/jnc_ct_UnOp_Ptr.cpp
)

source_group(
	jnc_ct_OperatorMgr
	FILES
	${OPERATORMGR_H_LIST}
	${OPERATORMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_TypeMgr folder
#

set(
	TYPEMGR_H_LIST
	jnc_ct_TypeMgr/jnc_ct_ArrayType.h
	jnc_ct_TypeMgr/jnc_ct_ClassPtrType.h
	jnc_ct_TypeMgr/jnc_ct_ClassType.h
	jnc_ct_TypeMgr/jnc_ct_ClosureClassType.h
	jnc_ct_TypeMgr/jnc_ct_DataPtrType.h
	jnc_ct_TypeMgr/jnc_ct_DerivableType.h
	jnc_ct_TypeMgr/jnc_ct_DynamicLibClassType.h
	jnc_ct_TypeMgr/jnc_ct_EnumType.h
	jnc_ct_TypeMgr/jnc_ct_Field.h
	jnc_ct_TypeMgr/jnc_ct_FunctionArg.h
	jnc_ct_TypeMgr/jnc_ct_FunctionPtrType.h
	jnc_ct_TypeMgr/jnc_ct_FunctionType.h
	jnc_ct_TypeMgr/jnc_ct_FunctionTypeOverload.h
	jnc_ct_TypeMgr/jnc_ct_ImportType.h
	jnc_ct_TypeMgr/jnc_ct_McSnapshotClassType.h
	jnc_ct_TypeMgr/jnc_ct_MulticastClassType.h
	jnc_ct_TypeMgr/jnc_ct_PropertyPtrType.h
	jnc_ct_TypeMgr/jnc_ct_PropertyType.h
	jnc_ct_TypeMgr/jnc_ct_ReactorClassType.h
	jnc_ct_TypeMgr/jnc_ct_StdType.h
	jnc_ct_TypeMgr/jnc_ct_StructType.h
	jnc_ct_TypeMgr/jnc_ct_Type.h
	jnc_ct_TypeMgr/jnc_ct_TypeMgr.h
	jnc_ct_TypeMgr/jnc_ct_UnionType.h
)

set(
	TYPEMGR_CPP_LIST
	jnc_ct_TypeMgr/jnc_ct_ArrayType.cpp
	jnc_ct_TypeMgr/jnc_ct_ClassPtrType.cpp
	jnc_ct_TypeMgr/jnc_ct_ClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_ClosureClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_DataPtrType.cpp
	jnc_ct_TypeMgr/jnc_ct_DerivableType.cpp
	jnc_ct_TypeMgr/jnc_ct_DynamicLibClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_EnumType.cpp
	jnc_ct_TypeMgr/jnc_ct_Field.cpp
	jnc_ct_TypeMgr/jnc_ct_FunctionArg.cpp
	jnc_ct_TypeMgr/jnc_ct_FunctionPtrType.cpp
	jnc_ct_TypeMgr/jnc_ct_FunctionType.cpp
	jnc_ct_TypeMgr/jnc_ct_FunctionTypeOverload.cpp
	jnc_ct_TypeMgr/jnc_ct_ImportType.cpp
	jnc_ct_TypeMgr/jnc_ct_McSnapshotClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_MulticastClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_PropertyPtrType.cpp
	jnc_ct_TypeMgr/jnc_ct_PropertyType.cpp
	jnc_ct_TypeMgr/jnc_ct_ReactorClassType.cpp
	jnc_ct_TypeMgr/jnc_ct_StdType.cpp
	jnc_ct_TypeMgr/jnc_ct_StructType.cpp
	jnc_ct_TypeMgr/jnc_ct_Type.cpp
	jnc_ct_TypeMgr/jnc_ct_TypeMgr.cpp
	jnc_ct_TypeMgr/jnc_ct_UnionType.cpp
)

source_group(
	jnc_ct_TypeMgr
	FILES
	${TYPEMGR_H_LIST}
	${TYPEMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_VariableMgr folder
#

set(
	VARIABLEMGR_H_LIST
	jnc_ct_VariableMgr/jnc_ct_Variable.h
	jnc_ct_VariableMgr/jnc_ct_VariableMgr.h
)

set(
	VARIABLEMGR_CPP_LIST
	jnc_ct_VariableMgr/jnc_ct_Variable.cpp
	jnc_ct_VariableMgr/jnc_ct_VariableMgr.cpp
)

source_group(
	jnc_ct_VariableMgr
	FILES
	${VARIABLEMGR_H_LIST}
	${VARIABLEMGR_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Value folder
#

set(
	VALUE_H_LIST
	jnc_ct_Value/jnc_ct_Closure.h
	jnc_ct_Value/jnc_ct_ConstMgr.h
	jnc_ct_Value/jnc_ct_LeanDataPtrValidator.h
	jnc_ct_Value/jnc_ct_Value.h
)

set(
	VALUE_CPP_LIST
	jnc_ct_Value/jnc_ct_Closure.cpp
	jnc_ct_Value/jnc_ct_ConstMgr.cpp
	jnc_ct_Value/jnc_ct_LeanDataPtrValidator.cpp
	jnc_ct_Value/jnc_ct_Value.cpp
)

source_group(
	jnc_ct_Value
	FILES
	${VALUE_H_LIST}
	${VALUE_CPP_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Module folder
#

set(
	MODULE_H_LIST
	jnc_ct_Module/jnc_ct_AttributeBlock.h
	jnc_ct_Module/jnc_ct_AttributeMgr.h
	jnc_ct_Module/jnc_ct_CapabilityMgr.h
	jnc_ct_Module/jnc_ct_CodeAssistMgr.h
	jnc_ct_Module/jnc_ct_DoxyHost.h
	jnc_ct_Module/jnc_ct_ExtensionLibMgr.h
	jnc_ct_Module/jnc_ct_GcShadowStackMgr.h
	jnc_ct_Module/jnc_ct_ImportMgr.h
	jnc_ct_Module/jnc_ct_Jit.h
	jnc_ct_Module/jnc_ct_LegacyJit.h
	jnc_ct_Module/jnc_ct_LlvmDiBuilder.h
	jnc_ct_Module/jnc_ct_LlvmIrBuilder.h
	jnc_ct_Module/jnc_ct_LlvmIrInsertPoint.h
	jnc_ct_Module/jnc_ct_McJit.h
	jnc_ct_Module/jnc_ct_Module.h
	jnc_ct_Module/jnc_ct_ModuleItem.h
	jnc_ct_Module/jnc_ct_OrcJit.h
	jnc_ct_Module/jnc_ct_PragmaMgr.h
	jnc_ct_Module/jnc_ct_UnitMgr.h
)

set(
	MODULE_CPP_LIST
	jnc_ct_Module/jnc_ct_AttributeBlock.cpp
	jnc_ct_Module/jnc_ct_AttributeMgr.cpp
	jnc_ct_Module/jnc_ct_CapabilityMgr.cpp
	jnc_ct_Module/jnc_ct_CodeAssistMgr.cpp
	jnc_ct_Module/jnc_ct_DoxyHost.cpp
	jnc_ct_Module/jnc_ct_ExtensionLibMgr.cpp
	jnc_ct_Module/jnc_ct_GcShadowStackMgr.cpp
	jnc_ct_Module/jnc_ct_ImportMgr.cpp
	jnc_ct_Module/jnc_ct_Jit.cpp
	jnc_ct_Module/jnc_ct_LegacyJit.cpp
	jnc_ct_Module/jnc_ct_LlvmDiBuilder.cpp
	jnc_ct_Module/jnc_ct_LlvmIrBuilder.cpp
	jnc_ct_Module/jnc_ct_McJit.cpp
	jnc_ct_Module/jnc_ct_Module.cpp
	jnc_ct_Module/jnc_ct_ModuleItem.cpp
	jnc_ct_Module/jnc_ct_OrcJit.cpp
	jnc_ct_Module/jnc_ct_PragmaMgr.cpp
	jnc_ct_Module/jnc_ct_UnitMgr.cpp
)

source_group(
	jnc_ct_Module
	FILES
	${MODULE_H_LIST}
	${MODULE_CPP_LIST}
)

if(${LLVM_VERSION} VERSION_GREATER_EQUAL 3.6)
	axl_exclude_from_build(jnc_ct_Module/jnc_ct_LegacyJit.cpp)
endif()

if(${LLVM_VERSION} VERSION_LESS 12.0)
	axl_exclude_from_build(jnc_ct_Module/jnc_ct_OrcJit.cpp)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc_ct_Parser folder
#

set(
	PARSER_H_LIST
	jnc_ct_Parser/jnc_ct_Decl.h
	jnc_ct_Parser/jnc_ct_DeclTypeCalc.h
	jnc_ct_Parser/jnc_ct_Lexer.h
)

set(
	PARSER_CPP_LIST
	jnc_ct_Parser/jnc_ct_Decl.cpp
	jnc_ct_Parser/jnc_ct_DeclTypeCalc.cpp
	jnc_ct_Parser/jnc_ct_Lexer.cpp
	jnc_ct_Parser/jnc_ct_Parser.cpp
)

set(
	PARSER_RL_LIST
	jnc_ct_Parser/jnc_ct_Lexer.rl
)

set(
	PARSER_LLK_LIST
	jnc_ct_Parser/jnc_ct_Decl.llk
	jnc_ct_Parser/jnc_ct_DeclarationSpecifier.llk
	jnc_ct_Parser/jnc_ct_Declarator.llk
	jnc_ct_Parser/jnc_ct_Expr.llk
	jnc_ct_Parser/jnc_ct_NamedTypeSpecifier.llk
	jnc_ct_Parser/jnc_ct_Parser.llk
	jnc_ct_Parser/jnc_ct_Reactor.llk
	jnc_ct_Parser/jnc_ct_Stmt.llk
)

source_group(
	jnc_ct_Parser
	FILES
	${PARSER_H_LIST}
	${PARSER_CPP_LIST}
	${PARSER_RL_LIST}
	${PARSER_LLK_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# perl folder
#

set(
	PERL_PL_LIST
	${JNC2CPP_PL}
	${FILE2LITERAL_PL}
)

source_group(
	perl
	FILES
	${PERL_PL_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# rtl_core_jnc folder
#

set(RTL_CORE_JNC_DIR ${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_core/jnc)

set(
	RTL_CORE_JNC_LIST
	${RTL_CORE_JNC_DIR}/jnc_gc.jnc
	${RTL_CORE_JNC_DIR}/jnc_DataPtr.jnc
	${RTL_CORE_JNC_DIR}/jnc_DynamicLib.jnc
	${RTL_CORE_JNC_DIR}/jnc_Promise.jnc
	${RTL_CORE_JNC_DIR}/jnc_Regex.jnc
	${RTL_CORE_JNC_DIR}/jnc_Scheduler.jnc
)

set(
	RTL_CORE_LEGACY_JNC_LIST
	${RTL_CORE_JNC_DIR}/jnc_StdTypes.jnc
	${RTL_CORE_JNC_DIR}/jnc_StdFunctions.jnc
)

source_group(
	jnc_rtl_core_jnc
	FILES
	${RTL_CORE_JNC_LIST}
	${RTL_CORE_LEGACY_JNC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# rtl_intro_jnc folder
#

set(RTL_INTRO_JNC_DIR ${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_intro/jnc)

set(
	RTL_INTRO_JNC_LIST
	${RTL_INTRO_JNC_DIR}/jnc_intro.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Alias.jnc
	${RTL_INTRO_JNC_DIR}/jnc_ArrayType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_AttributeBlock.jnc
	${RTL_INTRO_JNC_DIR}/jnc_ClassType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_DerivableType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_EnumType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Field.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Function.jnc
	${RTL_INTRO_JNC_DIR}/jnc_FunctionType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_MemberBlock.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Module.jnc
	${RTL_INTRO_JNC_DIR}/jnc_ModuleItem.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Namespace.jnc
	${RTL_INTRO_JNC_DIR}/jnc_OpKind.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Property.jnc
	${RTL_INTRO_JNC_DIR}/jnc_PropertyType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_StructType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Type.jnc
	${RTL_INTRO_JNC_DIR}/jnc_UnionType.jnc
	${RTL_INTRO_JNC_DIR}/jnc_Variable.jnc
)

source_group(
	jnc_rtl_intro_jnc
	FILES
	${RTL_INTRO_JNC_LIST}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# gen folder
#

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

axl_push_and_set(CMAKE_CURRENT_BINARY_DIR ${GEN_DIR})

set(GEN_JNC_CPP_LIST)

foreach(_JNC ${RTL_CORE_LEGACY_JNC_LIST})
	get_filename_component(_NAME ${_JNC} NAME)

	add_perl_step(
		${_NAME}.cpp
		${_JNC}
		${JNC2CPP_PL}
	)

	list(APPEND GEN_JNC_CPP_LIST ${GEN_DIR}/${_NAME}.cpp)
endforeach()

foreach(_JNC ${RTL_CORE_JNC_LIST} ${RTL_INTRO_JNC_LIST})
	get_filename_component(_NAME ${_JNC} NAME)

	add_perl_step(
		${_NAME}.cpp
		${_JNC}
		${FILE2LITERAL_PL}
	)

	list(APPEND GEN_JNC_CPP_LIST ${GEN_DIR}/${_NAME}.cpp)
endforeach()

add_ragel_step(
	jnc_ct_Lexer.rl.cpp
	jnc_ct_Parser/jnc_ct_Lexer.rl
)

add_graco_double_step(
	jnc_ct_Parser.llk.h
	jnc_ct_Parser.llk.cpp
	CppParser.h.in
	CppParser.cpp.in
	jnc_ct_Parser/jnc_ct_Parser.llk

	jnc_ct_Parser/jnc_ct_Decl.llk
	jnc_ct_Parser/jnc_ct_DeclarationSpecifier.llk
	jnc_ct_Parser/jnc_ct_Declarator.llk
	jnc_ct_Parser/jnc_ct_Expr.llk
	jnc_ct_Parser/jnc_ct_NamedTypeSpecifier.llk
	jnc_ct_Parser/jnc_ct_Reactor.llk
	jnc_ct_Parser/jnc_ct_Stmt.llk
)

axl_pop(CMAKE_CURRENT_BINARY_DIR)

set(
	GEN_RL_CPP_LIST
	${GEN_DIR}/jnc_ct_Lexer.rl.cpp
)

set(
	GEN_LLK_H_LIST
	${GEN_DIR}/jnc_ct_Parser.llk.h
)

set(
	GEN_LLK_CPP_LIST
	${GEN_DIR}/jnc_ct_Parser.llk.cpp
)

axl_exclude_from_build(${GEN_JNC_CPP_LIST}) # include "*.jnc.cpp" manually
axl_exclude_from_build(${GEN_RL_CPP_LIST})  # include "*.rl.cpp" manually
axl_exclude_from_build(${GEN_LLK_CPP_LIST}) # include "*.llk.cpp" manually

source_group(
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

set(PCH_H     pch.h)

source_group(
	pch
	FILES
	${PCH_H}
	REGULAR_EXPRESSION cmake_pch
)

#...............................................................................
#
# jnc_ct library
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${GRACO_INC_DIR}
	${JANCY_INC_DIR}
	${CMAKE_CURRENT_LIST_DIR} # for pch.h
	${JANCY_SRC_DIR}/jnc_rt
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_ext/jnc_rtl_core
	${GEN_DIR}
)

if(LINUX)
	include_directories(${OPENSSL_INC_DIR})
endif()

add_library(
	jnc_ct
	${PCH_H}
	${INC_H_LIST}
	${CALLCONV_H_LIST}
	${CALLCONV_CPP_LIST}
	${CONTROLFLOWMGR_H_LIST}
	${CONTROLFLOWMGR_CPP_LIST}
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
	${PERL_PL_LIST}
	${RTL_CORE_JNC_LIST}
	${RTL_INTRO_JNC_LIST}
	${GEN_JNC_CPP_LIST}
	${GEN_RL_CPP_LIST}
	${GEN_LLK_H_LIST}
	# ${GEN_LLK_CPP_LIST} # currently, cmake can't handle double-file output
)

target_precompile_headers(
	jnc_ct
	PRIVATE
	${PCH_H}
)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_ct
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
)

#...............................................................................
