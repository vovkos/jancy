#include "pch.h"

extern "C" {

jnc::ext::ExtensionLib*
getExtensionLib ();

//.............................................................................

JNIEXPORT 
jint 
JNICALL 
Java_JncExtensionLib_sourceFileCount (
	JNIEnv* env, 
	jobject obj
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	return (jint) lib->getSourceFileCount ();
}

JNIEXPORT 
jstring 
JNICALL 
Java_JncExtensionLib_sourceFileName (
	JNIEnv* env, 
	jobject obj, 
	jint index
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	const char* fileName = lib->getSourceFileName (index);
	return env->NewStringUTF (fileName);
}

JNIEXPORT 
jstring 
JNICALL 
Java_JncExtensionLib_sourceFileContents (
	JNIEnv* env, 
	jobject obj, 
	jint index
	)
{
	jnc::ext::ExtensionLib* lib = getExtensionLib ();
	sl::StringSlice source = lib->getSourceFileContents (index);
	return env->NewStringUTF (source);
}

JNIEXPORT 
jstring 
JNICALL 
Java_JncExtensionLib_findSourceFileContents (
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
