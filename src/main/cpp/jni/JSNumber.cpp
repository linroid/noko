//
// Created by linroid on 2019-10-21.
//

#include "JSNumber.h"

JNIClass numberClass;

jobject JSNumber::New(JNIEnv *env, NodeRuntime *runtime) {
    return env->NewObject(numberClass.clazz, numberClass.constructor, reinterpret_cast<jlong>(runtime));
}

jint JSNumber::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSNumber");
    if (!clazz) {
        return JNI_ERR;
    }
    numberClass.clazz = (jclass) env->NewGlobalRef(clazz);
    numberClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    return JNI_OK;
}
