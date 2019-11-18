//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSValue {
private:
    static jfieldID jreference;
    static jfieldID jcontext;
    static jmethodID jconstructor;
    static jmethodID jruntime;
public:
    static jclass jclazz;

    inline static jlong GetReference(JNIEnv *env, jobject jobj) {
        return env->GetLongField(jobj, jreference);
    }

    inline static void SetReference(JNIEnv *env, jobject jobj, jlong value) {
        env->SetLongField(jobj, jreference, value);
    }

    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext, (jlong) value);
    }

    inline static v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject jobj) {
        return reinterpret_cast<v8::Persistent<v8::Value> *>(GetReference(env, jobj));
    }

    inline static NodeRuntime *GetRuntime(JNIEnv *env, jobject jobj) {
        auto ptr = env->CallLongMethod(jobj, jruntime);
        return reinterpret_cast<NodeRuntime *>(ptr);
    }

    JNICALL static jstring ToString(JNIEnv *env, jobject jthis);

    JNICALL static jstring TypeOf(JNIEnv *env, jobject jthis);

    JNICALL static jstring ToJson(JNIEnv *env, jobject jthis);

    JNICALL static jdouble ToNumber(JNIEnv *env, jobject jthis);

    JNICALL static void Dispose(JNIEnv *env, jobject jthis);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSVALUE_H
