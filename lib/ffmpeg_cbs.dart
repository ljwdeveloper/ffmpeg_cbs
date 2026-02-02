// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/to/pubspec-plugin-platforms.

import 'dart:async';

import 'ffmpeg_cbs_platform_interface.dart';
export 'ffmpeg_cbs_platform_interface.dart' show RecordingStatus;

class FFMpegCBS {
  Future<String?> getPlatformVersion() {
    return FFMpegCBSPlatform.instance.getPlatformVersion();
  }

  Future<void> startRecord(
      {required String audioRecordPath,
      required String fileName,
      required String streamUrl}) async {
    final outputMp3Path = '$audioRecordPath/$fileName.mp3';
    final commandString =
        '-i "$streamUrl" -acodec libmp3lame -b:a 192k -f mp3 "$outputMp3Path"';

    await FFMpegCBSPlatform.instance.startRecordWithCommand(commandString);
  }

  /// Native코드 통해서 FFMpeg 작업 종료 명령(```session.cancel()```)을 내린다.
  /// 주의: recording 마무리작업을 뜻하지 않는다.
  ///
  /// 마무리작업이 완료된 후의 반환은 ```fileProcessCompleter.future```
  Future<void> stopRecord() async => FFMpegCBSPlatform.instance.stopRecord();

  ValueStream<RecordingStatus> get recordingStatusStream =>
      FFMpegCBSPlatform.instance.recordingStatusStream;
}
