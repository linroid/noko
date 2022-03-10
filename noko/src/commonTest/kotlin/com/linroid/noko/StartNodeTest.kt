package com.linroid.noko

import io.mockk.mockk
import io.mockk.verify
import kotlinx.coroutines.withTimeout
import kotlin.test.Test
import kotlin.test.assertEquals

class StartNodeTest : SetupNode() {

  @Test
  fun lifecycesl_normal() {
    val node = Node(keepAlive = true)
    val listener: LifecycleListener = mockk(relaxed = true)
    node.addListener(listener)
    node.start("-p", "process.version.node")
    verify(timeout = 1000) { listener.onAttach(node, any()) }
    verify(timeout = 1000) { listener.onStart(node, any()) }
    node.exit(0)
    verify(timeout = 1000) { listener.onDetach(node, any()) }
    verify(timeout = 1000) { listener.onStop(0) }
  }

  @Test
  fun lifecycles_error() {
    val node = Node(keepAlive = true)
    val listener: LifecycleListener = mockk(relaxed = true)
    node.addListener(listener)
    node.start("-p", "process.version.node")
    verify(timeout = 1000) { listener.onAttach(node, any()) }
    verify(timeout = 1000) { listener.onStart(node, any()) }
    node.post({
      node.eval("setTimeout(() => { throw \"test\"}, 1);")
    }, false)
    // verify(timeout = 1000) { listener.onError(any()) }
    verify(timeout = 1000) { listener.onDetach(node, any()) }
    verify(timeout = 1000) { listener.onStop(1) }
  }

  @Test
  fun keepAlive_false() {
    val node = Node(keepAlive = false)
    node.start("-p", "process.version.node")
    runBlocking {
      node.awaitStarted()
      withTimeout(3000) {
        node.awaitStopped()
      }
    }
  }

  @Test
  fun exit_with_zero() {
    val node = Node(keepAlive = true)
    node.start("-p", "process.version.node")
    runBlocking {
      node.awaitStarted()
      node.exit(0)
      withTimeout(3000) {
        assertEquals(0, node.awaitStopped())
      }
    }
  }

  @Test
  fun exit_with_non_zero() {
    val node = Node(keepAlive = true)
    node.start("-p", "process.version.node")
    runBlocking {
      node.awaitStarted()
      node.exit(1)
      withTimeout(3000) {
        assertEquals(1, node.awaitStopped())
      }
    }
  }
}