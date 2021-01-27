//
// Created by linroid on 2019-10-23.
//

#include "JSArray.h"
#include "JSValue.h"
#include "JSContext.h"
#include "JSError.h"

jclass JSArray::jClazz;
jmethodID JSArray::jConstructor;

jint JSArray::Size(JNIEnv *env, jobject jThis) {
  int result = 0;
  SETUP(env, jThis, v8::Array)
  result = that->Length();
  return result;
}


void JSArray::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Array::New(runtime->isolate_);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  JSValue::SetReference(env, jThis, (jlong) result);
}

jboolean JSArray::AddAll(JNIEnv *env, jobject jThis, jobjectArray jElements) {
  auto size = env->GetArrayLength(jElements);
  bool result = true;
  v8::Persistent<v8::Value> *elements[size];
  v8::Persistent<v8::Value> *error = nullptr;
  for (int i = 0; i < size; ++i) {
    auto jElement = env->GetObjectArrayElement(jElements, i);
    elements[i] = JSValue::Unwrap(env, jElement);
    env->DeleteLocalRef(jElement);
  }
  SETUP(env, jThis, v8::Array)
  v8::TryCatch tryCatch(runtime->isolate_);
  auto index = that->Length();
  for (int i = 0; i < size; ++i) {
    auto element = elements[i]->Get(isolate);
    if (!that->Set(context, index + i, element).ToChecked()) {
      result = false;
      break;
    }
    if (tryCatch.HasCaught()) {
      error = new v8::Persistent<v8::Value>(isolate, tryCatch.Exception());
      break;
    }
  }
  if (error) {
    JSError::Throw(env, runtime, error);
    return 0;
  }
  return static_cast<jboolean>(result);
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
    {"nativeGet",    "(I)Lcom/linroid/knode/js/JSValue;",  (void *) (Get)},
    {"nativeAdd",    "(Lcom/linroid/knode/js/JSValue;)Z",  (void *) (Add)},
    // {"nativeAddAllAt", "(I[Lcom/linroid/knode/js/JSValue;)Z",                             (void *) (AddAllAt)},
    // {"nativeAddAt",    "(ILcom/linroid/knode/js/JSValue;)Lcom/linroid/knode/js/JSValue;", (void *) (AddAllAt)},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/knode/js/JSContext;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

jobject JSArray::Get(JNIEnv *env, jobject jThis, jint jIndex) {
  SETUP(env, jThis, v8::Array)
  v8::TryCatch tryCatch(runtime->isolate_);
  auto value = that->Get(context, jIndex).ToLocalChecked();
  if (tryCatch.HasCaught()) {
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  return runtime->ToJava(env, value);
}

jboolean JSArray::Add(JNIEnv *env, jobject jThis, jobject jElement) {
  v8::Persistent<v8::Value> *error = nullptr;
  auto element = JSValue::Unwrap(env, jElement);
  bool success = false;
  SETUP(env, jThis, v8::TypedArray)
  v8::TryCatch tryCatch(runtime->isolate_);
  success = that->Set(context, that->Length(), element->Get(isolate)).ToChecked();
  if (tryCatch.HasCaught()) {
    runtime->Throw(env, tryCatch.Exception());
    return 0;
  }
  return static_cast<jboolean>(success);
}
