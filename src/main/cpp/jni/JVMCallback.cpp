//
// Created by linroid on 2019/11/3.
//

#include "JVMCallback.h"

JVMCallback::JVMCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId)
        : runtime(runtime), that(env->NewGlobalRef(that)), clazz(clazz), methodId(methodId) {
    env->GetJavaVM(&vm);
}

void JVMCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }

    auto parameters = env->NewObjectArray(info.kArgsLength, clazz, nullptr);
    for (int i = 0; i < info.kArgsLength; ++i) {
        v8::Local<v8::Value> element = info[i];
        env->SetObjectArrayElement(parameters, i, runtime->Wrap(env, element));
    }
    auto caller = (v8::Local<v8::Value>) info.This();
    auto jret = env->CallObjectMethod(that, methodId, runtime->Wrap(env, caller), parameters);
    if (jret != 0) {
        auto ret = JSValue::GetReference(env, runtime->isolate, jret);
        info.GetReturnValue().Set(ret);
    }
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

JVMCallback::~JVMCallback() {
    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    env->DeleteGlobalRef(that);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}