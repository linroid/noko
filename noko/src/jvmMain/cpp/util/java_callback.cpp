#include "java_callback.h"
#include "env_helper.h"
#include "jni_helper.h"

JavaCallback::JavaCallback(
    Runtime *runtime,
    JNIEnv *env,
    jobject that,
    jclass clazz,
    jmethodID method_id
) : runtime_(runtime),
    that_(env->NewGlobalRef(that)),
    class_(clazz),
    method_id_(method_id) {
}

void JavaCallback::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
  EnvHelper env(runtime_->vm_);
  auto parameters = env->NewObjectArray(info.Length(), class_, nullptr);
  for (int i = 0; i < info.Length(); ++i) {
    v8::Local<v8::Value> element = info[i];
    auto obj = runtime_->ToJava(*env, element);
    env->SetObjectArrayElement(parameters, i, obj);
  }
  auto caller = (v8::Local<v8::Value>) info.This();
  auto j_caller = runtime_->ToJava(*env, caller);
  auto j_result = env->CallObjectMethod(that_, method_id_, j_caller, parameters);
  env->DeleteLocalRef(j_caller);
  env->DeleteLocalRef(parameters);

  if (env->ExceptionCheck()) {
    // TODO: Throw exception into v8
    info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
    env->Throw(env->ExceptionOccurred());
    if (env.HasAttached()) {
      runtime_->vm_->DetachCurrentThread();
    }
    return;
  }

  if (j_result != nullptr) {
    auto result = JsValue::Unwrap(*env, j_result);
    if (result != nullptr) {
      info.GetReturnValue().Set(result->Get(runtime_->isolate_));
    }
  }
}

JavaCallback::~JavaCallback() {
  LOGD("~JavaCallback()");
  EnvHelper env(runtime_->vm_);
  env->DeleteGlobalRef(that_);
}
