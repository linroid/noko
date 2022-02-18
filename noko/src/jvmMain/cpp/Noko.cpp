#include <jni.h>
#include <mutex>
#include <uv.h>
#include <v8.h>
#include "Noko.h"
#include "types/JSValue.h"
#include "types/JSUndefined.h"
#include "types/JSBoolean.h"
#include "types/JSNumber.h"
#include "types/JSObject.h"
#include "types/JSString.h"
#include "types/JSFunction.h"
#include "types/JSNull.h"
#include "types/JSPromise.h"
#include "types/JSArray.h"
#include "types/JSError.h"
#include "EnvHelper.h"

int Noko::instanceCount_ = 0;
std::mutex Noko::sharedMutex_;
int Noko::seq_ = 0;

bool nodeInitialized = false;
std::unique_ptr<node::MultiIsolatePlatform> platform;

Noko::Noko(
    JNIEnv *env,
    jobject jThis,
    jmethodID jAttach,
    jmethodID jDetach,
    bool keepAlive,
    bool strict
) : jAttach_(jAttach),
    jDetach_(jDetach),
    keepAlive_(keepAlive),
    strict_(strict) {

  env->GetJavaVM(&vm_);
  jThis_ = env->NewGlobalRef(jThis);
  std::lock_guard<std::mutex> lock(sharedMutex_);
  ++instanceCount_;
  ++seq_;
  id_ = seq_;
  if (!nodeInitialized) {
    init_node();
  }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
  LOGD("new Noko: thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
}

Noko::~Noko() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
  LOGE("~Noko(): thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
  std::lock_guard<std::mutex> sharedLock(sharedMutex_);
  std::lock_guard<std::mutex> asyncLock(asyncMutex_);
  {
    EnvHelper env(vm_);
    env->DeleteGlobalRef(jSharedUndefined_);
    env->DeleteGlobalRef(jSharedNull_);
    env->DeleteGlobalRef(jSharedTrue_);
    env->DeleteGlobalRef(jSharedFalse_);
    env->SetLongField(jThis_, jPtrId, 0l);
    env->DeleteGlobalRef(jThis_);
  }

  isolate_ = nullptr;
  running_ = false;
  LOGI("set running=false");

  --instanceCount_;
  LOGE("~Noko() finished, instanceCount=%d", instanceCount_);
}

void Noko::Attach() {
  LOGI("Attach: %p", this);
  v8::Local<v8::Context> context = context_.Get(isolate_);
  v8::HandleScope handleScope(isolate_);
  v8::Context::Scope contextScope(context);
  running_ = true;
  EnvHelper env(vm_);
  auto nullValue = new v8::Persistent<v8::Value>(isolate_, v8::Null(isolate_));
  auto undefinedValue = new v8::Persistent<v8::Value>(isolate_, v8::Undefined(isolate_));
  auto trueValue = new v8::Persistent<v8::Value>(isolate_, v8::True(isolate_));
  auto falseValue = new v8::Persistent<v8::Value>(isolate_, v8::False(isolate_));
  // this->jGlobal_ = env->NewGlobalRef(JSObject::Wrap(*env, jThis_, this));
  this->jSharedNull_ = env->NewGlobalRef(JSNull::Wrap(*env, jThis_, nullValue));
  this->jSharedUndefined_ = env->NewGlobalRef(JSUndefined::Wrap(*env, jThis_, undefinedValue));
  this->jSharedTrue_ = env->NewGlobalRef(JSBoolean::Wrap(*env, jThis_, trueValue, true));
  this->jSharedFalse_ = env->NewGlobalRef(JSBoolean::Wrap(*env, jThis_, falseValue, false));

  env->SetObjectField(jThis_, jSharedNullId, jSharedNull_);
  env->SetObjectField(jThis_, jSharedUndefinedId, jSharedUndefined_);
  env->SetObjectField(jThis_, jSharedTrueId, jSharedTrue_);
  env->SetObjectField(jThis_, jSharedFalseId, jSharedFalse_);

  env->CallVoidMethod(jThis_, jAttach_, (jlong) &global_);
}

void Noko::Detach() {
  EnvHelper env(vm_);
  env->CallVoidMethod(jThis_, jDetach_, jThis_);
}

int Noko::Start(std::vector<std::string> &args) {
  threadId_ = std::this_thread::get_id();

  int exitCode = 0;
  if (!InitLoop()) {
    LOGE("Failed to call InitLoop()");
    return -1;
  }

  std::shared_ptr<node::ArrayBufferAllocator> allocator = node::ArrayBufferAllocator::Create();
  isolate_ = node::NewIsolate(allocator, eventLoop_, platform.get());

  if (isolate_ == nullptr) {
    LOGE("Failed to call node::NewIsolate()");
    return -2;
  }

  {
    v8::Locker locker(isolate_);
    v8::Isolate::Scope isolateScope(isolate_);

    auto isolateData = node::CreateIsolateData(isolate_, eventLoop_, platform.get(), allocator.get());
    v8::HandleScope handleScope(isolate_);
    v8::Local<v8::Context> context = node::NewContext(isolate_);
    if (context.IsEmpty()) {
      LOGE("Failed to call node::NewContext()");
      return -3;
    }

    v8::Context::Scope contextScope(context);
    auto flags = static_cast<node::EnvironmentFlags::Flags>(node::EnvironmentFlags::kOwnsEmbedded
                                                            | node::EnvironmentFlags::kOwnsProcessState
                                                            | node::EnvironmentFlags::kTrackUnmanagedFds);
    auto env = node::CreateEnvironment(isolateData, context, args, args, flags);
    node::SetProcessExitHandler(env, [](node::Environment *environment, int code) {
      LOGW("Node.js process is exiting: code=%d", code);
      node::Stop(environment);
    });

    context_.Reset(isolate_, context);
    global_.Reset(isolate_, context->Global());

    Attach();

    isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
    node::LoadEnvironment(env, [&](const node::StartExecutionCallbackInfo &info) -> v8::MaybeLocal<v8::Value> {
      require_.Reset(isolate_, info.native_require);
      process_.Reset(isolate_, info.process_object);
      return v8::Null(isolate_);
    });

    RunLoop(env);
    Detach();
    // node::EmitExit() returns the current exit code.
    auto exitCodeMaybe = node::EmitProcessExit(env);
    if (exitCodeMaybe.IsJust()) {
      exitCode = exitCodeMaybe.ToChecked();
    }
    // node::Stop() can be used to explicitly stop the event loop and keep
    // further JavaScript from running. It can be called from any thread,
    // and will act like worker.terminate() if called from another thread.
    node::Stop(env);

    node::FreeIsolateData(isolateData);
    node::FreeEnvironment(env);

    context_.Reset();
    global_.Reset();
    running_ = false;
  }

  CloseLoop();
  return exitCode;
}


void Noko::CloseLoop() {
  LOGW("closing uv loop");
  // Unregister the Isolate with the platform and add a listener that is called
  // when the Platform is done cleaning up any state it had associated with
  // the Isolate.
  bool platformFinished = false;
  platform->AddIsolateFinishedCallback(isolate_, [](void *data) {
    *static_cast<bool *>(data) = true;
  }, &platformFinished);
  platform->UnregisterIsolate(isolate_);
  isolate_->Dispose();
  // Wait until the platform has cleaned up all relevant resources.
  while (!platformFinished) {
    uv_run(eventLoop_, UV_RUN_ONCE);
  }
  int uvErrorCode = uv_loop_close(eventLoop_);
  eventLoop_ = nullptr;
  LOGI("close loop result: %d, callbacks.size=%lu", uvErrorCode, callbacks_.size());
  // uv_loop_delete(loop);
  // assert(err == 0);
}

void Noko::RunLoop(node::Environment *env) {
  v8::SealHandleScope seal(isolate_);

  int more;
  do {
    uv_run(eventLoop_, UV_RUN_DEFAULT);

    // V8 tasks on background threads may end up scheduling new tasks in the
    // foreground, which in turn can keep the event loop going. For example,
    // WebAssembly.compile() may do so.
    platform->DrainTasks(isolate_);

    // If there are new tasks, continue.
    more = uv_loop_alive(eventLoop_);
    if (more) continue;
    LOGW("no more task, try to exit");
    // node::EmitBeforeExit() is used to emit the 'beforeExit' event on
    // the `process` object.
    node::EmitProcessBeforeExit(env);

    // 'beforeExit' can also schedule new work that keeps the event loop
    // running.
    more = uv_loop_alive(eventLoop_);
  } while (more);
}


void Noko::Exit(int code) {
  LOGI("Exit(%d)", code);
  if (keepAlive_) {
    uv_async_send(keepAliveHandle_);
  }
  Post([&] {
    v8::HandleScope handle_scope(isolate_);
    auto process = process_.Get(isolate_);
    auto context = context_.Get(isolate_);
    v8::Local<v8::Value> v8Code = v8::Number::New(isolate_, code);
    auto exitFunc = process->Get(context, V8_UTF_STRING(isolate_, "exit"));
    LOGI("Calling process.exit(%d)", code);
    UNUSED(v8::Local<v8::Function>::Cast(exitFunc.ToLocalChecked())->Call(context, process, 1, &v8Code));
  });
}

bool Noko::InitLoop() {
  eventLoop_ = uv_loop_new();
  int ret = uv_loop_init(eventLoop_);
  if (ret != 0) {
    LOGE("Failed to initialize loop: %s", uv_err_name(ret));
    return false;
  }
  if (keepAlive_) {
    keepAliveHandle_ = new uv_async_t();
    ret = uv_async_init(eventLoop_, keepAliveHandle_, [](uv_async_t *handle) {
      LOGD("Stop keep alive");
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
      uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
        free(h);
      });
#pragma clang diagnostic pop
    });
    if (ret != 0) {
      LOGE("Failed to initialize keep alive handle: %s", uv_err_name(ret));
      return false;
    }
  }
  return true;
}

