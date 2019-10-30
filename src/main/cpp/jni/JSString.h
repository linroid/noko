//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"
#include "../NodeRuntime.h"

class JSString {
public:
    static jobject New(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    static jstring Empty(JNIEnv *env);

    static v8::Local<v8::String> ToV8(JNIEnv *env, v8::Isolate *isolate, jstring &string);

    JNICALL static void NativeInit(JNIEnv *env, jobject thiz, jstring content);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
