//
// Created by linroid on 2019-10-21.
//

#include "JSBoolean.h"
#include "JSContext.h"

jclass JSBoolean::jClazz;
jmethodID JSBoolean::jConstructor;

jint JSBoolean::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/knode/js/JSBoolean");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;JZ)V");
  return JNI_OK;
}
