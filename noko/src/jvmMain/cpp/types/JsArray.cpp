#include "JsArray.h"
#include "JsValue.h"
#include "JsError.h"

jclass JsArray::jClazz;
jmethodID JsArray::jConstructor;

jint JsArray::Size(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Array)
  return (jint) that->Length();
}

void JsArray::New(JNIEnv *env, jobject jThis) {
  V8_SCOPE(env, jThis)
  auto value = v8::Array::New(node->isolate_);
  auto result = new v8::Persistent<v8::Value>(node->isolate_, value);
  JsValue::SetPointer(env, jThis, (jlong) result);
}

jboolean JsArray::AddAll(JNIEnv *env, jobject jThis, jobjectArray jElements) {
  auto size = env->GetArrayLength(jElements);
  v8::Persistent<v8::Value> *elements[size];
  for (int i = 0; i < size; ++i) {
    auto jElement = env->GetObjectArrayElement(jElements, i);
    elements[i] = JsValue::Unwrap(env, jElement);
    env->DeleteLocalRef(jElement);
  }
  SETUP(env, jThis, v8::Array)
  v8::TryCatch tryCatch(node->isolate_);
  auto index = that->Length();
  for (int i = 0; i < size; ++i) {
    auto element = elements[i]->Get(isolate);
    if (!that->Set(context, index + i, element).ToChecked()) {
      return false;
    }
    if (tryCatch.HasCaught()) {
      node->Throw(env, tryCatch.Exception());
      return false;
    }
  }
  return true;
}

jint JsArray::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsArray");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeSize",   "()I",                                  (void *) (Size)},
      {"nativeNew",    "()V",                                  (void *) (New)},
      {"nativeAddAll", "([Lcom/linroid/noko/types/JsValue;)Z", (void *) (AddAll)},
      {"nativeGet",    "(I)Lcom/linroid/noko/types/JsValue;",  (void *) (Get)},
      {"nativeAdd",    "(Lcom/linroid/noko/types/JsValue;)Z",  (void *) (Add)},
      // {"nativeAddAllAt", "(I[Lcom/linroid/noko/types/JsValue;)Z",                             (void *) (AddAllAt)},
      // {"nativeAddAt",    "(ILcom/linroid/noko/types/JsValue;)Lcom/linroid/noko/types/JsValue;", (void *) (AddAllAt)},
  };
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

jobject JsArray::Get(JNIEnv *env, jobject jThis, jint jIndex) {
  SETUP(env, jThis, v8::Array)
  v8::TryCatch tryCatch(node->isolate_);
  auto value = that->Get(context, jIndex).ToLocalChecked();
  if (tryCatch.HasCaught()) {
    node->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  return node->ToJava(env, value);
}

jboolean JsArray::Add(JNIEnv *env, jobject jThis, jobject jElement) {
  auto element = JsValue::Unwrap(env, jElement);
  SETUP(env, jThis, v8::TypedArray)
  v8::TryCatch tryCatch(node->isolate_);
  auto success = that->Set(context, that->Length(), element->Get(isolate)).ToChecked();
  if (tryCatch.HasCaught()) {
    node->Throw(env, tryCatch.Exception());
    return 0;
  }
  return static_cast<jboolean>(success);
}
