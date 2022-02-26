#include <string>
#include "JsString.h"
#include "JsValue.h"

jclass JsString::jClazz;
jmethodID JsString::jConstructor;

jint JsString::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsString");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  JNINativeMethod methods[] = {
      {"nativeNew", "(Ljava/lang/String;)V", (void *) JsString::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  return JNI_OK;
}

void JsString::New(JNIEnv *env, jobject jThis, jstring jContent) {
  const uint16_t *content = env->GetStringChars(jContent, nullptr);
  const jint contentLen = env->GetStringLength(jContent);
  V8_SCOPE(env, jThis)
  auto result = new v8::Persistent<v8::Value>(isolate, V8_STRING(isolate, content, contentLen));
  env->ReleaseStringChars(jContent, content);
  JsValue::SetReference(env, jThis, (jlong) result);
}
