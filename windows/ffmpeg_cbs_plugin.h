#ifndef FLUTTER_PLUGIN_FFMPEG_CBS_PLUGIN_H_
#define FLUTTER_PLUGIN_FFMPEG_CBS_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace ffmpeg_cbs {

class FfmpegCbsPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FfmpegCbsPlugin();

  virtual ~FfmpegCbsPlugin();

  // Disallow copy and assign.
  FfmpegCbsPlugin(const FfmpegCbsPlugin&) = delete;
  FfmpegCbsPlugin& operator=(const FfmpegCbsPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace ffmpeg_cbs

#endif  // FLUTTER_PLUGIN_FFMPEG_CBS_PLUGIN_H_
