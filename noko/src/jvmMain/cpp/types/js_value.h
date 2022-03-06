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

jobject Of(JNIEnv *env, jobject node, jlong pointer);

bool Is(JNIEnv *env, jobject obj);

inline v8::Local<v8::Value> Value(
    v8::Isolate *isolate,
    JNIEnv *env,
    jobject obj) {
  if (obj == nullptr) {
    return v8::Null(isolate);
  } else if (JsValue::Is(env, obj)) {
    if (JsFunction::Is(env, obj)) {
      auto pointer = JsValue::GetPointer(env, obj);
      if (pointer == nullptr) {
        return JsFunction::Init(env, obj);
      }
      return pointer->Get(isolate);
    }
    return JsValue::GetPointer(env, obj)->Get(isolate);
  } else if (String::Is(env, obj)) {
    return String::Value(env, (jstring) obj);
  } else if (Boolean::Is(env, obj)) {
    return Boolean::Value(env, obj) ? v8::True(isolate) : v8::False(isolate);
  } else if (Integer::Is(env, obj)) {
    return v8::Int32::New(isolate, Integer::Value(env, obj));
  } else if (Long::Is(env, obj)) {
    return v8::BigInt::New(isolate, Long::Value(env, obj));
  } else if (Double::Is(env, obj)) {
    return v8::Number::New(isolate, Double::Value(env, obj));
  } else {
    auto class_name = JniHelper::GetClassName(env, obj);
    LOGE("Not supported type: %s", class_name.c_str());
    abort();
  }
}

jint OnLoad(JNIEnv *env);

}

#endif //NODE_JSVALUE_H
