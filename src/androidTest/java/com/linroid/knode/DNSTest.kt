package com.linroid.knode

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.google.gson.JsonParser
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

/**
 * @author linroid
 * @since 2020/5/5
 */
@RunWith(AndroidJUnit4::class)
class DNSTest : KNodeTest() {

  @Test
  fun getServers() {
    val result = context.eval("new (require('dns').promises.Resolver)().getServers()")
    val json = result.toJson()
    val servers = JsonParser.parseString(json).asJsonArray.map { it.asString }
    assertTrue(servers.isNotEmpty())
  }
}
