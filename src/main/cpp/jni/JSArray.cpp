//
// Created by linroid on 2019-10-23.
//

#include "JSArray.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSError.h"

static JNIClass arrayClass;

jint JSArray::Size(JNIEnv *env, jobject jthis) {
    int result = 0;
    V8_CONTEXT(env, jthis, v8::Array)
        result = that->Length();
    V8_END();
    return result;
}


void JSArray::New(JNIEnv *env, jobject jthis) {
    v8::Persistent<v8::Value> *result;
    V8_SCOPE(env, jthis)
        auto value = v8::Array::New(runtime->isolate);
        result = new v8::Persistent<v8::Value>(runtime->isolate, value);
    V8_END()
    JSValue::SetReference(env, jthis, (jlong) result);
}

jboolean JSArray::AddAll(JNIEnv *env, jobject jthis, jobjectArray jelements) {
    auto size = env->GetArrayLength(jelements);
    bool result = true;
    v8::Persistent<v8::Value> *elements[size];
    v8::Persistent<v8::Value> *error = nullptr;
    for (int i = 0; i < size; ++i) {
        auto jelement = env->GetObjectArrayElement(jelements, i);
        elements[i] = JSValue::Unwrap(env, jelement);
    }
    V8_CONTEXT(env, jthis, v8::Array)
        v8::TryCatch tryCatch(runtime->isolate);
        auto index = that->Length();
        for (int i = 0; i < size; ++i) {
            auto element = elements[i]->Get(isolate);
            if (!that->Set(index + i, element)) {
                result = false;
                break;
            }
            if (tryCatch.HasCaught()) {
                error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
                break;
            }
        }
    V8_END();
    if (error) {
        JSError::Throw(env, runtime, error);
        return 0;
    }
    return static_cast<jboolean>(result);
}


jobject JSArray::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Array> &value) {
    auto reference = new v8::Persistent<v8::Array>(runtime->isolate, value);
    return env->NewObject(arrayClass.clazz, arrayClass.constructor, runtime->jcontext, (jlong) reference);
}

jint JSArray::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSArray");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeSize",   "()I",                                (void *) (Size)},
            {"nativeNew",    "()V",                                (void *) (New)},
            {"nativeAddAll", "([Lcom/linroid/knode/js/JSValue;)Z", (void *) (AddAll)},
    };
    arrayClass.clazz = (jclass) env->NewGlobalRef(clazz);
    arrayClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}
