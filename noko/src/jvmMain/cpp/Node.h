#ifndef NODE_NOKO_H
#define NODE_NOKO_H

#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <node.h>
#include <uv.h>
#include "macros.h"

int init_node();

enum JSType {
  /** Cached types */
  kNone,
  kUndefined,
  kNull,
  kBoolean,

  /** Non cached types */
  kValue,
  kArray,
  kObject,
  kString,
  kNumber,
  kFunction,
  kPromise,
  kError,
};

class Node {

private:
  static jmethodID jAttachId;
  static jmethodID jDetachId;

  static jfieldID jSharedNullId;
  static jfieldID jSharedUndefinedId;
  static jfieldID jSharedTrueId;
  static jfieldID jSharedFalseId;
  static jfieldID jPointerId;

  jobject jThis_ = nullptr;

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

  void Detach() const;

public:
  jobject jSharedNull_ = nullptr;
  jobject jSharedUndefined_ = nullptr;
  jobject jSharedTrue_ = nullptr;
  jobject jSharedFalse_ = nullptr;

  JavaVM *vm_ = nullptr;

  v8::Isolate *isolate_ = nullptr;
  v8::Persistent<v8::Object> global_;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Function> require_;

  Node(JNIEnv *env, jobject jThis, bool keepAlive, bool strict);

  ~Node();

  int Start(std::vector<std::string> &args);

  void Exit(int code);

  bool Await(const std::function<void()> &runnable);

  bool Post(const std::function<void()> &runnable);

  jobject ToJava(JNIEnv *env, v8::Local<v8::Value> value) const;

  v8::Local<v8::Value> Require(const char *path);

  void MountFile(const char *src, const char *dst, int mode);

  void Chroot(const char *path);

  void Throw(JNIEnv *env, v8::Local<v8::Value> exception);

  void CheckThread();

  bool IsRunning() const {
    return running_;
  }

  jobject Eval(const uint16_t *code, int codeLen, const uint16_t *source, int sourceLen, int line);

  jobject ParseJson(const uint16_t *json, int jsonLen);

  jobject ThrowError(const uint16_t *message, int messageLen) const;

  jobject Require(const uint16_t *path, int pathLen);

  static jint OnLoad(JNIEnv *env);

  static Node* From(JNIEnv *env, jobject jThis) {
    jlong pointer = env->GetLongField(jThis, jPointerId);
    return reinterpret_cast<Node *>(pointer);
  }
};

#endif //NODE_NOKO_H
