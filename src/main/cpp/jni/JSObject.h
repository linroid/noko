//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSOBJECT_H
#define NODE_JSOBJECT_H

#include <jni.h>
#include "JSContext.h"
#include "../util.h"

class JSObject {
private:
    static jclass jclazz;
    static jmethodID jconstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext, (jlong) value);
    }

    static jint OnLoad(JNIEnv *env);

    static JNICALL jobject Get(JNIEnv *env, jobject jthis, jstring jkey);

    static JNICALL void Delete(JNIEnv *env, jobject jthis, jstring jkey);

    static JNICALL void Set(JNIEnv *env, jobject jthis, jstring jkey, jobject);

    static JNICALL jboolean Has(JNIEnv *env, jobject jthis, jstring jkey);

    static JNICALL void New(JNIEnv *env, jobject jthis);
};

#endif //NODE_JSOBJECT_H
