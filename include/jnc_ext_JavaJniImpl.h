// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#ifndef _JNC_SHARED_EXTENSION_LIB
#	error This file should only be included from shared extension libs.
#endif

#include "jnc_ext_ExtensionLib.h"

//.............................................................................

jnc::ext::ExtensionLib*
getExtensionLib ();

extern "C" {

inline
JNIEXPORT 
jint 
JNICALL 
Java_com_tibbo_jnc_ExtensionLib_sourceFileCount (
	JNIEnv* env, 
	jobject obj
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	return (jint) lib->getSourceFileCount ();
}

inline
JNIEXPORT 
jstring 
JNICALL 
Java_com_tibbo_jnc_ExtensionLib_sourceFileName (
	JNIEnv* env, 
	jobject obj, 
	jint index
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	const char* fileName = lib->getSourceFileName (index);
	return env->NewStringUTF (fileName);
}

inline
JNIEXPORT 
jstring 
JNICALL 
Java_com_tibbo_jnc_ExtensionLib_sourceFileContents (
	JNIEnv* env, 
	jobject obj, 
	jint index
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	sl::StringSlice source = lib->getSourceFileContents (index);
	return env->NewStringUTF (source);
}

inline
JNIEXPORT 
jstring 
JNICALL 
Java_com_tibbo_jnc_ExtensionLib_findSourceFileContents (
	JNIEnv* env, 
	jobject obj, 
	jstring fileName_j
	)
{
	const char* fileName_c = env->GetStringUTFChars (fileName_j, NULL);
	if (!fileName_c) 
		return NULL;
 
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	sl::StringSlice source = lib->findSourceFileContents (fileName_c);
	env->ReleaseStringUTFChars (fileName_j, fileName_c);  // release resources
	return env->NewStringUTF (source);
}

//.............................................................................

} // extern "C"
