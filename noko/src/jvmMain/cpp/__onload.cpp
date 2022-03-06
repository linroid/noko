#include <__bit_reference>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <cmath>
#include <ares.h>
#include "util/env_helper.h"
#include "runtime.h"
#include "types/js_value.h"
#include "types/js_object.h"
#include "types/js_undefined.h"
#include "types/double.h"
#include "types/string.h"
#include "types/boolean.h"
#include "types/js_array.h"
#include "types/js_null.h"
#include "types/js_function.h"
#include "types/js_error.h"
#include "types/js_promise.h"

#define LOAD_JNI_CLASS(clazz) if (clazz::OnLoad(env) != JNI_OK) { \
    return JNI_ERR; \
}

static jmethodID run_method_id_;

JNICALL jint Start(JNIEnv *env, jobject jThis, jobjectArray jArgs) {
  auto runtime = Runtime::Get(env, jThis);
  jsize argc = env->GetArrayLength(jArgs);
  std::vector<std::string> args(static_cast<unsigned long>(argc));

  for (int i = 0; i < argc; ++i) {
    auto string = (jstring) env->GetObjectArrayElement(jArgs, i);
    const char *rawString = env->GetStringUTFChars(string, nullptr);
    args[i] = std::string(rawString);
    env->ReleaseStringUTFChars(string, rawString);
  }

  int code = jint(runtime->Start(args));
  LOGI("free runtime=%p", runtime);
  delete runtime;
  return code;
}

struct PostMessage {
  jobject runnable;
  Runtime *runtime;
};

JNICALL jboolean Post(JNIEnv *env, jobject jThis, jobject jRunnable) {
  auto *message = new PostMessage();
  message->runtime = Runtime::Get(env, jThis);
  message->runnable = env->NewGlobalRef(jRunnable);
  auto success = message->runtime->Post([message] {
    EnvHelper _env(message->runtime->vm_);
    _env->CallVoidMethod(message->runnable, run_method_id_);
    _env->DeleteGlobalRef(message->runnable);
    delete message;
  });
  if (!success) {
    env->DeleteGlobalRef(message->runnable);
    delete message;
  }
  return 1;
}

JNICALL void MountFile(JNIEnv *env, jobject jThis, jstring jSrc, jstring jDst, jint mode) {
  const char *src = env->GetStringUTFChars(jSrc, nullptr);
  const char *dst = env->GetStringUTFChars(jDst, nullptr);

  auto runtime = Runtime::Get(env, jThis);
  runtime->MountFile(src, dst, mode);

  env->ReleaseStringUTFChars(jDst, src);
  env->ReleaseStringUTFChars(jSrc, dst);
}

JNICALL void Chroot(JNIEnv *env, jobject jThis, jstring jPath) {
  const char *path = env->GetStringUTFChars(jPath, nullptr);

  auto runtime = Runtime::Get(env, jThis);
  runtime->Chroot(path);

  env->ReleaseStringUTFChars(jPath, path);
}

JNICALL jlong New(JNIEnv *env, jobject jThis, jboolean keepAlive, jboolean strict) {
  auto *runtime = new Runtime(env, jThis, keepAlive, strict);
  return reinterpret_cast<jlong>(runtime);
}

JNICALL void Exit(JNIEnv *env, jobject jThis, jint exitCode) {
  auto runtime = Runtime::Get(env, jThis);
  runtime->Exit(exitCode);
}

JNICALL void Setup(__unused JNIEnv *env, __unused jclass jThis, jobject connectivity_manager) {
#if defined(__ANDROID__)
  ares_library_init_android(connectivity_manager);
  LOGI("ares_library_android_initialized: %d", ares_library_android_initialized());
#endif
  // int max = 5;
  // size_t num = 0;
  // ares_get_server_list_from_env(max, &num);
}

JNICALL jobject Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine) {
  auto node = Runtime::Get(env, jThis);

  jint codeLen = env->GetStringLength(jCode);
  auto code = env->GetStringChars(jCode, nullptr);
  jint sourceLen = env->GetStringLength(jSource);
  auto source = env->GetStringChars(jSource, nullptr);

  auto result = node->Eval(code, codeLen, source, sourceLen, jLine);
  env->ReleaseStringChars(jCode, code);
  env->ReleaseStringChars(jSource, source);
  return result;
}

