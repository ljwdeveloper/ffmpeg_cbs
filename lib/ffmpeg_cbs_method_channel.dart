import 'dart:async';
import 'dart:developer';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'ffmpeg_cbs_platform_interface.dart';

/// An implementation of [FFMpegCBSPlatform] that uses method channels.
class MethodChannelFFMpegCBS extends FFMpegCBSPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('ffmpeg_cbs/method');
  @visibleForTesting
  final eventChannel = const EventChannel('ffmpeg_cbs/event');

  MethodChannelFFMpegCBS() {
    setupListener();
  }
  @override
  void setupListener() {
    // TODO: implement setupListener
    eventChannel.receiveBroadcastStream().listen((event) {
      if (event is! Map) {
        log('[ffmpeg_cbs] Recording FAILED!!!! parsing err; event type not "Map"; ${event.runtimeType}',
            name: runtimeType.toString());
      }
      final parsed = Map<String, dynamic>.from(
        event.map((k, v) => MapEntry(k.toString(), v)),
      );
      final status = parsed['status'];
      if (status == 'error') {
        log('[ffmpeg_cbs] Recording FAILED!!!! ${event['error']}',
            name: runtimeType.toString());
      } else {
        log('[ffmpeg_cbs] Recording completed: ${event['duration']} sec',
            name: runtimeType.toString());
      }
      if (fileProcessCompleter.isCompleted) return;
      fileProcessCompleter.complete();
    });
  }

  @override
  Future<String?> getPlatformVersion() async {
    final version =
        await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

  @override
  Future<void> startRecordWithCommand(String command) async {
    try {
      if (!recordingCompleter.isCompleted) {
        await stopRecord();
        await fileProcessCompleter.future;
      }
      recordingCompleter = Completer();
      await methodChannel.invokeMethod('startRecord', {'command': command});
      log('[ffmpeg_cbs] startRecord invoked', name: runtimeType.toString());
    } on PlatformException catch (e) {
      log('[ffmpeg_cbs] startRecord error: ${e.message}',
          name: runtimeType.toString());
      recordingCompleter = Completer()..complete();
      rethrow;
    }
  }

  @override
  Future<void> stopRecord() async {
    try {
      if (!recordingCompleter.isCompleted) {
        recordingCompleter.complete();
      }
      fileProcessCompleter = Completer();
      await methodChannel.invokeMethod('stopRecord');
    } on PlatformException catch (e) {
      log('[ffmpeg_cbs] stopRecord error: ${e.message}',
          name: runtimeType.toString());
      rethrow;
    }
  }
}
