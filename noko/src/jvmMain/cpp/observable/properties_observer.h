#ifndef NOKO_JS_PROPERTIESOBSERVER_H
#define NOKO_JS_PROPERTIESOBSERVER_H

#include <jni.h>
#include "../node_runtime.h"

class PropertiesObserver {
private:
  JavaVM *vm_ = nullptr;
  jmethodID methodId_;
  jobject that_;
public:
  NodeRuntime *runtime_ = nullptr;

  PropertiesObserver(NodeRuntime *node, JNIEnv *env, jobject that, jmethodID method_id);

  ~PropertiesObserver();

  void onPropertyChanged(JNIEnv *env, jstring key, jobject value);
};


#endif //NOKO_JS_PROPERTIESOBSERVER_H
