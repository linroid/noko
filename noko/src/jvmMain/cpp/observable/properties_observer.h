#ifndef NOKO_JS_PROPERTIESOBSERVER_H
#define NOKO_JS_PROPERTIESOBSERVER_H

#include <jni.h>
#include "../runtime.h"

class PropertiesObserver {
private:
  JavaVM *vm_ = nullptr;
  jmethodID methodId_;
  jobject that_;
public:
  Runtime *runtime_ = nullptr;

  PropertiesObserver(Runtime *node, JNIEnv *env, jobject that, jmethodID method_id);

  ~PropertiesObserver();

  void onPropertyChanged(JNIEnv *env, jstring key, jobject value);
};


#endif //NOKO_JS_PROPERTIESOBSERVER_H
