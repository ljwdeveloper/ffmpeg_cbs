import 'package:flutter_test/flutter_test.dart';
import 'package:ffmpeg_cbs/ffmpeg_cbs.dart';
import 'package:ffmpeg_cbs/ffmpeg_cbs_platform_interface.dart';
import 'package:ffmpeg_cbs/ffmpeg_cbs_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFfmpegCbsPlatform
    with MockPlatformInterfaceMixin
    implements FfmpegCbsPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final FfmpegCbsPlatform initialPlatform = FfmpegCbsPlatform.instance;

  test('$MethodChannelFfmpegCbs is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFfmpegCbs>());
  });

  test('getPlatformVersion', () async {
    FfmpegCbs ffmpegCbsPlugin = FfmpegCbs();
    MockFfmpegCbsPlatform fakePlatform = MockFfmpegCbsPlatform();
    FfmpegCbsPlatform.instance = fakePlatform;

    expect(await ffmpegCbsPlugin.getPlatformVersion(), '42');
  });
}
