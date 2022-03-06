#include "js_array.h"
#include "js_value.h"
#include "js_error.h"

namespace JsArray {

jclass class_;
jmethodID init_method_id_;

jobject Of(JNIEnv *env, jobject node, jlong pointer) {
  return env->NewObject(class_, init_method_id_, node, pointer);
}

jint Size(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Array);
  return (jint) that->Length();
}

jlong New(JNIEnv *env, jclass clazz) {
  V8_SCOPE(env);
  UNUSED(clazz);
  auto value = v8::Array::New(isolate);
  return (jlong) new v8::Persistent<v8::Value>(isolate, value);
}

jboolean AddAll(JNIEnv *env, jobject j_this, jobjectArray j_elements) {
  SETUP(env, j_this, v8::Array);
  auto size = env->GetArrayLength(j_elements);
  v8::TryCatch try_catch(isolate);
  auto index = that->Length();
  for (int i = 0; i < size; ++i) {
    auto j_element = env->GetObjectArrayElement(j_elements, i);
    v8::Local<v8::Value> element = JsValue::Value(isolate, env, j_element);
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

jobject Get(JNIEnv *env, jobject j_this, jint j_index) {
  SETUP(env, j_this, v8::Array);
  v8::TryCatch try_catch(isolate);
  auto value = that->Get(context, j_index).ToLocalChecked();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  return runtime->ToJava(env, value);
}

jboolean Add(JNIEnv *env, jobject j_this, jobject j_element) {
  SETUP(env, j_this, v8::Array);
  auto element = JsValue::Value(isolate, env, j_element);
  v8::TryCatch try_catch(isolate);
  auto success = that->Set(context, that->Length(), element).ToChecked();
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return 0;
  }
  return static_cast<jboolean>(success);
}

void JNICALL Clear(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Array);
  // auto size = that->Length();
  // while (size-- > 0) {
  //   that->Delete(context, size - 1).Check();
  // }
  that->Set(context, V8_UTF_STRING(isolate, "length"), v8::Number::New(isolate, 0)).Check();
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsArray");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeSize", "()I", (void *) (Size)},
      {"nativeNew", "()J", (void *) (New)},
      {"nativeAddAll", "([Ljava/lang/Object;)Z", (void *) (AddAll)},
      {"nativeGet", "(I)Ljava/lang/Object;", (void *) (Get)},
      {"nativeAdd", "(Ljava/lang/Object;)Z", (void *) (Add)},
      {"nativeClear", "()V", (void *) (Clear)},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}