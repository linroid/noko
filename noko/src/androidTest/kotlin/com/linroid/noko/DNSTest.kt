package com.linroid.noko

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.google.gson.JsonParser
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class DNSTest : NodeTest() {

  @Test
  fun getServers() {
    val result = context.eval("new (require('dns').promises.Resolver)().getServers()")
    val json = result.toJson()
    val servers = JsonParser.parseString(json).asJsonArray.map { it.asString }
    assertTrue(servers.isNotEmpty())
  }
}
