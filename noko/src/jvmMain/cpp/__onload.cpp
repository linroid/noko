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
#include "types/integer.h"
#include "types/double.h"
#include "types/string.h" // NOLINT(modernize-deprecated-headers)
#include "types/boolean.h"
#include "types/long.h"
#include "types/js_array.h"
#include "types/js_function.h"
#include "types/js_error.h"
#include "types/js_promise.h"

#define LOAD_JNI_CLASS(clazz) if (clazz::OnLoad(env) != JNI_OK) { \
    return JNI_ERR; \
}

static jmethodID run_method_id_;
std::unique_ptr<node::MultiIsolatePlatform> platform_;

JNICALL jint Start(JNIEnv *env, jobject j_this, jobjectArray j_args) {
  auto runtime = Runtime::Get(env, j_this);
  jsize argc = env->GetArrayLength(j_args);
  std::vector<std::string> args(static_cast<unsigned long>(argc));

  for (int i = 0; i < argc; ++i) {
    auto string = (jstring) env->GetObjectArrayElement(j_args, i);
    const char *rawString = env->GetStringUTFChars(string, nullptr);
    args[i] = std::string(rawString);
    env->ReleaseStringUTFChars(string, rawString);
  }

  int code = jint(runtime->Start(args));
  delete runtime;
  return code;
}

struct PostMessage {
  jobject runnable;
  Runtime *runtime;
};

JNICALL jboolean Post(JNIEnv *env, jobject j_this, jobject runnable, jboolean force) {
  auto *message = new PostMessage();
  message->runtime = Runtime::Get(env, j_this);
  message->runnable = env->NewGlobalRef(runnable);
  auto success = message->runtime->Post([message] {
    EnvHelper _env(message->runtime->Jvm());
    _env->CallVoidMethod(message->runnable, run_method_id_);
    _env->DeleteGlobalRef(message->runnable);
    delete message;
  }, force);
  if (!success) {
    env->DeleteGlobalRef(message->runnable);
    delete message;
  }
  return 1;
}

JNICALL void MountFile(JNIEnv *env, jobject j_this, jstring j_src, jstring j_dst, jint mode) {
  const char *src = env->GetStringUTFChars(j_src, nullptr);
  const char *dst = env->GetStringUTFChars(j_dst, nullptr);

  auto runtime = Runtime::Get(env, j_this);
  runtime->MountFile(src, dst, mode);

  env->ReleaseStringUTFChars(j_dst, src);
  env->ReleaseStringUTFChars(j_src, dst);
}

JNICALL void Chroot(JNIEnv *env, jobject j_this, jstring j_path) {
  const char *path = env->GetStringUTFChars(j_path, nullptr);

  auto runtime = Runtime::Get(env, j_this);
  runtime->Chroot(path);

  env->ReleaseStringUTFChars(j_path, path);
}

JNICALL jlong New(JNIEnv *env, jobject j_this, jboolean keep_alive, jboolean strict) {
  if (!platform_) {
    env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                  "Unable to create Node instance, have you called Node.setup(...)?");
    return 0;
  }
  auto *runtime = new Runtime(env, j_this, platform_.get(), keep_alive, strict);
  return reinterpret_cast<jlong>(runtime);
}

JNICALL void Exit(JNIEnv *env, jobject j_this, jint code) {
  auto runtime = Runtime::Get(env, j_this);
  runtime->Exit(code);
}

JNICALL void Setup(__unused JNIEnv *env, __unused jclass clazz, jint thread_pool_size, jobject connectivity_manager) {
#if defined(__ANDROID__)
  ares_library_init_android(connectivity_manager);
  LOGI("ares_library_android_initialized: %d", ares_library_android_initialized());
#endif

  // Create a v8::Platform instance. `
  // MultiIsolatePlatform::Create()` is a way
  // to create a v8::Platform instance that Node.js can use when creating
  // Worker threads. When no `MultiIsolatePlatform` instance is present,
  // Worker threads are disabled.
  platform_ = node::MultiIsolatePlatform::Create(thread_pool_size, node::GetTracingController());
  v8::V8::InitializePlatform(platform_.get());
  v8::V8::Initialize();

  char cmd[128];
  strcpy(cmd, "node --trace-exit --trace-sigint --trace-sync-io --trace-warnings --title=node");
  int argc = 0;
  char *argv[128];
  char *p2 = strtok(cmd, " ");
  while (p2 && argc < 128 - 1) {
    argv[argc++] = p2;
    p2 = strtok(nullptr, " ");
  }
  argv[argc] = nullptr;

  std::vector<std::string> args = std::vector<std::string>(argv, argv + argc);
  std::vector<std::string> exec_args;
  std::vector<std::string> errors;
  // Parse Node.js CLI options, and print any errors that have occurred while
  // trying to parse them.
  int exit_code = node::InitializeNodeWithArgs(&args, &exec_args, &errors);
  if (exit_code != 0) {
    LOGE("Failed to call node::InitializeNodeWithArgs()");
    for (const std::string &error : errors) {
      LOGE("%s: %s\n", args[0].c_str(), error.c_str());
    }
    abort();
  }
}

