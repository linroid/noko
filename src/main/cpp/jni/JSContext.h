//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JS_CONTEXT_H
#define NODE_JS_CONTEXT_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSContext {
private:
    static jclass jClazz;
    static jmethodID jConstructor;
    static jfieldID jNullId;
    static jfieldID jUndefinedId;
    static jfieldID jTrueId;
    static jfieldID jFalseId;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime) {
        return env->NewObject(jClazz, jConstructor, (jlong) runtime, (jlong) runtime->global);
    }

    static JNICALL jobject ParseJson(JNIEnv *env, jstring jThis, jstring jJson);

    static JNICALL jobject Eval(JNIEnv *env, jstring jThis, jstring jCode, jstring jSource, jint jLine);

    static JNICALL jobject ThrowError(JNIEnv *env, jstring jThis, jstring jMessage);

    static jint OnLoad(JNIEnv *env);

    static void SetShared(JNIEnv *env, NodeRuntime *runtime);
};


#endif //NODE_JS_CONTEXT_H
