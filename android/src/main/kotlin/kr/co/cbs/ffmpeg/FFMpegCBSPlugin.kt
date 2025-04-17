package com.example.ffmpeg_cbs

import android.os.Build
import android.util.Log
import com.arthenica.ffmpegkit.FFmpegKit
import com.arthenica.ffmpegkit.FFmpegSession
import com.arthenica.ffmpegkit.ReturnCode
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.EventChannel
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler

/** FFMpegCBSPlugin */
class FFMpegCBSPlugin : FlutterPlugin, MethodCallHandler, EventChannel.StreamHandler {
  private lateinit var methodChannel: MethodChannel
  private lateinit var eventChannel: EventChannel
  private var eventSink: EventChannel.EventSink? = null
  private var session: FFmpegSession? = null

  override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    methodChannel = MethodChannel(flutterPluginBinding.binaryMessenger, "ffmpeg_cbs/method")
    eventChannel = EventChannel(flutterPluginBinding.binaryMessenger, "ffmpeg_cbs/event")

    methodChannel.setMethodCallHandler(this)
    eventChannel.setStreamHandler(this)
    Log.d("FFMpegCBS", "Plugin registered successfully")
  }

  override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
    when (call.method) {
      "getPlatformVersion" -> {
        result.success("Android ${Build.VERSION.RELEASE}")
      }
      "startRecord" -> {
        val command = call.argument<String>("command")
        if (command == null) {
          Log.e("FFMpegCBS", "Invalid arguments for startRecord")
          result.error("INVALID_ARGUMENT", "Missing command", null)
          return
        }
        Log.d("FFMpegCBS", "Starting recording with command: $command")
        startRecording(command)
        result.success(null)
      }
      "stopRecord" -> {
        Log.d("FFMpegCBS", "Stopping recording session")
        stopRecording()
        result.success(null)
      }
      else -> {
        result.notImplemented()
      }
    }
  }

  private fun startRecording(command: String) {
    session = FFmpegKit.executeAsync(command,
      { session -> handleFFmpegCompletion(session) },
      { log ->
        log?.let {
          Log.d("FFMpegCBS", "[Log] ${it.sessionId}: lvl${it.level} ${it.message}")
        }
      },
      { statistics ->
        // optional stats handling
      }
    )
  }

  private fun stopRecording() {
    session?.cancel()
    FFmpegKit.cancel()
  }

  private fun handleFFmpegCompletion(session: FFmpegSession?) {
    if (session == null) {
      Log.e("FFMpegCBS", "Session is null")
      return
    }

    val returnCode = session.returnCode
    val output = session.output ?: "(no output)"
    val failStackTrace = session.failStackTrace

    val startTime = session.startTime
    val endTime = session.endTime
    val durationSec = session.duration.toDouble() / 1000.0

    Log.d("FFMpegCBS", "Start: $startTime, End: $endTime, Duration: $durationSec sec")
    Log.d("FFMpegCBS", "ReturnCode: ${returnCode?.value} Output: $output")

    val message = mutableMapOf<String, Any>()
    when {
      ReturnCode.isSuccess(returnCode) -> {
        Log.d("FFMpegCBS", "Recording SUCCESS")
        message["status"] = "completed"
        message["duration"] = durationSec
      }
      ReturnCode.isCancel(returnCode) -> {
        Log.d("FFMpegCBS", "Recording CANCELLED")
        message["status"] = "cancelled"
        message["duration"] = durationSec
      }
      else -> {
        Log.d("FFMpegCBS", "Recording FAILED")
        message["status"] = "error"
        message["error"] = failStackTrace ?: "Unknown error"
      }
    }

    eventSink?.success(message)
  }

  override fun onListen(arguments: Any?, events: EventChannel.EventSink?) {
    Log.d("FFMpegCBS", "Event channel: onListen")
    eventSink = events
  }

  override fun onCancel(arguments: Any?) {
    Log.d("FFMpegCBS", "Event channel: onCancel")
    eventSink = null
  }

  override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    methodChannel.setMethodCallHandler(null)
    eventChannel.setStreamHandler(null)
  }
}