#include "EnvHelper.h"

EnvHelper::EnvHelper(JavaVM *vm) : vm_(vm) {
  stat_ = vm->GetEnv((void **) (&env_), JNI_VERSION_1_6);
  if (stat_ == JNI_EDETACHED) {
    vm->AttachCurrentThread(&env_, nullptr);
  }
  if (env_->ExceptionCheck()) {
    LOGE("Attach JNI with pending Exception, %s: %d", __FILE__, __LINE__);
    env_->Throw(env_->ExceptionOccurred());
  }
}

EnvHelper::~EnvHelper() {
  if (env_->ExceptionCheck()) {
    LOGE("Detach JNI with pending Exception, %s:%d", __FILE__, __LINE__);
    env_->Throw(env_->ExceptionOccurred());
  }
  if (stat_ == JNI_EDETACHED) {
    vm_->DetachCurrentThread();
  }
}
