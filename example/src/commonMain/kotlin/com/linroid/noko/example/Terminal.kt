package com.linroid.noko.example

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.Text
import androidx.compose.material.Surface
import androidx.compose.material.MaterialTheme
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import com.linroid.noko.Node
import com.linroid.noko.awaitStarted
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.flow.collect

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
    Column {
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
      Text(output)
      var input by remember { mutableStateOf("") }
      BasicTextField(
        value = input,
        onValueChange = {
          input = it
        },
        keyboardActions = KeyboardActions(onSend = {
          node.stdio.write(input + "\n")
          input = ""
        }),
        keyboardOptions = KeyboardOptions(imeAction = ImeAction.Send),
      )
    }
  }
}
