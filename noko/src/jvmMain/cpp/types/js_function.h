#ifndef NOKO_FUNCTION_H
#define NOKO_FUNCTION_H

#include <jni.h>

namespace JsFunction {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_FUNCTION_H