JNICALL jobject ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
  const uint16_t *json = env->GetStringChars(jJson, nullptr);
  const jint jsonLen = env->GetStringLength(jJson);
  auto node = Runtime::Get(env, jThis);
  auto result = node->ParseJson(json, jsonLen);
  env->ReleaseStringChars(jJson, json);
  return result;
}

JNICALL jobject ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
  const uint16_t *message = env->GetStringChars(jMessage, nullptr);
  const jint messageLen = env->GetStringLength(jMessage);
  auto node = Runtime::Get(env, jThis);
  auto result = node->ThrowError(message, messageLen);
  env->ReleaseStringChars(jMessage, message);
  return result;
}

// TODO: Not working, illegal context
JNICALL jobject Require(JNIEnv *env, jobject jThis, jstring jPath) {
  jint pathLen = env->GetStringLength(jPath);
  auto path = env->GetStringChars(jPath, nullptr);
  auto node = Runtime::Get(env, jThis);
  auto result = node->Require(path, pathLen);
  env->ReleaseStringChars(jPath, path);
  return result;
}

JNICALL void ClearReference(JNIEnv *env, jobject j_this, jlong j_reference) {
  auto runtime = JsValue::GetRuntime(env, j_this);
  auto reference = reinterpret_cast<v8::Persistent <v8::Value> *>(j_reference);

  LOGV("clear reference: reference=%p, runtime=%p", reference, runtime);
  if (runtime == nullptr) {
    delete reference;
    return;
  }

  if (!runtime->IsRunning()) {
    LOGW("runtime is not running, free %p", reference);
    delete reference;
    return;
  }

  bool submitted = runtime->Post([reference, runtime] {
    if (runtime->IsRunning()) {
      v8::Locker locker(runtime->isolate_);
      v8::HandleScope handle_scope(runtime->isolate_);
      reference->Reset();
    }
    delete reference;
  });
  if (!submitted) {
    LOGW("clear(%p) but couldn't enqueue the request", reference);
    delete reference;
  }
}

static JNINativeMethod nodeMethods[] = {
#if defined(__ANDROID__)
    {"nativeSetup", "(Landroid/net/ConnectivityManager;)V", (void *) Setup},
#endif
    {"nativeNew", "(ZZ)J", (void *) New},
    {"nativeExit", "(I)V", (void *) Exit},
    {"nativeStart", "([Ljava/lang/String;)I", (void *) Start},
    {"nativeMountFile", "(Ljava/lang/String;Ljava/lang/String;I)V", (void *) MountFile},
    {"nativeChroot", "(Ljava/lang/String;)V", (void *) Chroot},
    {"nativePost", "(Ljava/lang/Runnable;)Z", (void *) Post},
    {"nativeEval", "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/noko/types/JsValue;",
     (void *) Eval},
    {"nativeParseJson", "(Ljava/lang/String;)Lcom/linroid/noko/types/JsValue;", (void *) ParseJson},
    {"nativeThrowError", "(Ljava/lang/String;)Lcom/linroid/noko/types/JsError;",
     (void *) ThrowError},
    {"nativeRequire", "(Ljava/lang/String;)Lcom/linroid/noko/types/JsObject;", (void *) Require},
    {"nativeClearReference", "(J)V", (void *) ClearReference},
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
  JNIEnv *env = nullptr;
#if defined(__ANDROID__)
  ares_library_init_jvm(vm);
#endif
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }
  jclass clazz = env->FindClass("com/linroid/noko/Node");
  if (clazz == nullptr) {
    return JNI_ERR;
  }

  int rc = env->RegisterNatives(clazz, nodeMethods, sizeof(nodeMethods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  jclass runnableClass = env->FindClass("java/lang/Runnable");
  run_method_id_ = env->GetMethodID(runnableClass, "run", "()V");

  LOAD_JNI_CLASS(Runtime)
  LOAD_JNI_CLASS(JsValue)
  LOAD_JNI_CLASS(JsObject)
  LOAD_JNI_CLASS(Boolean)
  LOAD_JNI_CLASS(Double)
  LOAD_JNI_CLASS(String)
  LOAD_JNI_CLASS(JsArray)
  LOAD_JNI_CLASS(JsUndefined)
  LOAD_JNI_CLASS(JsNull)
  LOAD_JNI_CLASS(JsFunction)
  LOAD_JNI_CLASS(JsError)
  LOAD_JNI_CLASS(JsPromise)

  return JNI_VERSION_1_6;
}
