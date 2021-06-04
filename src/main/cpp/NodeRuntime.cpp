//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include <mutex>
#include <uv.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include "NodeRuntime.h"
#include "jni/JSContext.h"
#include "jni/JSValue.h"
#include "jni/JSUndefined.h"
#include "jni/JSBoolean.h"
#include "jni/JSNumber.h"
#include "jni/JSObject.h"
#include "jni/JSString.h"
#include "jni/JSFunction.h"
#include "jni/JSNull.h"
#include "jni/JSPromise.h"
#include "jni/JSArray.h"
#include "jni/JSError.h"

int NodeRuntime::instanceCount_ = 0;
std::mutex NodeRuntime::sharedMutex_;
int NodeRuntime::seq_ = 0;

bool nodeInitialized = false;
std::unique_ptr<node::MultiIsolatePlatform> platform;

NodeRuntime::NodeRuntime(
  JNIEnv *env,
  jobject jThis,
  jmethodID onBeforeStart,
  jmethodID onBeforeExit,
  bool keepAlive,
  bool strict
) : onBeforeStart_(onBeforeStart),
    onBeforeExit_(onBeforeExit),
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
  LOGD("new NodeRuntime: thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
}

NodeRuntime::~NodeRuntime() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
  LOGE("~NodeRuntime(): thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id_, this);
#pragma clang diagnostic pop
  std::lock_guard<std::mutex> sharedLock(sharedMutex_);
  std::lock_guard<std::mutex> asyncLock(asyncMutex_);
  ENTER_JNI(vm_)
    env->DeleteGlobalRef(jThis_);
    env->DeleteGlobalRef(jUndefined_);
    env->DeleteGlobalRef(jNull_);
    env->DeleteGlobalRef(jTrue_);
    env->DeleteGlobalRef(jFalse_);
    JSContext::ClearPtr(env, jContext_);
    env->DeleteGlobalRef(jContext_);
  EXIT_JNI(vm_)

  isolate_ = nullptr;
  running_ = false;
  LOGI("set running=false");

  --instanceCount_;
  LOGE("~NodeRuntime() finished, instanceCount=%d", instanceCount_);
}

void NodeRuntime::OnPrepared() {
  LOGI("OnPrepared: %p", this);
  v8::Local<v8::Context> context = context_.Get(isolate_);
  v8::HandleScope handleScope(isolate_);
  v8::Context::Scope contextScope(context);
  running_ = true;
  ENTER_JNI(vm_)
    auto nullValue = new v8::Persistent<v8::Value>(isolate_, v8::Null(isolate_));
    auto undefinedValue = new v8::Persistent<v8::Value>(isolate_, v8::Undefined(isolate_));
    auto trueValue = new v8::Persistent<v8::Value>(isolate_, v8::True(isolate_));
    auto falseValue = new v8::Persistent<v8::Value>(isolate_, v8::False(isolate_));
    this->jContext_ = env->NewGlobalRef(JSContext::Wrap(env, this));
    this->jNull_ = env->NewGlobalRef(JSNull::Wrap(env, this, nullValue));
    this->jUndefined_ = env->NewGlobalRef(JSUndefined::Wrap(env, this, undefinedValue));
    this->jTrue_ = env->NewGlobalRef(JSBoolean::Wrap(env, this, trueValue, true));
    this->jFalse_ = env->NewGlobalRef(JSBoolean::Wrap(env, this, falseValue, false));
    JSContext::SetShared(env, this);

    env->CallVoidMethod(jThis_, onBeforeStart_, jContext_);
  EXIT_JNI(vm_)
}

void NodeRuntime::OnBeforeExit() {
  ENTER_JNI(vm_)
    env->CallVoidMethod(jThis_, onBeforeExit_, jContext_);
  EXIT_JNI(vm_)
}

