#ifndef NOKO_PROMISE_H
#define NOKO_PROMISE_H

#include <jni.h>

namespace JsPromise {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_PROMISE_H
