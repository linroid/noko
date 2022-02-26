#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../Node.h"

class JsValue {
private:
  static jfieldID jPointer;
  static jmethodID jConstructor;
  static jmethodID jNodePointer;
public:
  static jclass jClazz;

  inline static jlong GetPointer(JNIEnv *env, jobject jObj) {
    return env->GetLongField(jObj, jPointer);
  }

  inline static void SetPointer(JNIEnv *env, jobject jThis, jlong value) {
    env->SetLongField(jThis, jPointer, value);
  }

  inline static jobject Wrap(JNIEnv *env, jobject jNode, v8::Persistent<v8::Value> *value) {
    return env->NewObject(jClazz, jConstructor, jNode, (jlong) value);
  }

  inline static v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject jObj) {
    return reinterpret_cast<v8::Persistent<v8::Value> *>(GetPointer(env, jObj));
  }

  inline static Node *GetNode(JNIEnv *env, jobject jObj) {
    auto ptr = env->CallLongMethod(jObj, jNodePointer);
    if (ptr == 0) return nullptr;
    return reinterpret_cast<Node *>(ptr);
  }

  JNICALL static jstring ToString(JNIEnv *env, jobject jThis);

  JNICALL static jstring TypeOf(JNIEnv *env, jobject jThis);

  JNICALL static jstring ToJson(JNIEnv *env, jobject jThis);

  JNICALL static jdouble ToNumber(JNIEnv *env, jobject jThis);

  JNICALL static void Dispose(JNIEnv *env, jobject jThis);

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSVALUE_H
