#ifndef DORA_JS_PROPERTIESOBSERVER_H
#define DORA_JS_PROPERTIESOBSERVER_H

#include <jni.h>
#include "../Noko.h"

class PropertiesObserver {
private:
  JavaVM *vm_ = nullptr;
  jmethodID methodId_;
  jobject that_;
public:
  Noko *runtime = nullptr;

  PropertiesObserver(Noko *runtime, JNIEnv *env, jobject that, jmethodID methodId);

  ~PropertiesObserver();

  void onPropertyChanged(JNIEnv *env, jstring key, jobject value);
};


#endif //DORA_JS_PROPERTIESOBSERVER_H
