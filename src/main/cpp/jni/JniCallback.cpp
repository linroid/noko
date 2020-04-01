//
// Created by linroid on 2019/11/3.
//

#include "JniCallback.h"

JniCallback::JniCallback(JNIEnv *env, jobject that, jclass clazz, jmethodID methodId)
        : that(env->NewGlobalRef(that)), clazz(clazz), methodId(methodId) {
    env->GetJavaVM(&vm);
}

void JniCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
    ENTER_JNI(runtime->vm_);
        auto parameters = env->NewObjectArray(info.kArgsLength, clazz, nullptr);
        for (int i = 0; i < info.kArgsLength; ++i) {
            v8::Local<v8::Value> element = info[i];
            auto value = new v8::Persistent<v8::Value>(runtime->isolate_, element);
            auto type = runtime->GetType(element);
            auto obj = runtime->Wrap(env, value, type);
            env->SetObjectArrayElement(parameters, i, obj);
            if (type >= kValue) {
                env->DeleteLocalRef(obj);
            }
        }
        auto caller = (v8::Local<v8::Value>) info.This();
        auto value = new v8::Persistent<v8::Value>(runtime->isolate_, caller);
        auto type = runtime->GetType(caller);
        auto jCaller = runtime->Wrap(env, value, type);
        auto jRet = env->CallObjectMethod(that, methodId, jCaller, parameters);
        env->DeleteLocalRef(jCaller);

        env->DeleteLocalRef(parameters);

        if (env->ExceptionCheck()) {
            info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
            env->Throw(env->ExceptionOccurred());
            if (stat == JNI_EDETACHED) {
                vm->DetachCurrentThread();
            }
            return;
        }

        if (jRet != 0) {
            auto result = JSValue::Unwrap(env, jRet);
            if (result != nullptr) {
                info.GetReturnValue().Set(result->Get(runtime->isolate_));
            }
        }
    EXIT_JNI(runtime->vm_);
}

JniCallback::~JniCallback() {
    JNIEnv *env;
    LOGD("JniCallback destruct");
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    env->DeleteGlobalRef(that);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}
