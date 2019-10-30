//
// Created by linroid on 2019-10-21.
//

#include "JSBoolean.h"

JNIClass booleanClass;

jobject JSBoolean::New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value>& value) {
    auto reference = new v8::Persistent<v8::Value>(runtime->isolate, value);
    return env->NewObject(booleanClass.clazz, booleanClass.constructor, runtime->javaContext, reference, value->BooleanValue());
}

jint JSBoolean::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSBoolean");
    if (!clazz) {
        return JNI_ERR;
    }
    booleanClass.clazz = (jclass) env->NewGlobalRef(clazz);
    booleanClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;JZ)V");
    return JNI_OK;
}
