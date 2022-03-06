#ifndef NOKO_TYPES_INTEGER_H_
#define NOKO_TYPES_INTEGER_H_

#include <jni.h>
#include "v8.h"

namespace Integer {

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value);

jint Value(JNIEnv *env, jobject obj);

bool Is(JNIEnv *env, jobject obj);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_TYPES_INTEGER_H_
