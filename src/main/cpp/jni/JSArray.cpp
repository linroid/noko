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

jobject JSArray::Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
    return env->NewObject(arrayClass.clazz, arrayClass.constructor, runtime->jcontext, (jlong) value);
}

jint JSArray::OnLoad(JNIEnv *env) {
    jclass clazz = env->FindClass("com/linroid/knode/js/JSArray");
    if (!clazz) {
        return JNI_ERR;
    }

    JNINativeMethod methods[] = {
            {"nativeSize",     "()I",                                                             (void *) (Size)},
            {"nativeNew",      "()V",                                                             (void *) (New)},
            {"nativeAddAll",   "([Lcom/linroid/knode/js/JSValue;)Z",                              (void *) (AddAll)},
            {"nativeAddAllAt", "(I[Lcom/linroid/knode/js/JSValue;)Z",                             (void *) (AddAllAt)},
            {"nativeGet",      "(I)Lcom/linroid/knode/js/JSValue;",                               (void *) (Get)},
            {"nativeAdd",      "(Lcom/linroid/knode/js/JSValue;)Z",                               (void *) (Add)},
            {"nativeAddAt",    "(ILcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;", (void *) (AddAllAt)},
    };
    arrayClass.clazz = (jclass) env->NewGlobalRef(clazz);
    arrayClass.constructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
    return JNI_OK;
}

jobject JSArray::Get(JNIEnv *env, jobject jthis, jint jindex) {
    v8::Persistent<v8::Value> *error = nullptr;
    v8::Persistent<v8::Value> *result = nullptr;
    JSType type = None;
    V8_CONTEXT(env, jthis, v8::Array)
        v8::TryCatch tryCatch(runtime->isolate);
        auto value = that->Get(jindex);
        if (tryCatch.HasCaught()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
        type = runtime->GetType(value);
        result = new v8::Persistent<v8::Value>(isolate, value);
    V8_END();
    if (error) {
        JSError::Throw(env, runtime, error);
        return nullptr;
    }
    return runtime->Wrap(env, result, type);
}

jboolean JSArray::Add(JNIEnv *env, jobject jthis, jobject jelement) {
    v8::Persistent<v8::Value> *error = nullptr;
    auto element = JSValue::Unwrap(env, jelement);
    bool success = false;
    V8_CONTEXT(env, jthis, v8::TypedArray)
        v8::TryCatch tryCatch(runtime->isolate);
        success = that->Set(that->Length(), element->Get(isolate));
        if (tryCatch.HasCaught()) {
            error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
            return;
        }
    V8_END();
    if (error) {
        JSError::Throw(env, runtime, error);
    }
    return static_cast<jboolean>(success);
}

jboolean JSArray::AddAt(JNIEnv *env, jobject jthis, jint jindex, jobject jelement) {

}

jboolean JSArray::AddAllAt(JNIEnv *env, jobject jthis, jint jindex, jobjectArray jelements) {
    return 0;
}
