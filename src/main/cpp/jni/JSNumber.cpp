//
// Created by linroid on 2019-10-21.
//

#include "JSNumber.h"

JNIClass numberClass;

jobject JSNumber::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Number> &value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(numberClass.clazz, numberClass.constructor, runtime->javaContext, reference);
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
