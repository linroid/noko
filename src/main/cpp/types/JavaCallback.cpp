//
// Created by linroid on 2019/11/3.
//

#include "JavaCallback.h"
#include "../EnvHelper.h"

JavaCallback::JavaCallback(
  NodeRuntime *runtime,
  JNIEnv *env,
  jobject that,
  jclass clazz,
  jmethodID methodId
) : runtime(runtime), that(env->NewGlobalRef(that)), clazz_(clazz), methodId_(methodId) {
  env->GetJavaVM(&vm_);
}

void JavaCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
  EnvHelper env(runtime->vm_);
  auto parameters = env->NewObjectArray(info.Length(), clazz_, nullptr);
  for (int i = 0; i < info.Length(); ++i) {
    v8::Local<v8::Value> element = info[i];
    auto obj = runtime->ToJava(*env, element);
    env->SetObjectArrayElement(parameters, i, obj);
  }
  auto caller = (v8::Local<v8::Value>) info.This();
  auto jCaller = runtime->ToJava(*env, caller);
  auto jRet = env->CallObjectMethod(that, methodId_, jCaller, parameters);
  env->DeleteLocalRef(jCaller);

  env->DeleteLocalRef(parameters);

  if (env->ExceptionCheck()) {
    // TODO: Throw crash into v8
    info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
    env->Throw(env->ExceptionOccurred());
    if (env.HasAttached()) {
      vm_->DetachCurrentThread();
    }
    return;
  }

  if (jRet != nullptr) {
    auto result = JSValue::Unwrap(*env, jRet);
    if (result != nullptr) {
      info.GetReturnValue().Set(result->Get(runtime->isolate_));
    }
  }
}

JavaCallback::~JavaCallback() {
  LOGD("~JavaCallback()");
  EnvHelper env(vm_);
  env->DeleteGlobalRef(that);
}
