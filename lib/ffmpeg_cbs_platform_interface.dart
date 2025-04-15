import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'ffmpeg_cbs_method_channel.dart';

abstract class FfmpegCbsPlatform extends PlatformInterface {
  /// Constructs a FfmpegCbsPlatform.
  FfmpegCbsPlatform() : super(token: _token);

  static final Object _token = Object();

  static FfmpegCbsPlatform _instance = MethodChannelFfmpegCbs();

  /// The default instance of [FfmpegCbsPlatform] to use.
  ///
  /// Defaults to [MethodChannelFfmpegCbs].
  static FfmpegCbsPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FfmpegCbsPlatform] when
  /// they register themselves.
  static set instance(FfmpegCbsPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
