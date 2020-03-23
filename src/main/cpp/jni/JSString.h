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
private:
    static jclass jClazz;
    static jmethodID jConstructor;

public:
    JNICALL static void New(JNIEnv *env, jobject jThis, jstring content);

    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jClazz,
                              jConstructor,
                              runtime->jContext_,
                              (jlong) value);
    }

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSSTRING_H
