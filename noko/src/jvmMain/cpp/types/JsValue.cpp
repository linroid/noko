#include <jni.h>
#include "../Node.h"
#include "JsValue.h"
#include "JsError.h"

jmethodID JsValue::jConstructor;
jclass JsValue::jClazz;
jfieldID JsValue::jPointer;
jmethodID JsValue::jNodePointer;

jint JsValue::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsValue");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  jPointer = env->GetFieldID(clazz, "pointer", "J");
  jNodePointer = env->GetMethodID(clazz, "nodePointer", "()J");

  JNINativeMethod methods[] = {
      {"nativeToString", "()Ljava/lang/String;", (void *) JsValue::ToString},
      {"nativeTypeOf",   "()Ljava/lang/String;", (void *) JsValue::TypeOf},
      {"nativeToJson",   "()Ljava/lang/String;", (void *) JsValue::ToJson},
      {"nativeDispose",  "()V",                  (void *) JsValue::Dispose},
      {"nativeToNumber", "()D",                  (void *) JsValue::ToNumber},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

jstring JsValue::ToString(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(isolate);
  if (that.IsEmpty()) {
    return env->NewStringUTF("null");
  }
  auto str = that->ToString(context);
  if (str.IsEmpty()) {
    node->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  v8::String::Value unicodeString(isolate, str.ToLocalChecked());
  return env->NewString(*unicodeString, unicodeString.length());
}

jstring JsValue::TypeOf(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  auto type = that->TypeOf(isolate);
  v8::String::Value unicodeString(isolate, type);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jstring JsValue::ToJson(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(node->isolate_);
  auto str = v8::JSON::Stringify(context, that);
  if (str.IsEmpty()) {
    if (tryCatch.HasCaught()) {
      node->Throw(env, tryCatch.Exception());
      return nullptr;
    }
    return env->NewString(new uint16_t[0], 0);
  }
  auto value = str.ToLocalChecked();
  v8::String::Value unicodeString(isolate, value);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jdouble JsValue::ToNumber(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(node->isolate_);
  auto number = that->ToNumber(context);
  if (number.IsEmpty()) {
    node->Throw(env, tryCatch.Exception());
    return 0.0;
  }
  v8::Local<v8::Number> checked = number.ToLocalChecked();
  return checked->Value();
}

void JsValue::Dispose(JNIEnv *env, jobject jThis) {
  auto value = JsValue::Unwrap(env, jThis);
  value->Reset();
  delete value;
  JsValue::SetPointer(env, jThis, 0);
}
