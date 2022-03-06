#ifndef NOKO_ERROR_H
#define NOKO_ERROR_H

#include <jni.h>
#include <v8.h>

namespace JsError {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

jthrowable ToException(JNIEnv *env, jobject j_error);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_ERROR_H
