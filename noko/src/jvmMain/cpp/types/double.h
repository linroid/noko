#ifndef NOKO_TYPES_DOUBLE_H_
#define NOKO_TYPES_DOUBLE_H_

#include <jni.h>
#include <v8.h>
#include "../util/macros.h"

namespace Double {

jdouble Value(JNIEnv *env, jobject obj);

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value);

bool Is(JNIEnv *env, jobject obj);

jint OnLoad(JNIEnv *env);

}

#endif //NOKO_TYPES_DOUBLE_H_