jobject Noko::ToJava(JNIEnv *env, v8::Local<v8::Value> value) {
  if (value->IsNull()) {
    return this->jSharedNull_;
  } else if (value->IsUndefined()) {
    return this->jSharedUndefined_;
  } else if (value->IsBoolean()) {
    v8::Local<v8::Boolean> target = value->ToBoolean(isolate_);
    if (target->Value()) {
      return this->jSharedTrue_;
    } else {
      return this->jSharedFalse_;
    }
  } else {
    auto reference = new v8::Persistent<v8::Value>(isolate_, value);
    if (value->IsNumber()) {
      return JSNumber::Wrap(env, jThis_, reference);
    } else if (value->IsObject()) {
      if (value->IsFunction()) {
        return JSFunction::Wrap(env, jThis_, reference);
      } else if (value->IsPromise()) {
        return JSPromise::Wrap(env, jThis_, reference);
      } else if (value->IsNativeError()) {
        return JSError::Wrap(env, jThis_, reference);
      } else if (value->IsArray()) {
        return JSArray::Wrap(env, jThis_, reference);
      }
      return JSObject::Wrap(env, jThis_, reference);
    } else if (value->IsString()) {
      return JSString::Wrap(env, jThis_, reference);
    }
    return JSValue::Wrap(env, jThis_, reference);
  }
}

