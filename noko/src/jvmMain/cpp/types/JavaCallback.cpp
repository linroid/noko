#include "JavaCallback.h"
#include "../EnvHelper.h"

JavaCallback::JavaCallback(
    Noko *noko,
    JNIEnv *env,
    jobject that,
    jclass clazz,
    jmethodID methodId
) : noko(noko), that(env->NewGlobalRef(that)), clazz_(clazz), methodId_(methodId) {
  env->GetJavaVM(&vm_);
}

void JavaCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
  EnvHelper env(noko->vm_);
  auto parameters = env->NewObjectArray(info.Length(), clazz_, nullptr);
  for (int i = 0; i < info.Length(); ++i) {
    v8::Local<v8::Value> element = info[i];
    auto obj = noko->ToJava(*env, element);
    env->SetObjectArrayElement(parameters, i, obj);
  }
  auto caller = (v8::Local<v8::Value>) info.This();
  auto jCaller = noko->ToJava(*env, caller);
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
      info.GetReturnValue().Set(result->Get(noko->isolate_));
    }
  }
}

JavaCallback::~JavaCallback() {
  LOGD("~JavaCallback()");
  EnvHelper env(vm_);
  env->DeleteGlobalRef(that);
}