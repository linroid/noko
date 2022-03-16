#ifndef NOKO_JSFUNCTION_H
#define NOKO_JSFUNCTION_H

#include <jni.h>
#include <v8.h>

namespace JsFunction {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

bool Is(JNIEnv *env, jobject obj);

v8::Local<v8::Function> Init(JNIEnv *env, jobject j_this);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_JSFUNCTION_H
