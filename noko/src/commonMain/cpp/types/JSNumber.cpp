#include "JSNumber.h"
#include "JSValue.h"

jclass JSNumber::jClazz;
jmethodID JSNumber::jConstructor;

jint JSNumber::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSNumber");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");
  JNINativeMethod methods[] = {
      {"nativeNew", "(D)V", (void *) JSNumber::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

void JSNumber::New(JNIEnv *env, jobject jThis, jdouble jData) {
  V8_SCOPE(env, jThis)
  auto value = v8::Number::New(noko->isolate_, jData);
  auto result = new v8::Persistent<v8::Value>(noko->isolate_, value);
  JSValue::SetReference(env, jThis, (jlong) result);
}
