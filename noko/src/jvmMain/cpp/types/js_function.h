#ifndef NOKO_JSFUNCTION_H
#define NOKO_JSFUNCTION_H

#include <jni.h>
#include <v8.h>
#include "../runtime.h"

namespace JsFunction {

  jobject Of(JNIEnv *env, jobject node, jlong pointer);

  bool Is(JNIEnv *env, jobject obj);

  v8::Local<v8::Function> Value(JNIEnv *env, jobject obj);

  jint OnLoad(JNIEnv *env);

  class JavaCallback {
  private:
    v8::Persistent<v8::Function> value_;
    jobject obj_;

    ~JavaCallback();

    void Call(const v8::FunctionCallbackInfo<v8::Value> &info) const;

    static void StaticCallback(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void WeakCallback(const v8::WeakCallbackInfo<JavaCallback> &data);

    static void CleanupHook(void *arg);

  public:
    JavaCallback(JNIEnv *env, jobject obj);

    v8::Local<v8::Function> Get(v8::Isolate *isolate);
  };
}


#endif //NOKO_JSFUNCTION_H
