//
// Created by linroid on 2019/11/3.
//

#include "JniCallback.h"

JniCallback::JniCallback(NodeRuntime *runtime, JNIEnv *env, jobject that, jclass clazz, jmethodID methodId)
  : runtime(runtime), that(env->NewGlobalRef(that)), clazz(clazz), methodId(methodId) {
  env->GetJavaVM(&vm);
}

void JniCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
  ENTER_JNI(runtime->vm_)
    auto parameters = env->NewObjectArray(info.Length(), clazz, nullptr);
    for (int i = 0; i < info.Length(); ++i) {
      v8::Local<v8::Value> element = info[i];
      auto obj = runtime->ToJava(env, element);
      env->SetObjectArrayElement(parameters, i, obj);
    }
    auto caller = (v8::Local<v8::Value>) info.This();
    auto jCaller = runtime->ToJava(env, caller);
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

    if (jRet != nullptr) {
      auto result = JSValue::Unwrap(env, jRet);
      if (result != nullptr) {
        info.GetReturnValue().Set(result->Get(runtime->isolate_));
      }
    }
  EXIT_JNI(runtime->vm_)
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
