#include "JsArray.h"
#include "JsValue.h"
#include "JsError.h"

jclass JsArray::class_;
jmethodID JsArray::init_method_id_;

jint JsArray::Size(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Array)
  return (jint) that->Length();
}

void JsArray::New(JNIEnv *env, jobject j_this) {
  V8_SCOPE(env, j_this)
  auto value = v8::Array::New(runtime->isolate_);
  auto result = new v8::Persistent<v8::Value>(runtime->isolate_, value);
  JsValue::SetPointer(env, j_this, (jlong) result);
}

jboolean JsArray::AddAll(JNIEnv *env, jobject j_this, jobjectArray j_elements) {
  auto size = env->GetArrayLength(j_elements);
  v8::Persistent<v8::Value> *elements[size];
  for (int i = 0; i < size; ++i) {
    auto element = env->GetObjectArrayElement(j_elements, i);
    elements[i] = JsValue::Unwrap(env, element);
    env->DeleteLocalRef(element);
  }
  SETUP(env, j_this, v8::Array)
  v8::TryCatch try_catch(runtime->isolate_);
  auto index = that->Length();
  for (int i = 0; i < size; ++i) {
    auto element = elements[i]->Get(isolate);
    if (!that->Set(context, index + i, element).ToChecked()) {
      return false;
    }
    if (try_catch.HasCaught()) {
      runtime->Throw(env, try_catch.Exception());
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
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

jobject JsArray::Get(JNIEnv *env, jobject j_this, jint j_index) {
  SETUP(env, j_this, v8::Array)
  v8::TryCatch try_catch(runtime->isolate_);
  auto value = that->Get(context, j_index).ToLocalChecked();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  return runtime->ToJava(env, value);
}

jboolean JsArray::Add(JNIEnv *env, jobject j_this, jobject j_element) {
  auto element = JsValue::Unwrap(env, j_element);
  SETUP(env, j_this, v8::TypedArray)
  v8::TryCatch try_catch(runtime->isolate_);
  auto success = that->Set(context, that->Length(), element->Get(isolate)).ToChecked();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return 0;
  }
  return static_cast<jboolean>(success);
}
