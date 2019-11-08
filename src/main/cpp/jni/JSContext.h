//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JS_CONTEXT_H
#define NODE_JS_CONTEXT_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSContext {
public:
    static NodeRuntime *GetRuntime(JNIEnv *env, jobject jobj);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime);

    static JNICALL jobject ParseJson(JNIEnv *env, jstring jthis, jstring jjson);

    static JNICALL jobject Eval(JNIEnv *env, jstring jthis, jstring jcode, jstring jsource, jint jline);

    static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JS_CONTEXT_H
