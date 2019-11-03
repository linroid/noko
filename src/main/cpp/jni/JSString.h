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
    JNICALL static void New(JNIEnv *env, jobject thiz, jstring content);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::String> &value);

    /**
     * Convert Java String to v8 String
     */
    static v8::Local<v8::String> From(JNIEnv *env, v8::Isolate *isolate, jstring &string);

    static jstring Value(JNIEnv *env, v8::Local<v8::String> &value);
    
    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