int NodeRuntime::Start(std::vector<std::string> &args) {
  threadId_ = std::this_thread::get_id();

  // See below for the contents of this function.
  int exitCode = 0;
  if (!InitLoop()) {
    return 1;
  }

  std::shared_ptr<node::ArrayBufferAllocator> allocator = node::ArrayBufferAllocator::Create();
  isolate_ = node::NewIsolate(allocator, eventLoop_, platform.get());

  if (isolate_ == nullptr) {
    LOGE("Failed to initialize V8 isolate: %s", args[0].c_str());
    return 2;
  }

  {
    v8::Locker locker(isolate_);
    v8::Isolate::Scope isolateScope(isolate_);

    auto isolateData = node::CreateIsolateData(isolate_, eventLoop_, platform.get(), allocator.get());
    v8::HandleScope handleScope(isolate_);
    v8::Local<v8::Context> context = node::NewContext(isolate_);
    if (context.IsEmpty()) {
      LOGE("%s: Failed to initialize V8 Context", args[0].c_str());
      return 3;
    }

    v8::Context::Scope contextScope(context);
    auto flags = static_cast<node::EnvironmentFlags::Flags>(node::EnvironmentFlags::kOwnsProcessState |
                                                            node::EnvironmentFlags::kOwnsEmbedded);
    LOGD("CreateEnvironment: flags=%lud", flags);
    auto env = node::CreateEnvironment(isolateData, context, args, args, flags);
    node::SetProcessExitHandler(env, [](node::Environment *environment, int code) {
      LOGW("exiting node process: %d", code);
      node::Stop(environment);
    });

    context_.Reset(isolate_, context);
    global_.Reset(isolate_, context->Global());

    OnPrepared();

    // Set up the Node.js instance for execution, and run code inside of it.
    // There is also a variant that takes a callback and provides it with
    // the `require` and `process` objects, so that it can manually compile
    // and run scripts as needed.
    // The `require` function inside this script does *not* access the file
    // system, and can only load built-in Node.js modules.
    // `module.createRequire()` is being used to create one that is able to
    // load files from the disk, and uses the standard CommonJS file loader
    // instead of the internal-only `require` function.
    // node::LoadEnvironment(
    //         env.get(),
    //         "const publicRequire ="
    //         "  require('module').createRequire(process.cwd() + '/');"
    //         "globalThis.require = publicRequire;"
    //         "require('vm').runInThisContext(process.argv[1]);");

    isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);
    node::LoadEnvironment(env, [&](const node::StartExecutionCallbackInfo &info) -> v8::MaybeLocal<v8::Value> {
      require_.Reset(isolate_, info.native_require);
      process_.Reset(isolate_, info.process_object);
      return v8::Null(isolate_);
    });

    RunLoop(env);
    OnBeforeExit();
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


void NodeRuntime::CloseLoop() {
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

void NodeRuntime::RunLoop(node::Environment *env) {
  if (keepAlive_) {
    isolate_->SetPromiseHook([](v8::PromiseHookType type,
                                v8::Local<v8::Promise> promise,
                                v8::Local<v8::Value> parent) {
      if (type == v8::PromiseHookType::kInit) {
        promise->GetIsolate()->RunMicrotasks();
      }
    });
  }

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
    LOGW("no more task, try exit");
    // node::EmitBeforeExit() is used to emit the 'beforeExit' event on
    // the `process` object.
    node::EmitProcessBeforeExit(env);

    // 'beforeExit' can also schedule new work that keeps the event loop
    // running.
    more = uv_loop_alive(eventLoop_);
    LOGW("more=%d after call `beforeExit`", more);
  } while (more);
}


void NodeRuntime::Exit(int code) {
  LOGI("NodeRuntime.Exit(%d)", code);
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

bool NodeRuntime::InitLoop() {
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

jobject NodeRuntime::ToJava(JNIEnv *env, v8::Local<v8::Value> value) {
  if (value->IsNull()) {
    return this->jNull_;
  } else if (value->IsUndefined()) {
    return this->jUndefined_;
  } else if (value->IsBoolean()) {
    v8::Local<v8::Boolean> target = value->ToBoolean(isolate_);
    if (target->Value()) {
      return this->jTrue_;
    } else {
      return this->jFalse_;
    }
  } else {
    auto reference = new v8::Persistent<v8::Value>(isolate_, value);
    if (value->IsNumber()) {
      return JSNumber::Wrap(env, this, reference);
    } else if (value->IsObject()) {
      if (value->IsFunction()) {
        return JSFunction::Wrap(env, this, reference);
      } else if (value->IsPromise()) {
        return JSPromise::Wrap(env, this, reference);
      } else if (value->IsNativeError()) {
        return JSError::Wrap(env, this, reference);
      } else if (value->IsArray()) {
        return JSArray::Wrap(env, this, reference);
      }
      return JSObject::Wrap(env, this, reference);
    } else if (value->IsString()) {
      return JSString::Wrap(env, this, reference);
    }
    return JSValue::Wrap(env, this, reference);
  }
}

void NodeRuntime::TryLoop() {
  if (!eventLoop_) {
    LOGE("TryLoop but eventLoop is NULL");
    return;
  }
  if (!callbackHandle_) {
    callbackHandle_ = new uv_async_t();
    callbackHandle_->data = this;
    uv_async_init(eventLoop_, callbackHandle_, [](uv_async_t *handle) {
      auto *runtime = reinterpret_cast<NodeRuntime *>(handle->data);
      runtime->Handle(handle);
    });
    uv_async_send(callbackHandle_);
  }
}

bool NodeRuntime::Await(const std::function<void()> &runnable) {
  assert(isolate_ != nullptr);
  if (std::this_thread::get_id() == threadId_) {
    runnable();
    return true;
  } else if (strict_) {
    ENTER_JNI(vm_)
      env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "Illegal thread access");
    EXIT_JNI(vm_)
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

bool NodeRuntime::Post(const std::function<void()> &runnable) {
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

void NodeRuntime::Handle(uv_async_t *handle) {
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

// v8::Local<v8::Value> NodeRuntime::Require(const char *path) {
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

v8::Local<v8::Value> NodeRuntime::Require(const char *path) {
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

void NodeRuntime::Mount(const char *src, const char *dst, const int mode) {
  v8::Locker _locker(isolate_);
  v8::HandleScope _handleScope(isolate_);
  auto context = context_.Get(isolate_);
  auto env = node::GetCurrentEnvironment(context);
  node::Mount(env, src, dst, mode);
}

void NodeRuntime::Chroot(const char *path) {
  v8::Locker _locker(isolate_);
  v8::HandleScope _handleScope(isolate_);
  auto context = context_.Get(isolate_);
  auto env = node::GetCurrentEnvironment(context);
  node::Chroot(env, path);
}

void NodeRuntime::Throw(JNIEnv *env, v8::Local<v8::Value> exception) {
  auto jException = JSError::ToException(env, this, exception);
  env->Throw((jthrowable) jException);
}

void NodeRuntime::CheckThread() {
  if (std::this_thread::get_id() != threadId_) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    LOGE("js object can only be accessed from the Node.js thread: current=%ld, threadId=%ld", std::this_thread::get_id(), threadId_);
#pragma clang diagnostic pop
    std::abort();
  }
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
  for (const std::string &error : errors)
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
