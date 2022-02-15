#include "JSNull.h"
#include "JSValue.h"

jclass JSNull::jClazz;
jmethodID JSNull::jConstructor;

jint JSNull::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSNull");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");

  return JNI_OK;
}

void JSNull::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Null(isolate);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  JSValue::SetReference(env, jThis, (jlong) result);
}
