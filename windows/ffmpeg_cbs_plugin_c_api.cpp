#include "include/ffmpeg_cbs/ffmpeg_cbs_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "ffmpeg_cbs_plugin.h"

void FfmpegCbsPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ffmpeg_cbs::FfmpegCbsPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
