#ifndef NOKO_JS_PROPERTIESOBSERVER_H
#define NOKO_JS_PROPERTIESOBSERVER_H

#include <jni.h>
#include "../Node.h"

class PropertiesObserver {
private:
  JavaVM *vm_ = nullptr;
  jmethodID methodId_;
  jobject that_;
public:
  Node *node = nullptr;

  PropertiesObserver(Node *node, JNIEnv *env, jobject that, jmethodID methodId);

  ~PropertiesObserver();

  void onPropertyChanged(JNIEnv *env, jstring key, jobject value);
};


#endif //NOKO_JS_PROPERTIESOBSERVER_H
