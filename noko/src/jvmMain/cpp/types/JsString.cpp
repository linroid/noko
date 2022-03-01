#include <string>
#include "JsString.h"
#include "JsValue.h"

jclass JsString::class_;
jmethodID JsString::init_method_id;

jint JsString::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsString");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;JLjava/lang/String;)V");

  JNINativeMethod methods[] = {
      {"nativeNew", "(Ljava/lang/String;)V", (void *) JsString::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  return JNI_OK;
}

void JsString::New(JNIEnv *env, jobject j_this, jstring j_value) {
  const uint16_t *content = env->GetStringChars(j_value, nullptr);
  const jint content_len = env->GetStringLength(j_value);
  V8_SCOPE(env, j_this)
  auto result = new v8::Persistent<v8::Value>(isolate, V8_STRING(isolate, content, content_len));
  env->ReleaseStringChars(j_value, content);
  JsValue::SetPointer(env, j_this, (jlong) result);
}
