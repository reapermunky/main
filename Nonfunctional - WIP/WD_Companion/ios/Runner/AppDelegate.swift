import UIKit
import Flutter

@UIApplicationMain
@objc class AppDelegate: FlutterAppDelegate {
    private let CHANNEL = "com.example.packet_pals/share"

    override func application(
        _ application: UIApplication,
        didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
    ) -> Bool {
        let controller = window?.rootViewController as! FlutterViewController
        let methodChannel = FlutterMethodChannel(name: CHANNEL, binaryMessenger: controller.binaryMessenger)

        methodChannel.setMethodCallHandler { [weak self] call, result in
            if call.method == "shareFile" {
                if let args = call.arguments as? [String: Any],
                   let filePath = args["filePath"] as? String {
                    self?.shareFile(filePath: filePath)
                    result(nil)
                } else {
                    result(FlutterError(code: "INVALID_ARGUMENT", message: "File path is null", details: nil))
                }
            } else {
                result(FlutterMethodNotImplemented)
            }
        }

        GeneratedPluginRegistrant.register(with: self)
        return super.application(application, didFinishLaunchingWithOptions: launchOptions)
    }

    private func shareFile(filePath: String) {
        let fileURL = URL(fileURLWithPath: filePath)
        let activityViewController = UIActivityViewController(activityItems: [fileURL], applicationActivities: nil)
        window?.rootViewController?.present(activityViewController, animated: true, completion: nil)
    }
}
