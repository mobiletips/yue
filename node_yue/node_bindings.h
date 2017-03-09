// Copyright 2014 GitHub, Inc.
// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NODE_YUE_NODE_BINDINGS_H_
#define NODE_YUE_NODE_BINDINGS_H_

#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "uv.h"  // NOLINT(build/include)
#include "v8.h"  // NOLINT(build/include)

namespace node {
class Environment;
}

namespace node_yue {

class NodeBindings {
 public:
  static NodeBindings* Create();

  virtual ~NodeBindings();

  // Prepare for message loop integration.
  void PrepareMessageLoop();

  // Do message loop integration.
  virtual void RunMessageLoop();

 protected:
  NodeBindings();

  // Called to poll events in new thread.
  virtual void PollEvents() = 0;

  // Run the libuv loop for once.
  void UvRunOnce();

  // Make the main thread run libuv loop.
  void WakeupMainThread();

  // Interrupt the PollEvents.
  void WakeupEmbedThread();

  // Main thread's MessageLoop.
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  // Main thread's libuv loop.
  uv_loop_t* uv_loop_;

 private:
  // Thread to poll uv events.
  static void EmbedThreadRunner(void *arg);

  // Whether the libuv loop has ended.
  bool embed_closed_;

  // Dummy handle to make uv's loop not quit.
  uv_async_t dummy_uv_handle_;

  // Thread for polling events.
  uv_thread_t embed_thread_;

  // Semaphore to wait for main loop in the embed thread.
  uv_sem_t embed_sem_;

  base::WeakPtrFactory<NodeBindings> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeBindings);
};

}  // namespace node_yue

#endif  // NODE_YUE_NODE_BINDINGS_H_