void Noko::TryLoop() {
  if (!eventLoop_) {
    LOGE("TryLoop but eventLoop is null");
    return;
  }
  if (!callbackHandle_) {
    callbackHandle_ = new uv_async_t();
    callbackHandle_->data = this;
    uv_async_init(eventLoop_, callbackHandle_, [](uv_async_t *handle) {
      auto *runtime = reinterpret_cast<Noko *>(handle->data);
      runtime->Handle(handle);
    });
    uv_async_send(callbackHandle_);
  }
}

bool Noko::Await(const std::function<void()> &runnable) {
  assert(isolate_ != nullptr);
  if (std::this_thread::get_id() == threadId_) {
    runnable();
    return true;
  } else if (strict_) {
    EnvHelper env(vm_);
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "Illegal thread access");
    return true;
  } else {
    std::condition_variable cv;
    bool signaled = false;
    auto callback = [&]() {
      runnable();
      {
        std::lock_guard<std::mutex> lock(asyncMutex_);
        signaled = true;
      }
      cv.notify_one();
    };

    std::unique_lock<std::mutex> lock(asyncMutex_);
    if (!running_) {
      LOGE("Instance has been destroyed, ignore await");
      return false;
    }
    callbacks_.emplace_back(callback);
    this->TryLoop();

    cv.wait(lock, [&] { return signaled; });
    lock.unlock();
    return true;
  }
}

