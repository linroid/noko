#include "JsBoolean.h"

jclass JsBoolean::jClazz;
jmethodID JsBoolean::jConstructor;

jint JsBoolean::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsBoolean");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;JZ)V");
  return JNI_OK;
}
