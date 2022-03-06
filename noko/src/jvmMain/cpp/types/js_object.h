#ifndef NODE_JSOBJECT_H
#define NODE_JSOBJECT_H

#include <jni.h>

namespace JsObject {

   jobject Of(JNIEnv *env, jobject node, jlong pointer);

   jint OnLoad(JNIEnv *env);
}

#endif //NODE_JSOBJECT_H
