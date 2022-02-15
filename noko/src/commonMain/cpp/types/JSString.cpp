#include <string>
#include "JSString.h"
#include "JSValue.h"
#include "JSContext.h"

jclass JSString::jClazz;
jmethodID JSString::jConstructor;

jint JSString::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSString");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");

  JNINativeMethod methods[] = {
    {"nativeNew", "(Ljava/lang/String;)V", (void *) JSString::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  return JNI_OK;
}

void JSString::New(JNIEnv *env, jobject jThis, jstring jContent) {
  const uint16_t *content = env->GetStringChars(jContent, nullptr);
  const jint contentLen = env->GetStringLength(jContent);
  V8_SCOPE(env, jThis)
  auto result = new v8::Persistent<v8::Value>(isolate, V8_STRING(isolate, content, contentLen));
  env->ReleaseStringChars(jContent, content);
  JSValue::SetReference(env, jThis, (jlong) result);
}
