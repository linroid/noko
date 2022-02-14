//
// Created by linroid on 2019-10-19.
//

#ifndef NODE_NODE_RUNTIME_H
#define NODE_NODE_RUNTIME_H

#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <node.h>
#include <uv.h>
#include "macros.h"

int init_node();

enum JSType {
  /** cached types */
  kNone,
  kUndefined,
  kNull,
  kBoolean,

  /** non cached types */
  kValue,
  kArray,
  kObject,
  kString,
  kNumber,
  kFunction,
  kPromise,
  kError,
};

class NodeRuntime {

private:
  jobject jThis_ = nullptr;
  jmethodID jAttach_ = nullptr;
  jmethodID jDetach_ = nullptr;

  bool running_ = false;
  std::thread::id threadId_;
  std::mutex asyncMutex_;
  std::vector<std::function<void()>> callbacks_;
  int id_ = -1;
  bool keepAlive_ = false;
  bool strict_ = false;

  static std::mutex sharedMutex_;
  static jint instanceCount_;
  static jint seq_;

  uv_loop_t *eventLoop_ = nullptr;
  uv_async_t *keepAliveHandle_ = nullptr;
  uv_async_t *callbackHandle_ = nullptr;

  v8::Persistent<v8::Object> process_;

  void Handle(uv_async_t *handle);

  bool InitLoop();

  void TryLoop();

  void RunLoop(node::Environment *env);

  void CloseLoop();

  void Attach();

  void Detach();

public:
  jobject jContext_ = nullptr;
  jobject jNull_ = nullptr;
  jobject jUndefined_ = nullptr;
  jobject jTrue_ = nullptr;
  jobject jFalse_ = nullptr;
  JavaVM *vm_ = nullptr;

  v8::Isolate *isolate_ = nullptr;
  v8::Persistent<v8::Object> global_;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Function> require_;

  NodeRuntime(JNIEnv *env, jobject jThis, jmethodID jAttach, jmethodID jDetach, bool keepAlive, bool strict);

  ~NodeRuntime();

  int Start(std::vector<std::string> &args);

  void Exit(int code);

  bool Await(const std::function<void()> &runnable);

  bool Post(const std::function<void()> &runnable);

  jobject ToJava(JNIEnv *env, v8::Local<v8::Value> value);

  v8::Local<v8::Value> Require(const char *path);

  void MountFile(const char *src, const char *dst, const int mode);

  void Chroot(const char *path);

  void Throw(JNIEnv *env, v8::Local<v8::Value> exception);

  void CheckThread();

  bool IsRunning() {
    return running_;
  }
};

#endif //NODE_NODE_RUNTIME_H