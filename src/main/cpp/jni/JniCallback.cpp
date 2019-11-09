//
// Created by linroid on 2019/11/3.
//

#include "JniCallback.h"

JniCallback::JniCallback(JNIEnv *env, jobject that, jclass clazz, jmethodID methodId)
        : that(env->NewGlobalRef(that)), clazz(clazz), methodId(methodId) {
    env->GetJavaVM(&vm);
}

void JniCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }

    auto parameters = env->NewObjectArray(info.kArgsLength, clazz, nullptr);
    for (int i = 0; i < info.kArgsLength; ++i) {
        v8::Local<v8::Value> element = info[i];
        auto value = new v8::Persistent<v8::Value>(runtime->isolate, element);
        auto type = runtime->GetType(element);
        env->SetObjectArrayElement(parameters, i, runtime->Wrap(env, value, type));
    }
    auto caller = (v8::Local<v8::Value>) info.This();
    auto value = new v8::Persistent<v8::Value>(runtime->isolate, caller);
    auto type = runtime->GetType(caller);
    auto jret = env->CallObjectMethod(that, methodId, runtime->Wrap(env, value, type), parameters);
    if (jret != 0) {
        auto result = JSValue::Unwrap(env, jret);
        info.GetReturnValue().Set(result->Get(runtime->isolate));
    }
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

JniCallback::~JniCallback() {
    JNIEnv *env;
    LOGD("JVMCallback destruct");
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    env->DeleteGlobalRef(that);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}