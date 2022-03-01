#include <jni.h>
#include "../NodeRuntime.h"
#include "JsValue.h"
#include "JsError.h"

jmethodID JsValue::init_method_id_;
jclass JsValue::class_;
jfieldID JsValue::pointer_field_id_;
jmethodID JsValue::get_node_pointer_id_;

jint JsValue::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsValue");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  pointer_field_id_ = env->GetFieldID(clazz, "pointer", "J");
  get_node_pointer_id_ = env->GetMethodID(clazz, "nodePointer", "()J");

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

jstring JsValue::ToString(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value)
  v8::TryCatch try_catch(isolate);
  if (that.IsEmpty()) {
    return env->NewStringUTF("null");
  }
  auto str = that->ToString(context);
  if (str.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  v8::String::Value unique_string(isolate, str.ToLocalChecked());
  return env->NewString(*unique_string, unique_string.length());
}

jstring JsValue::TypeOf(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value)
  auto type = that->TypeOf(isolate);
  v8::String::Value unicode_string(isolate, type);
  uint16_t *unicode_chars = *unicode_string;
  return env->NewString(unicode_chars, unicode_string.length());
}

jstring JsValue::ToJson(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value)
  v8::TryCatch try_catch(runtime->isolate_);
  auto str = v8::JSON::Stringify(context, that);
  if (str.IsEmpty()) {
    if (try_catch.HasCaught()) {
      runtime->Throw(env, try_catch.Exception());
      return nullptr;
    }
    return env->NewString(new uint16_t[0], 0);
  }
  auto value = str.ToLocalChecked();
  v8::String::Value unicode_string(isolate, value);
  uint16_t *unicode_chars = *unicode_string;
  return env->NewString(unicode_chars, unicode_string.length());
}

jdouble JsValue::ToNumber(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value)
  v8::TryCatch try_catch(runtime->isolate_);
  auto number = that->ToNumber(context);
  if (number.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return 0.0;
  }
  v8::Local<v8::Number> checked = number.ToLocalChecked();
  return checked->Value();
}

void JsValue::Dispose(JNIEnv *env, jobject j_this) {
  auto value = JsValue::Unwrap(env, j_this);
  value->Reset();
  delete value;
  JsValue::SetPointer(env, j_this, 0);
}
