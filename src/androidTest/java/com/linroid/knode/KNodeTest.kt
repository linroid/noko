package com.linroid.knode

import androidx.test.platform.app.InstrumentationRegistry
import com.linroid.knode.js.JSContext
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

    protected lateinit var node: KNode
    protected lateinit var context: JSContext
    private lateinit var file: File
    private val startlatch = CountDownLatch(1)
    @Before
    fun setupNode() {
        val appContext = InstrumentationRegistry.getInstrumentation().targetContext
        file = File.createTempFile("knode_test", this.javaClass.name)
        file.writeText("setInterval(() => { }, 50)")
        node = KNode(appContext.cacheDir, this, false)
        node.start(file)
        node.addEventListener(this)
        startlatch.await(3, TimeUnit.SECONDS)
    }

    override fun onNodePrepared(context: JSContext) {
        super.onNodePrepared(context)
        this.context = context
        startlatch.countDown()
    }

    @After
    fun destroyNode() {
        file.delete()
        node.exit(0)
    }

    override fun stdout(str: String) {
        println(str)
    }

    override fun stderr(str: String) {
        println(str)
    }
}