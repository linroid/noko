//
// Created by linroid on 2/26/21.
//

#include "JniPropertyObserver.h"
#include "jni/JSValue.h"

JniPropertyObserver::JniPropertyObserver(NodeRuntime *runtime, JNIEnv *env, jobject that, jmethodID methodId)
  : runtime(runtime), that(env->NewGlobalRef(that)), methodId(methodId) {
  env->GetJavaVM(&vm);
}

void JniPropertyObserver::onPropertyChanged(JNIEnv *env, jstring key, jobject value) {
  env->CallVoidMethod(that, methodId, key, value);
}

JniPropertyObserver::~JniPropertyObserver() {
  JNIEnv *env;
  LOGD("JniPropertyObserver destruct");
  auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
  if (stat == JNI_EDETACHED) {
    vm->AttachCurrentThread(&env, nullptr);
  }
  env->DeleteGlobalRef(that);
  if (stat == JNI_EDETACHED) {
    vm->DetachCurrentThread();
  }
}
