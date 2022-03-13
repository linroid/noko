package com.linroid.noko

import android.app.Application
import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.system.Os
import com.linroid.noko.Node

class TestApplication : Application() {
  override fun onCreate() {
    super.onCreate()
    val connectionManager = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
    setDnsEnv(connectionManager)

    val request = NetworkRequest.Builder()
      .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
      .build()
    connectionManager.registerNetworkCallback(
      request,
      object : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
          setDnsEnv(connectionManager)
        }
      })
    Node.setup(4, connectionManager)
  }

  fun setDnsEnv(connectionManager: ConnectivityManager) {
    val network = connectionManager.activeNetwork

    val properties = connectionManager.getLinkProperties(network) ?: return
    if (properties.dnsServers.isNotEmpty()) {
      Os.setenv("DNS_SERVERS", properties.dnsServers.joinToString(",") {
        it.hostAddress ?: "8.8.8.8"
      }, true)
    } else {
      Os.unsetenv("DNS_SERVERS")
    }

    val domains = properties.domains
    Os.setenv("DNS_DOMAINS", domains ?: "", true)
  }
}
