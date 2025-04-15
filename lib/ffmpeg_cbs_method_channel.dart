import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'ffmpeg_cbs_platform_interface.dart';

/// An implementation of [FfmpegCbsPlatform] that uses method channels.
class MethodChannelFfmpegCbs extends FfmpegCbsPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('ffmpeg_cbs');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
