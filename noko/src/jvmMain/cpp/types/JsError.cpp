#include "JsValue.h"
#include "JsString.h"
#include "JsError.h"

jclass JsError::jClazz;
jmethodID JsError::jConstructor;
jclass JsError::jExceptionClazz;
jmethodID JsError::jExceptionConstructor;

jint JsError::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsError");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");

  JNINativeMethod methods[] = {
      {"nativeNew", "(Ljava/lang/String;)V", (void *) JsError::New},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  clazz = env->FindClass("com/linroid/noko/JSException");
  if (!clazz) {
    return JNI_ERR;
  }
  jExceptionClazz = (jclass) env->NewGlobalRef(clazz);
  jExceptionConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JsObject;)V");

  return JNI_OK;
}

void JsError::New(JNIEnv *env, jobject jThis, jstring jMessage) {
  auto messageChars = env->GetStringChars(jMessage, nullptr);
  const jint messageLen = env->GetStringLength(jMessage);

  V8_SCOPE(env, jThis)
  auto message = V8_STRING(isolate, messageChars, messageLen);
  auto value = v8::Exception::Error(message);
  auto result = new v8::Persistent<v8::Value>(node->isolate_, value);
  env->ReleaseStringChars(jMessage, messageChars);
  JsValue::SetPointer(env, jThis, (jlong) result);
}
