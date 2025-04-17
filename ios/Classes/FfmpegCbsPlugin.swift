import Flutter
import ffmpegkit

public class FFMpegCBSPlugin: NSObject, FlutterPlugin, FlutterStreamHandler {

    private var methodChannel: FlutterMethodChannel?
    private var eventSink: FlutterEventSink?
    private var session: FFmpegSession?

    
    public static func register(with registrar: FlutterPluginRegistrar) {
        let methodChannel = FlutterMethodChannel(name: "ffmpeg_cbs/method", binaryMessenger: registrar.messenger())
        let eventChannel = FlutterEventChannel(name: "ffmpeg_cbs/event", binaryMessenger: registrar.messenger())
        
        let instance = FFMpegCBSPlugin()
        instance.methodChannel = methodChannel
        
        registrar.addMethodCallDelegate(instance, channel: methodChannel)
        eventChannel.setStreamHandler(instance)

        print("[FFmpegCBS] Plugin registered successfully.")
    }

    // MARK: - Method Call Handling

    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        print("[FFmpegCBS] Received method call: \(call.method)")

        switch call.method {
        case "getPlatformVersion":
            let version = "iOS " + UIDevice.current.systemVersion
            print("[FFmpegCBS] Returning platform version: \(version)")
            result(version)

        case "startRecord":
            guard let args = call.arguments as? [String: Any],
                  let command = args["command"] as? String else {
                print("[FFmpegCBS] startRecord: Invalid arguments received.")
                result(FlutterError(code: "INVALID_ARGUMENT", message: "Missing command", details: nil))
                return
            }
            print("[FFmpegCBS] Starting recording with command:\n\(command)")
            startRecording(with: command)
            result(nil)

        case "stopRecord":
            print("[FFmpegCBS] Stopping recording session.")
            stopRecording()
            result(nil)

        default:
            print("[FFmpegCBS] Method not implemented: \(call.method)")
            result(FlutterMethodNotImplemented)
        }
    }

    // MARK: - FFmpeg Operations

    private func startRecording(with command: String) {
        print("[FFmpegCBS] FFmpeg session started")

        session = FFmpegKit.executeAsync(command, withCompleteCallback: self.handleFFmpegCompletion,
            withLogCallback: { log in
            if let log = log {
                print("[FFmpegCBS][Log] \(log.getSessionId()): lvl\(log.getLevel()) \(log.getMessage().description)")
            }
        }, withStatisticsCallback: { statistics in
            
        })
    }

    private func stopRecording() {
        print("[FFmpegCBS] Cancelling FFmpegKit session.")
        guard let session = self.session else {
            print("[FFmpegCBS] No active session to cancel.")
            return
        }
        session.cancel()
        FFmpegKit.cancel()
    }
    
    private func handleFFmpegCompletion(session: FFmpegSession?) {
        guard let session = session else {
            print("[FFmpegCBS] Session is nil")
            return
        }

        let returnCode = session.getReturnCode()
        let output = session.getOutput() ?? "(no output)"
        let failStackTrace = session.getFailStackTrace()

        // 시간 정보
        let startTime = session.getStartTime()
        let endTime = session.getEndTime()
        let durationSec = Double(session.getDuration()) / 1000.0

        print("[FFmpegCBS] Start time: \(startTime?.description ?? "nil") End time: \(endTime?.description ?? "nil")")
        print("[FFmpegCBS] Duration: \(durationSec) sec")

        print("[FFmpegCBS] FFmpeg session completed. ReturnCode: \(returnCode?.description ?? "Unknown")")
        print("[FFmpegCBS] Session output:\n\(output)")
        
        var message = [:] as [String: Any]

        if ReturnCode.isSuccess(returnCode) {
            print("[FFmpegCBS] Recording SUCCESS.")
            message = ["status": "completed",
                       "duration": durationSec]
        } else if ReturnCode.isCancel(returnCode) {
            print("[FFmpegCBS] Recording CANCELLED.")
            message = ["status": "cancelled",
                       "duration": durationSec]
        } else {
            print("[FFmpegCBS] Recording FAILED.")
            message["status"] = "error"
            message["error"] = failStackTrace ?? "Unknown error"
        }
        DispatchQueue.main.async {
            guard let sink = self.eventSink else {
                print("[FFmpegCBS] EventSink is nil, cannot send recording status.")
                return
            }
            sink(message)
       }
    }
    
    public func onListen(withArguments arguments: Any?, eventSink events: @escaping FlutterEventSink) -> FlutterError? {
        print("[FFmpegCBS] Event channel: onListen")
        self.eventSink = events
        return nil
    }

    public func onCancel(withArguments arguments: Any?) -> FlutterError? {
        print("[FFmpegCBS] Event channel: onCancel")
        self.eventSink = nil
        return nil
    }
}
