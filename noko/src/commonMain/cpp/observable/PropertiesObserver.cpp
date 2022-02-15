#include "PropertiesObserver.h"
#include "../types/JSValue.h"
#include "../EnvHelper.h"

PropertiesObserver::PropertiesObserver(NodeRuntime *runtime, JNIEnv *env, jobject that, jmethodID methodId)
  : runtime(runtime), that_(env->NewGlobalRef(that)), methodId_(methodId) {
  env->GetJavaVM(&vm_);
}

void PropertiesObserver::onPropertyChanged(JNIEnv *env, jstring key, jobject value) {
  env->CallVoidMethod(that_, methodId_, key, value);
  if (env->ExceptionCheck()) {
    LOGW("An error occurred when calling Java callback");
    env->Throw(env->ExceptionOccurred());
  }
}

PropertiesObserver::~PropertiesObserver() {
  LOGD("~PropertiesObserver()");
  EnvHelper env(vm_);
  env->DeleteGlobalRef(that_);
}
