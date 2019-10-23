//
// Created by linroid on 2019-10-21.
//

#include "jni/JSValue.h"
#include "jni/JSUndefined.h"
#include "jni/JSBoolean.h"
#include "jni/JSNumber.h"
#include "jni/JSObject.h"
#include "V8Runtime.h"

jobject V8Runtime::Wrap(JNIEnv *env, v8::Local<v8::Value> &value) {
    if (value->IsUndefined()) {
        return JSUndefined::New(env, this);
    } else if (value->IsBoolean()) {
        return JSBoolean::New(env, this);
    } else if (value->IsNumber()) {
        return JSNumber::New(env, this);
    } else if (value->IsObject()) {
        return JSObject::New(env, this, value);
    }
    return nullptr;
}
