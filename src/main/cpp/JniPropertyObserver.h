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
  jmethodID methodId;
public:
  NodeRuntime *runtime = nullptr;
  jobject that;

  JniPropertyObserver(NodeRuntime *runtime, JNIEnv *env, jobject that, jmethodID methodId);

  ~JniPropertyObserver();

  void onPropertyChanged(JNIEnv *env, jstring key, jobject value);
};


#endif //DORA_JS_JNIPROPERTYOBSERVER_H
