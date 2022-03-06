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

jobject JNICALL RemoveAt(JNIEnv *env, jobject j_this, jint index) {
  SETUP(env, j_this, v8::Array);
  auto splice_func = that->Get(context, V8_UTF_STRING(isolate, "splice"));
  if (splice_func.IsEmpty()) {
    LOGE("Couldn't find 'splice' function for v8::Array");
    return nullptr;
  }
  v8::TryCatch try_catch(isolate);

  auto value = that->Get(context, index);
  auto *argv = new v8::Local<v8::Value>[2];
  argv[0] = v8::Int32::New(isolate, index); // start
  argv[1] = v8::Int32::New(isolate, 1); // end
  auto result = splice_func.ToLocalChecked().As<v8::Function>()->Call(context, that, 2, argv);

  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }

  if (result.IsEmpty()) {
    return nullptr;
  }

  if (value.IsEmpty()) {
    return nullptr;
  }
  return runtime->ToJava(env, value.ToLocalChecked());
}

void JNICALL AddAt(JNIEnv *env, jobject j_this, jint index, jobject j_value) {
  SETUP(env, j_this, v8::Array);
  auto value = JsValue::Value(isolate, env, j_value);
  v8::TryCatch try_catch(isolate);
  auto splice_func = that->Get(context, V8_UTF_STRING(isolate, "splice"));
  if (splice_func.IsEmpty()) {
    LOGE("Couldn't find 'splice' function for v8::Array");
    return;
  }
  auto *argv = new v8::Local<v8::Value>[3];
  argv[0] = v8::Int32::New(isolate, index);
  argv[1] = v8::Int32::New(isolate, 0);
  argv[2] = value;
  auto result = splice_func.ToLocalChecked().As<v8::Function>()->Call(context, that, 3, argv);
  if (try_catch.HasCaught()) {
    runtime->Throw(env, try_catch.Exception());
    return;
  }
  if (result.IsEmpty()) {
    return;
  }
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsArray");
  if (!clazz) {
    return JNI_ERR;
  }

  JNINativeMethod methods[] = {
      {"nativeSize", "()I", (void *) Size},
      {"nativeNew", "()J", (void *) New},
      {"nativeAddAll", "([Ljava/lang/Object;)Z", (void *) AddAll},
      {"nativeGet", "(I)Ljava/lang/Object;", (void *) Get},
      {"nativeAdd", "(Ljava/lang/Object;)Z", (void *) Add},
      {"nativeClear", "()V", (void *) Clear},
      {"nativeRemoveAt", "(I)Ljava/lang/Object;", (void *) RemoveAt},
      {"nativeAddAt", "(ILjava/lang/Object;)V", (void *) AddAt},
  };
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  return JNI_OK;
}

}