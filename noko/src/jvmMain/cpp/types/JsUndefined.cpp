#include "JsUndefined.h"

jclass JsUndefined::jClazz;
jmethodID JsUndefined::jConstructor;

jint JsUndefined::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsUndefined");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  return JNI_OK;
}
