//
// Created by linroid on 2019/11/5.
//

#ifndef DORA_JSERROR_H
#define DORA_JSERROR_H

#include <jni.h>
#include <v8.h>

class JSError {
private:
    static jclass jClazz;
    static jmethodID jConstructor;
    static jclass jExceptionClazz;
    static jmethodID jExceptionConstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        LOGE("Wrap new JSError");
        return env->NewObject(jClazz, jConstructor, runtime->jContext, (jlong) value);
    }

    JNICALL static void New(JNIEnv *env, jobject jThis, jstring jmessage);

    static void Throw(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *error);

    static jint OnLoad(JNIEnv *env);
};


#endif //DORA_JSERROR_H
