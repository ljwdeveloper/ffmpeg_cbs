import 'dart:async';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'ffmpeg_cbs_method_channel.dart';

abstract class FFMpegCBSPlatform extends PlatformInterface {
  /// Constructs a FfmpegCbsPlatform.
  FFMpegCBSPlatform() : super(token: _token);

  static final Object _token = Object();

  static FFMpegCBSPlatform _instance = MethodChannelFFMpegCBS();

  /// The default instance of [FFMpegCBSPlatform] to use.
  ///
  /// Defaults to [MethodChannelFFMpegCBS].
  static FFMpegCBSPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FFMpegCBSPlatform] when
  /// they register themselves.
  static set instance(FFMpegCBSPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  void setupListener() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future<void> startRecordWithCommand(String command) {
    throw UnimplementedError(
        'startRecordWithCommand() has not been implemented.');
  }

  Future<void> stopRecord() {
    throw UnimplementedError('stopRecord() has not been implemented.');
  }

  Completer<void> recordingCompleter = Completer()..complete();
  Completer<void> fileProcessCompleter = Completer()..complete();
}
