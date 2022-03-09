#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../runtime.h"
#include "../util/jni_helper.h"
#include "integer.h"
#include "long.h"
#include "double.h"
#include "string.h"
#include "boolean.h"
#include "js_function.h"

namespace JsValue {

v8::Persistent<v8::Value> *GetPointer(JNIEnv *env, jobject obj);

void SetPointer(JNIEnv *env, jobject obj, v8::Persistent<v8::Value> *value);

jobject Of(JNIEnv *env, v8::Local<v8::Value> value);

bool Is(JNIEnv *env, jobject obj);

v8::Local<v8::Value> Value(JNIEnv *env, jobject obj);

jint OnLoad(JNIEnv *env);

}

#endif //NODE_JSVALUE_H
