#ifndef NOKO_UTIL_JNI_HELPER_H_
#define NOKO_UTIL_JNI_HELPER_H_
#include <jni.h>
#include <string>
namespace JniHelper {

std::string GetClassName(JNIEnv *env, jobject obj);

std::string GetString(JNIEnv *env, jstring str);

std::string GetExceptionMessage(JNIEnv *env, jthrowable exception);

std::string GetStackTrace(JNIEnv *env, jthrowable exception);
}

#endif //NOKO_UTIL_JNI_HELPER_H_
