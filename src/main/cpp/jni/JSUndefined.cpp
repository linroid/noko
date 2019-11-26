//
// Created by linroid on 2019-10-20.
//

#include "JSUndefined.h"
#include "JSContext.h"


jclass JSUndefined::jClazz;
jmethodID JSUndefined::jConstructor;

jint JSUndefined::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSUndefined");
    if (clazz == nullptr) {
        return JNI_ERR;
    }
    jClazz = (jclass) env->NewGlobalRef(clazz);
    jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    return JNI_OK;
}
