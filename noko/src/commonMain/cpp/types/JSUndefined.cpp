#include "JSUndefined.h"

jclass JSUndefined::jClazz;
jmethodID JSUndefined::jConstructor;

jint JSUndefined::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSUndefined");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");
  return JNI_OK;
}