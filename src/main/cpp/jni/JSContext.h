//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JS_CONTEXT_H
#define NODE_JS_CONTEXT_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSContext {
private:
    static jclass jclazz;
    static jmethodID jconstructor;
    static jfieldID jnullId;
    static jfieldID jUndefinedId;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime) {
        return env->NewObject(jclazz, jconstructor, (jlong) runtime, (jlong) runtime->global);
    }

    static JNICALL jobject ParseJson(JNIEnv *env, jstring jthis, jstring jjson);

    static JNICALL jobject Eval(JNIEnv *env, jstring jthis, jstring jcode, jstring jsource, jint jline);

    static jint OnLoad(JNIEnv *env);

    static void SetShared(JNIEnv *env, NodeRuntime *runtime);
};


#endif //NODE_JS_CONTEXT_H
