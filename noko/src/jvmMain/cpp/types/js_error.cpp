#include "js_value.h"
#include "string.h"
#include "js_error.h"

jclass JsError::class_;
jmethodID JsError::init_method_id_;
jclass JsError::exception_class_;
jmethodID JsError::exception_init_method_id_;

jint JsError::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsError");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  JNINativeMethod methods[] = {
      {"nativeNew", "(Ljava/lang/String;)V", (void *) JsError::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  clazz = env->FindClass("com/linroid/noko/JsException");
  if (!clazz) {
    return JNI_ERR;
  }
  exception_class_ = (jclass) env->NewGlobalRef(clazz);
  exception_init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JsObject;)V");

  return JNI_OK;
}

void JsError::New(JNIEnv *env, jobject j_this, jstring j_message) {
  auto message_chars = env->GetStringChars(j_message, nullptr);
  const jint message_len = env->GetStringLength(j_message);

  V8_SCOPE(env, j_this)
  auto message = V8_STRING(isolate, message_chars, message_len);
  auto value = v8::Exception::Error(message);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  env->ReleaseStringChars(j_message, message_chars);
  JsValue::SetPointer(env, j_this, (jlong) result);
}
