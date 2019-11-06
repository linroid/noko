//
// Created by linroid on 2019/11/5.
//

#ifndef DORA_JSERROR_H
#define DORA_JSERROR_H

#include <jni.h>
#include <v8.h>

class JSError {
public:
    JNICALL static void New(JNIEnv *env, jobject jthis, jstring jmessage);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    static void Throw(JNIEnv *env, NodeRuntime *runtime, v8::TryCatch &tryCatch);

    static void Throw(JNIEnv *env, jobject jerror);

    static jint OnLoad(JNIEnv *env);
};


#endif //DORA_JSERROR_H