JNICALL jobject Eval(JNIEnv *env, jobject j_this, jstring j_code, jstring j_source, jint line) {
  auto node = Runtime::Get(env, j_this);

  jint codeLen = env->GetStringLength(j_code);
  auto code = env->GetStringChars(j_code, nullptr);
  jint sourceLen = env->GetStringLength(j_source);
  auto source = env->GetStringChars(j_source, nullptr);

  auto result = node->Eval(code, codeLen, source, sourceLen, line);
  env->ReleaseStringChars(j_code, code);
  env->ReleaseStringChars(j_source, source);
  return result;
}

JNICALL jobject ParseJson(JNIEnv *env, jobject j_this, jstring j_json) {
  const uint16_t *json = env->GetStringChars(j_json, nullptr);
  const jint jsonLen = env->GetStringLength(j_json);
  auto node = Runtime::Get(env, j_this);
  auto result = node->ParseJson(json, jsonLen);
  env->ReleaseStringChars(j_json, json);
  return result;
}

JNICALL jobject ThrowError(JNIEnv *env, jobject j_this, jstring j_message) {
  const uint16_t *message = env->GetStringChars(j_message, nullptr);
  const jint messageLen = env->GetStringLength(j_message);
  auto node = Runtime::Get(env, j_this);
  auto result = node->ThrowError(message, messageLen);
  env->ReleaseStringChars(j_message, message);
  return result;
}

JNICALL jobject Require(JNIEnv *env, jobject j_this, jstring j_path) {
  jint pathLen = env->GetStringLength(j_path);
  auto path = env->GetStringChars(j_path, nullptr);
  auto node = Runtime::Get(env, j_this);
  auto result = node->Require(path, pathLen);
  env->ReleaseStringChars(j_path, path);
  return result;
}

JNICALL void ClearReference(JNIEnv *env, jobject j_this, jlong j_reference) {
  auto runtime = Runtime::Get(env, j_this);
  auto reference = reinterpret_cast<v8::Persistent<v8::Value> *>(j_reference);

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
      v8::Locker locker(runtime->Isolate());
      v8::HandleScope handle_scope(runtime->Isolate());
      reference->Reset();
    }
    delete reference;
  });
  if (!submitted) {
    LOGW("clear(%p) but couldn't enqueue the request", reference);
    delete reference;
  }
}

static JNINativeMethod methods[] = {
    {"nativeSetup", "(ILjava/lang/Object;)V", (void *) Setup},
    {"nativeNew", "(ZZ)J", (void *) New},
    {"nativeExit", "(I)V", (void *) Exit},
    {"nativeStart", "([Ljava/lang/String;)I", (void *) Start},
    {"nativeMountFile", "(Ljava/lang/String;Ljava/lang/String;I)V", (void *) MountFile},
    {"nativeChroot", "(Ljava/lang/String;)V", (void *) Chroot},
    {"nativePost", "(Ljava/lang/Runnable;Z)Z", (void *) Post},
    {"nativeEval", "(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/Object;",
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

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  jclass runnableClass = env->FindClass("java/lang/Runnable");
  run_method_id_ = env->GetMethodID(runnableClass, "run", "()V");
  LOAD_JNI_CLASS(JsObject)

  LOAD_JNI_CLASS(Runtime)
  LOAD_JNI_CLASS(JsValue)
  LOAD_JNI_CLASS(Integer)
  LOAD_JNI_CLASS(Boolean)
  LOAD_JNI_CLASS(Double)
  LOAD_JNI_CLASS(String)
  LOAD_JNI_CLASS(Long)
  LOAD_JNI_CLASS(JsArray)
  LOAD_JNI_CLASS(JsUndefined)
  LOAD_JNI_CLASS(JsFunction)
  LOAD_JNI_CLASS(JsError)
  LOAD_JNI_CLASS(JsPromise)

  return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
}
