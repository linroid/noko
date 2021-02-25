//
// Created by linroid on 2/26/21.
//

#ifndef DORA_JS_JNIPROPERTYOBSERVER_H
#define DORA_JS_JNIPROPERTYOBSERVER_H

#include <jni.h>
#include "NodeRuntime.h"

class JniPropertyObserver {
private:
  JavaVM *vm = nullptr;
  jclass clazz;
  jmethodID methodId;
public:
  jobject that;
  NodeRuntime *runtime = nullptr;

  JniPropertyObserver(JNIEnv *env, jobject that, jclass clazz, jmethodID methodId);

  ~JniPropertyObserver();

  void onPropertyChanged(v8::Local<v8::Value>&key, v8::Local<v8::Value>&value);
};


#endif //DORA_JS_JNIPROPERTYOBSERVER_H
