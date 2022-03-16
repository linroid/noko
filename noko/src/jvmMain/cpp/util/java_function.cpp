#include "java_function.h"
#include "env_helper.h"
#include "jni_helper.h"

JavaFunction::JavaFunction(
    Runtime *runtime,
    JNIEnv *env,
    jobject that,
    jclass clazz,
    jmethodID method_id
) : runtime_(runtime),
    that_(env->NewGlobalRef(that)),
    class_(clazz),
    method_id_(method_id) {
  node::AddEnvironmentCleanupHook(runtime->Isolate(), CleanupHook, this);
}

void JavaFunction::Call(const v8::FunctionCallbackInfo<v8::Value> &info) {
  EnvHelper env(runtime_->Jvm());
  auto parameters = env->NewObjectArray(info.Length(), class_, nullptr);
  for (int i = 0; i < info.Length(); ++i) {
    v8::Local<v8::Value> element = info[i];
    auto obj = JsValue::Of(*env, element);
    env->SetObjectArrayElement(parameters, i, obj);
  }
  auto caller = (v8::Local<v8::Value>) info.This();
  auto j_caller = JsValue::Of(*env, caller);
  auto j_result = env->CallObjectMethod(that_, method_id_, j_caller, parameters);
  env->DeleteLocalRef(j_caller);
  env->DeleteLocalRef(parameters);

  if (env->ExceptionCheck()) {
    // TODO: Throw exception into v8
    info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
    env->Throw(env->ExceptionOccurred());
    if (env.HasAttached()) {
      runtime_->Jvm()->DetachCurrentThread();
    }
    return;
  }

  if (j_result != nullptr) {
    auto result = JsValue::Value(*env, j_result);
    info.GetReturnValue().Set(result);
  }
}

void JavaFunction::CleanupHook(void *arg) {
  auto *self = static_cast<JavaFunction *>(arg);
  delete self;
}

JavaFunction::~JavaFunction() {
  LOGD("JavaFunction()");
  node::RemoveEnvironmentCleanupHook(runtime_->Isolate(), CleanupHook, this);
  EnvHelper env(runtime_->Jvm());
  env->DeleteGlobalRef(that_);
}
