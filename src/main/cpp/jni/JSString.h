//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"

class JSString {
public:

    static jstring Empty(JNIEnv *env);

    static v8::Local<v8::String> ToV8(JNIEnv *env, v8::Isolate *isolate, jstring &string);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
