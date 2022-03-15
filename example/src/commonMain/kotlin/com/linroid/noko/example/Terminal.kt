package com.linroid.noko.example

import androidx.compose.foundation.gestures.rememberScrollableState
import androidx.compose.foundation.gestures.scrollable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.Text
import androidx.compose.material.Surface
import androidx.compose.material.MaterialTheme
import androidx.compose.material.TextField
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.input.key.key
import androidx.compose.ui.input.key.onPreviewKeyEvent
import androidx.compose.ui.text.input.ImeAction
import com.linroid.noko.Node
import com.linroid.noko.awaitStarted
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.flow.collect
import androidx.compose.ui.input.key.Key
import androidx.compose.ui.input.key.type
import androidx.compose.ui.input.key.KeyEventType
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalComposeUiApi::class)
@Composable
fun Terminal() {
  // A surface container using the 'background' color from the theme
  Surface(
    modifier = Modifier.fillMaxSize(),
    color = MaterialTheme.colors.background
  ) {
    val node = remember {
      Node(keepAlive = true).also {
        it.start("-i")
      }
    }
    Column(modifier = Modifier.padding(horizontal = 32.dp, vertical = 16.dp)) {
      var output by remember { mutableStateOf("") }
      LaunchedEffect(Unit) {
        node.awaitStarted()
        coroutineScope {
          launch(node.coroutineDispatcher) {
            node.stdio.output().collect {
              output += it
            }
          }
          launch(node.coroutineDispatcher) {
            node.stdio.error().collect {
              output += it
            }
          }
        }
      }
      var input by remember { mutableStateOf("") }
      TextField(
        value = input,
        onValueChange = {
          input = it
        },
        singleLine = true,
        keyboardActions = KeyboardActions(
          onSend = {
            node.stdio.write(input + "\n")
            input = ""
          }),
        keyboardOptions = KeyboardOptions(imeAction = ImeAction.Send),
        modifier = Modifier.align(Alignment.CenterHorizontally).onPreviewKeyEvent {event->
          if (event.key == Key.Enter&& event.type == KeyEventType.KeyUp) {
            node.stdio.write(input + "\n")
            input = ""
            return@onPreviewKeyEvent true
          }
          return@onPreviewKeyEvent false
        }
      )
      val scrollState = rememberScrollState()
      Text(output, modifier = Modifier.verticalScroll(scrollState))
    }
  }
}
