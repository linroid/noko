#include "properties_observer.h"
#include "../types/js_value.h"
#include "../util/env_helper.h"

PropertiesObserver::PropertiesObserver(
    NodeRuntime *node,
    JNIEnv *env,
    jobject that,
    jmethodID method_id
) : runtime_(node), that_(env->NewGlobalRef(that)), methodId_(method_id) {
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
