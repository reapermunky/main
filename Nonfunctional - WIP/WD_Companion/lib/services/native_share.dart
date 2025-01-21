import 'package:flutter/services.dart';

class NativeShare {
  static const MethodChannel _channel = MethodChannel('com.example.packet_pals/share');

  static Future<void> shareFile(String filePath) async {
    try {
      await _channel.invokeMethod('shareFile', {'filePath': filePath});
    } on PlatformException catch (e) {
      print("Failed to share file: '${e.message}'.");
    }
  }
}
