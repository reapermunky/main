import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'dart:convert';
import 'dart:async';

class BLEService {
  final FlutterBluePlus flutterBlue = FlutterBluePlus.instance;
  BluetoothDevice? connectedDevice;
  BluetoothCharacteristic? characteristic;

  final String SERVICE_UUID = "12345678-1234-1234-1234-123456789abc";
  final String CHARACTERISTIC_UUID = "abcd1234-5678-90ab-cdef-1234567890ab";

  Stream<Map<String, dynamic>>? dataStream;
  final StreamController<Map<String, dynamic>> _dataController = StreamController<Map<String, dynamic>>.broadcast();

  BLEService() {
    dataStream = _dataController.stream;
  }

  Future<void> connect() async {
    try {
      // Start scanning
      flutterBlue.startScan(timeout: Duration(seconds: 4));

      // Listen to scan results
      var subscription = flutterBlue.scanResults.listen((results) async {
        for (ScanResult r in results) {
          if (r.device.name == 'PacketPals_Device') {
            flutterBlue.stopScan();
            connectedDevice = r.device;
            await connectedDevice!.connect(autoConnect: false);

            List<BluetoothService> services = await connectedDevice!.discoverServices();
            for (BluetoothService service in services) {
              if (service.uuid.toString() == SERVICE_UUID) {
                for (BluetoothCharacteristic c in service.characteristics) {
                  if (c.uuid.toString() == CHARACTERISTIC_UUID) {
                    characteristic = c;
                    await c.setNotifyValue(true);
                    c.value.listen((value) {
                      String jsonString = utf8.decode(value);
                      try {
                        Map<String, dynamic> data = jsonDecode(jsonString);
                        _dataController.add(data);
                      } catch (e) {
                        print('Error decoding JSON: $e');
                      }
                    });
                  }
                }
              }
            }
          }
        }
      });

      // Optionally handle scan timeout or no devices found
    } catch (e) {
      print('Error connecting to ESP32: $e');
      // Handle connection errors, possibly retry or notify the user
    }
  }

  Future<void> disconnect() async {
    if (connectedDevice != null) {
      await connectedDevice!.disconnect();
      connectedDevice = null;
      characteristic = null;
    }
  }

  void dispose() {
    _dataController.close();
  }
}
