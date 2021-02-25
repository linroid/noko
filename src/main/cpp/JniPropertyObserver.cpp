//
// Created by linroid on 2/26/21.
//

#include "JniPropertyObserver.h"
#include "jni/JSValue.h"

JniPropertyObserver::JniPropertyObserver(JNIEnv *env, jobject that, jclass clazz, jmethodID methodId)
  : that(env->NewGlobalRef(that)), clazz(clazz), methodId(methodId) {
  env->GetJavaVM(&vm);
}

void JniPropertyObserver::onPropertyChanged(v8::Local<v8::Value> &key, v8::Local<v8::Value> &value) {
  ENTER_JNI(runtime->vm_)
    env->CallObjectMethod(that, methodId, runtime->ToJava(env, key), runtime->ToJava(env, value));
  EXIT_JNI(runtime->vm_)
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