bool Noko::Post(const std::function<void()> &runnable) {
  if (!running_) {
    return false;
  }
  std::lock_guard<std::mutex> lock(asyncMutex_);
  if (!running_) {
    return false;
  }
  if (std::this_thread::get_id() == threadId_) {
    runnable();
    return true;
  } else {
    callbacks_.push_back(runnable);
    this->TryLoop();
    return true;
  }
}

void Noko::Handle(uv_async_t *handle) {
  asyncMutex_.lock();
  while (!callbacks_.empty()) {
    auto callback = callbacks_.front();
    asyncMutex_.unlock();
    callback();
    asyncMutex_.lock();
    callbacks_.erase(callbacks_.begin());
  }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
  uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
    delete (uv_async_t *) h;
  });
#pragma clang diagnostic pop
  callbackHandle_ = nullptr;
  asyncMutex_.unlock();
}

// v8::Local<v8::Value> Noko::Require(const char *path) {
//     v8::EscapableHandleScope handleScope(isolate_);
//     auto context = context_.Get(isolate_);
//     auto global = global_->Get(isolate_);
//     v8::Context::Scope contextScope(context);
//     auto key = v8::String::NewFromUtf8(isolate_, "require").ToLocalChecked();
//     v8::Local<v8::Object> require = global->Get(context, key).ToLocalChecked()->ToObject(context).ToLocalChecked();
//     v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[1];
//     argv[0] = V8_UTF_STRING(isolate_, path);
//     assert(require->IsFunction());
//     auto result = require->CallAsFunction(context, global, 1, argv).ToLocalChecked();
//     return handleScope.Escape(result->ToObject(context).ToLocalChecked());
// }

v8::Local<v8::Value> Noko::Require(const char *path) {
  CheckThread();
  v8::EscapableHandleScope scope(isolate_);
  auto context = context_.Get(isolate_);
  auto require = require_.Get(isolate_);
  auto global = global_.Get(isolate_);
  v8::Context::Scope contextScope(context);

  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_UTF_STRING(isolate_, path);
  assert(require->IsFunction());
  auto result = require->Call(context, global, 1, argv).ToLocalChecked();
  return scope.Escape(result);
}

void Noko::MountFile(const char *src, const char *dst, const int mode) {
  CheckThread();
  v8::Locker _locker(isolate_);
  v8::HandleScope _handleScope(isolate_);
  auto context = context_.Get(isolate_);
  auto env = node::GetCurrentEnvironment(context);
  node::Mount(env, src, dst, mode);
}

void Noko::Chroot(const char *path) {
  CheckThread();
  v8::Locker _locker(isolate_);
  v8::HandleScope _handleScope(isolate_);
  auto context = context_.Get(isolate_);
  auto env = node::GetCurrentEnvironment(context);
  node::Chroot(env, path);
}

void Noko::Throw(JNIEnv *env, v8::Local<v8::Value> exception) {
  CheckThread();
  auto reference = new v8::Persistent<v8::Value>(isolate_, exception);
  auto jException = JSError::ToException(env, JSError::Wrap(env, jThis_, reference));
  env->Throw((jthrowable) jException);
}

void Noko::CheckThread() {
  if (std::this_thread::get_id() != threadId_) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    LOGE("js object can only be accessed from the Node.js thread: current=%ld, threadId=%ld",
         std::this_thread::get_id(), threadId_);
#pragma clang diagnostic pop
    std::abort();
  }
}

