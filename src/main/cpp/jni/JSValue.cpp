//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"
#include "JSString.h"
#include "JSContext.h"
#include "JSError.h"

jmethodID JSValue::jConstructor;
jclass JSValue::jClazz;
jfieldID JSValue::jReference;
jmethodID JSValue::jGetRuntime;

jint JSValue::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/knode/js/JSValue");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
  jReference = env->GetFieldID(clazz, "reference", "J");
  jGetRuntime = env->GetMethodID(clazz, "runtimePtr", "()J");

  JNINativeMethod methods[] = {
    {"nativeToString", "()Ljava/lang/String;", (void *) JSValue::ToString},
    {"nativeTypeOf",   "()Ljava/lang/String;", (void *) JSValue::TypeOf},
    {"nativeToJson",   "()Ljava/lang/String;", (void *) JSValue::ToJson},
    {"nativeDispose",  "()V",                  (void *) JSValue::Dispose},
    {"nativeToNumber", "()D",                  (void *) JSValue::ToNumber},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

jstring JSValue::ToString(JNIEnv *env, jobject jThis) {
  uint16_t *unicodeChars = nullptr;
  jsize length = 0;
  auto runtime = JSValue::GetRuntime(env, jThis);
  auto isolate = runtime->isolate_;
  auto value = JSValue::Unwrap(env, jThis);
  auto _runnable = [&]() {
    v8::Locker _locker(runtime->isolate_);
    v8::HandleScope _handleScope(runtime->isolate_);
    auto context = runtime->context_.Get(isolate);
    auto str = value->Get(isolate)->ToString(context);
    if (str.IsEmpty()) {
      unicodeChars = new uint16_t[0];
    } else {
      v8::String::Value unicodeString(isolate, str.ToLocalChecked());
      unicodeChars = *unicodeString;
      length = unicodeString.length();
    }
  };
  runtime->Await(_runnable);
  return env->NewString(unicodeChars, length);
}

jstring JSValue::TypeOf(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  auto type = that->TypeOf(isolate);
  v8::String::Value unicodeString(isolate, type);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jstring JSValue::ToJson(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(runtime->isolate_);
  auto str = v8::JSON::Stringify(context, that);
  if (str.IsEmpty()) {
    if (tryCatch.HasCaught()) {
      runtime->Throw(env, tryCatch.Exception());
      return nullptr;
    }
    return env->NewString(new uint16_t[0], 0);
  }
  auto value = str.ToLocalChecked();
  v8::String::Value unicodeString(isolate, value);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jdouble JSValue::ToNumber(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(runtime->isolate_);
  auto number = that->ToNumber(context);
  if (number.IsEmpty()) {
    runtime->Throw(env, tryCatch.Exception());
    return 0.0;
  }
  v8::Local<v8::Number> checked = number.ToLocalChecked();
  return checked->Value();
}

void JSValue::Dispose(JNIEnv *env, jobject jThis) {
  auto value = JSValue::Unwrap(env, jThis);
  value->Reset();
  delete value;
  JSValue::SetReference(env, jThis, 0);
}
