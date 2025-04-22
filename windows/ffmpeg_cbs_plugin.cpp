#include "ffmpeg_cbs_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <shlwapi.h>

namespace ffmpeg_cbs {
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
    std::thread stderr_thread_;
    HANDLE stderr_read_handle_ = NULL;
    PROCESS_INFORMATION ffmpeg_process_info_ = {};
    std::atomic<bool> recording_active(false);
    extern "C" IMAGE_DOS_HEADER __ImageBase;

    std::wstring ToWString(const std::string& str) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        std::wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size_needed);
        result.pop_back(); // Remove null terminator
        return result;
    }
    std::wstring GetPluginLibraryPath() {
        HMODULE hModule = NULL;
        // Get handle to the plugin DLL (current module)
        hModule = reinterpret_cast<HMODULE>(&__ImageBase);

        wchar_t path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);
        PathRemoveFileSpecW(path);  // remove DLL filename
        return std::wstring(path);  // e.g. ...\plugins\ffmpeg_cbs\Debug
    }
    std::string ToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        std::string str(size_needed - 1, 0);  // -1 to exclude null terminator
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);
        return str;
    }

    void SendEvent(const flutter::EncodableMap& map) {
        if (event_sink_) {
            event_sink_->Success(flutter::EncodableValue(map));
        }
    }
    /*
    * bool StartFFmpegProcess(const std::string& command_utf8) {
        if (ffmpeg_process_info_.hProcess != NULL) {
            OutputDebugStringW(L"[FFmpegCBS] FFmpeg process already running.\n");
            return false;
        }

        std::wstring plugin_path = GetPluginExecutablePath();
        std::wstring ffmpeg_path = plugin_path + L"bin\\ffmpeg.exe";
        //std::wstring ffmpeg_path = plugin_path + L"\\..\\..\\..\\..\\..\\plugins\\ffmpeg_cbs\\bin\\ffmpeg.exe";

        if (GetFileAttributesW(ffmpeg_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            DWORD err_code = GetLastError();
            std::wstringstream err_stream;
            err_stream << L"[FFmpegCBS] executable not found at: " << ffmpeg_path
                << L" (Windows error code: " << err_code << L")";

            OutputDebugStringW(err_stream.str().c_str());
            return false;
        }

        std::wstring command = L"\"" + ffmpeg_path + L"\" " + ToWString(command_utf8);

        // Debugging log
        OutputDebugStringW((L"[FFmpegCBS] FFmpeg path: " + ffmpeg_path + L"\n").c_str());
        OutputDebugStringW((L"[FFmpegCBS] Full command: " + command + L"\n").c_str());

        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE stderr_write = NULL;

        if (!CreatePipe(&stderr_read_handle_, &stderr_write, &saAttr, 0)) {
            OutputDebugStringW(L"[FFmpegCBS] Failed to create stderr pipe.\n");
            return false;
        }

        SetHandleInformation(stderr_read_handle_, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = NULL;
        si.hStdInput = NULL;
        si.hStdError = stderr_write;

        BOOL success = CreateProcessW(
            NULL, &command[0], NULL, NULL, TRUE, CREATE_NO_WINDOW,
            NULL, NULL, &si, &ffmpeg_process_info_);

        CloseHandle(stderr_write);

        if (!success) {
            DWORD err_code = GetLastError();
            std::wstringstream err_stream;
            err_stream << L"[FFmpegCBS] CreateProcessW failed. Error code: " << err_code << L"\n";
            OutputDebugStringW(err_stream.str().c_str());

            CloseHandle(stderr_read_handle_);
            stderr_read_handle_ = NULL;
            return false;
        }

        OutputDebugStringW(L"[FFmpegCBS] FFmpeg process started successfully.\n");

        stderr_thread_ = std::thread([]() {
            char buffer[512];
            DWORD bytesRead;
            while (ReadFile(stderr_read_handle_, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                buffer[bytesRead] = '\0';
                std::string log_line(buffer);
                if (event_sink_) {
                    flutter::EncodableMap log_message = {
                        {flutter::EncodableValue("status"), flutter::EncodableValue("log")},
                        {flutter::EncodableValue("message"), flutter::EncodableValue(log_line)} };
                    event_sink_->Success(flutter::EncodableValue(log_message));
                }
            }

            if (event_sink_) {
                flutter::EncodableMap done_message = {
                    {flutter::EncodableValue("status"), flutter::EncodableValue("completed")},
                    {flutter::EncodableValue("message"), flutter::EncodableValue("Recording completed.")} };
                event_sink_->Success(flutter::EncodableValue(done_message));
            }

            CloseHandle(stderr_read_handle_);
            stderr_read_handle_ = NULL;
            });

        return true;
    }
    */
    std::string StartFFmpegProcess(const std::string& command_utf8) {
        if (ffmpeg_process_info_.hProcess != NULL) {
            return "FFmpeg process already running.";
        }

        std::wstring ffmpeg_path = GetPluginLibraryPath() + L"\\bin\\ffmpeg.exe";

        if (GetFileAttributesW(ffmpeg_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            DWORD err_code = GetLastError();
            std::wstringstream detail;
            detail << L"[FFmpegCBS] executable not found at: " << ffmpeg_path
                << L" (Windows error code: " << err_code << L")";
            OutputDebugStringW(detail.str().c_str());
            return ToUtf8(detail.str());
        }

        std::wstring command = L"\"" + ffmpeg_path + L"\" " + ToWString(command_utf8);
        OutputDebugStringW((L"[FFmpegCBS] Full command: " + command + L"\n").c_str());

        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE stderr_write = NULL;

        if (!CreatePipe(&stderr_read_handle_, &stderr_write, &saAttr, 0)) {
            return "Failed to create stderr pipe.";
        }

        SetHandleInformation(stderr_read_handle_, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = NULL;
        si.hStdInput = NULL;
        si.hStdError = stderr_write;

        BOOL success = CreateProcessW(
            NULL, &command[0], NULL, NULL, TRUE, CREATE_NO_WINDOW,
            NULL, NULL, &si, &ffmpeg_process_info_);

        CloseHandle(stderr_write);

        if (!success) {
            DWORD err_code = GetLastError();
            std::stringstream detail;
            detail << "[FFmpegCBS] CreateProcessW failed. Windows error code: " << err_code;
            OutputDebugStringA(detail.str().c_str());
            CloseHandle(stderr_read_handle_);
            stderr_read_handle_ = NULL;
            return detail.str();
        }

        OutputDebugStringW(L"[FFmpegCBS] FFmpeg process started successfully.\n");

        stderr_thread_ = std::thread([]() {
            char buffer[512];
            DWORD bytesRead;
            while (ReadFile(stderr_read_handle_, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                if (bytesRead < sizeof(buffer)) {
                    buffer[static_cast<size_t>(bytesRead)] = '\0';
                }
                std::string log_line(buffer);
                if (event_sink_) {
                    flutter::EncodableMap log_message = {
                        {flutter::EncodableValue("status"), flutter::EncodableValue("log")},
                        {flutter::EncodableValue("message"), flutter::EncodableValue(log_line)} };
                    event_sink_->Success(flutter::EncodableValue(log_message));
                }
            }

            if (event_sink_) {
                flutter::EncodableMap done_message = {
                    {flutter::EncodableValue("status"), flutter::EncodableValue("completed")},
                    {flutter::EncodableValue("message"), flutter::EncodableValue("Recording completed.")} };
                event_sink_->Success(flutter::EncodableValue(done_message));
            }

            CloseHandle(stderr_read_handle_);
            stderr_read_handle_ = NULL;
            });

        return "";  // success
    }


    bool StopFFmpegProcess() {
        if (ffmpeg_process_info_.hProcess == NULL) return false;

        TerminateProcess(ffmpeg_process_info_.hProcess, 0);
        WaitForSingleObject(ffmpeg_process_info_.hProcess, 3000);

        CloseHandle(ffmpeg_process_info_.hProcess);
        CloseHandle(ffmpeg_process_info_.hThread);
        ffmpeg_process_info_ = {};

        if (stderr_thread_.joinable()) {
            stderr_thread_.join();
        }

        return true;
    }


    // Plugin registration
    void FFMpegCBSPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarWindows* registrar) {
        auto plugin = std::make_unique<FFMpegCBSPlugin>();

        auto method_channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "ffmpeg_cbs/method",
            &flutter::StandardMethodCodec::GetInstance());

        method_channel->SetMethodCallHandler(
            [plugin_pointer = plugin.get()](const auto& call, auto result) {
                plugin_pointer->HandleMethodCall(call, std::move(result));
            });

        auto event_channel = std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "ffmpeg_cbs/event",
            &flutter::StandardMethodCodec::GetInstance());

        auto stream_handler =
            std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
                [](const flutter::EncodableValue* /*arguments*/,
                    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& sink)
                -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {
                    event_sink_ = std::move(sink);
                    return nullptr;
                },
                [](const flutter::EncodableValue* /*arguments*/)
                -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {
                    event_sink_ = nullptr;
                    return nullptr;
                });

        event_channel->SetStreamHandler(std::move(stream_handler));
        registrar->AddPlugin(std::move(plugin));
    }

    FFMpegCBSPlugin::FFMpegCBSPlugin() {}
    FFMpegCBSPlugin::~FFMpegCBSPlugin() {}

    void FFMpegCBSPlugin::HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue>& call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
        const std::string& method = call.method_name();

        if (method == "getPlatformVersion") {
            std::ostringstream version_stream;
            version_stream << "Windows ";
            if (IsWindows10OrGreater()) {
                version_stream << "10+";
            }
            else if (IsWindows8OrGreater()) {
                version_stream << "8";
            }
            else {
                version_stream << "7";
            }
            result->Success(flutter::EncodableValue(version_stream.str()));
            return;
        }

        if (method == "startRecord") {
            if (!call.arguments()) {
                result->Error("INVALID_ARGUMENT", "Arguments must be a map.");
                return;
            }

            const auto& args = std::get<flutter::EncodableMap>(*call.arguments());
            const auto it = args.find(flutter::EncodableValue("command"));
            if (it == args.end()) {
                result->Error("INVALID_ARGUMENT", "Missing or invalid 'command'.");
                return;
            }

            std::string command = std::get<std::string>(it->second);
            /*
            bool started = StartFFmpegProcess(command);
            if (!started) {
                DWORD err_code = GetLastError();
                std::stringstream detail;
                detail << "CreateProcessW failed. Windows error code: " << err_code;
                result->Error("START_FAILED", "Failed to start FFmpeg process.", flutter::EncodableValue(detail.str()));
            }
            else {
                result->Success();
            }
            */
            std::string error = StartFFmpegProcess(command);
            if (!error.empty()) {
                result->Error("START_FAILED", "Failed to start FFmpeg process.", flutter::EncodableValue(error));
            }
            else {
                result->Success();
            }
            return;
        }

        if (method == "stopRecord") {
            bool stopped = StopFFmpegProcess();
            result->Success(flutter::EncodableValue(stopped));
            return;
        }

        result->NotImplemented();
    }
}  // namespace ffmpeg_cbs
