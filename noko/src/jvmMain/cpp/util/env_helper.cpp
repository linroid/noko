#include "env_helper.h"
#include "jni_helper.h"

EnvHelper::EnvHelper(JavaVM *vm) : vm_(vm) {
  stat_ = vm->GetEnv((void **) (&env_), JNI_VERSION_1_6);
  if (stat_ == JNI_EDETACHED) {
#if defined(__ANDROID__)
    vm->AttachCurrentThread(&env_, nullptr);
#else
    vm->AttachCurrentThread((void **)&env_, nullptr);
#endif
  }
  if (env_->ExceptionCheck()) {
    auto error = env_->ExceptionOccurred();
    LOGE("Attach JNI with pending exception: %s", JniHelper::GetStackTrace(env_, error).c_str());
    abort();
  }
}

EnvHelper::~EnvHelper() {
  if (env_->ExceptionCheck()) {
    auto error = env_->ExceptionOccurred();
    LOGE("Detach JNI with pending exception: %s", JniHelper::GetStackTrace(env_, error).c_str());
    abort();
  }
  if (stat_ == JNI_EDETACHED) {
    vm_->DetachCurrentThread();
  }
}
