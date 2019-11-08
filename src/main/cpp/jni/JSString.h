//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"
#include "../NodeRuntime.h"
#include "JSValue.h"

class JSString {
public:
    JNICALL static void New(JNIEnv *env, jobject jthis, jstring content);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value);

    static jstring From(JNIEnv *env, v8::Local<v8::String> &value);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