jobject Noko::Eval(const uint16_t *code, int codeLen, const uint16_t *source, int sourceLen, int line) {
  v8::TryCatch tryCatch(isolate_);
  auto context = context_.Get(isolate_);
  v8::ScriptOrigin scriptOrigin(V8_STRING(isolate_, source, sourceLen), v8::Integer::New(isolate_, line),
                                v8::Integer::New(isolate_, 0));
  auto script = v8::Script::Compile(context, V8_STRING(isolate_, code, codeLen), &scriptOrigin);
  EnvHelper env(vm_);
  if (script.IsEmpty()) {
    LOGE("Compile script with an exception");
    Throw(*env, tryCatch.Exception());
    return nullptr;
  }
  auto result = script.ToLocalChecked()->Run(context);
  if (result.IsEmpty()) {
    LOGE("Run script with an exception");
    Throw(*env, tryCatch.Exception());
    return nullptr;
  }
  return ToJava(*env, result.ToLocalChecked());
}

jobject Noko::ParseJson(const uint16_t *json, int jsonLen) {
  auto context = context_.Get(isolate_);
  v8::Context::Scope contextScope(context);
  v8::TryCatch tryCatch(isolate_);
  auto result = v8::JSON::Parse(context, V8_STRING(isolate_, json, jsonLen));
  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, tryCatch.Exception());
    return nullptr;
  }
  return ToJava(*env, result.ToLocalChecked());
}

jobject Noko::ThrowError(const uint16_t *message, int messageLen) {
  auto error = v8::Exception::Error(V8_STRING(isolate_, message, messageLen));
  isolate_->ThrowException(error);
  EnvHelper env(vm_);

  return ToJava(*env, error);
}

jobject Noko::Require(const uint16_t *path, int pathLen) {
  auto context = context_.Get(isolate_);
  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_STRING(isolate_, path, pathLen);
  auto global = global_.Get(isolate_);
  v8::TryCatch tryCatch(isolate_);

  auto require = global->Get(context, V8_UTF_STRING(isolate_, "require")).ToLocalChecked()->ToObject(
      context).ToLocalChecked();
  assert(require->IsFunction());

  auto result = require->CallAsFunction(context, global, 1, argv);

  EnvHelper env(vm_);
  if (result.IsEmpty()) {
    Throw(*env, tryCatch.Exception());
    return nullptr;
  }
  return ToJava(*env, result.ToLocalChecked());
}


int init_node() {
  // Make argv memory adjacent
  char cmd[128];
  strcpy(cmd, "node --trace-exit --trace-sigint --trace-sync-io --trace-warnings --title=Dora.js");
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
  for (const std::string &error: errors)
    LOGE("%s: %s\n", args[0].c_str(), error.c_str());
  if (exit_code == 0) {
    nodeInitialized = true;
  }

  // Create a v8::Platform instance. `MultiIsolatePlatform::Create()` is a way
  // to create a v8::Platform instance that Node.js can use when creating
  // Worker threads. When no `MultiIsolatePlatform` instance is present,
  // Worker threads are disabled.
  platform = node::MultiIsolatePlatform::Create(4);
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  return exit_code;
}

// void shutdownNode() {
//     nodeInitialized = false;
//     v8::V8::Dispose();
//     v8::V8::ShutdownPlatform();
// }

jfieldID  Noko::jSharedNullId;
jfieldID  Noko::jSharedUndefinedId;
jfieldID  Noko::jSharedTrueId;
jfieldID  Noko::jSharedFalseId;
jfieldID  Noko::jPtrId;

jint Noko::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/Noko");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  jSharedNullId = env->GetFieldID(clazz, "sharedNull", "Lcom/linroid/noko/types/JSNull;");
  jSharedUndefinedId = env->GetFieldID(clazz, "sharedUndefined", "Lcom/linroid/noko/types/JSUndefined;");
  jSharedTrueId = env->GetFieldID(clazz, "sharedTrue", "Lcom/linroid/noko/types/JSBoolean;");
  jSharedFalseId = env->GetFieldID(clazz, "sharedFalse", "Lcom/linroid/noko/types/JSBoolean;");
  jPtrId = env->GetFieldID(clazz, "ptr", "J");

  return JNI_OK;
}
