package com.linroid.knode

import androidx.test.platform.app.InstrumentationRegistry
import com.linroid.knode.js.JSContext
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Before
import java.io.File
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/**
 * @author linroid
 * @since 2019/11/27
 */
abstract class KNodeTest : KNode.EventListener, StdOutput {

  private lateinit var file: File
  private val startLatch = CountDownLatch(1)

  private lateinit var node: KNode
  protected lateinit var context: JSContext

  @Before
  fun setupNode() {
    println("setupNode")
    val appContext = InstrumentationRegistry.getInstrumentation().targetContext
    file = File.createTempFile("tests_", "temp.js")
    node = KNode(appContext.cacheDir, this, true)
    node.start(file.absolutePath)
    node.addEventListener(this)
    startLatch.await(3, TimeUnit.SECONDS)
    println("Node.js is ready")
  }

  override fun onNodePrepared(context: JSContext) {
    super.onNodePrepared(context)
    println("onNodePrepared")
    this.context = context
    startLatch.countDown()
  }

  @After
  fun destroyNode() {
    println("destroyNode")
    file.delete()
    node.exit(0)
  }

  override fun stdout(str: String) {
    println(str)
  }

  override fun stderr(str: String) {
    println(str)
  }

  protected fun runInNode(action: () -> Unit) {
    runBlocking {
      node.await(action)
    }
  }

  companion object {
    init {
      val appContext = InstrumentationRegistry.getInstrumentation().targetContext
      KNode.setup(appContext)
    }
  }
}
