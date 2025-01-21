// bluetooth_scanner.h
#ifndef BLUETOOTH_SCANNER_H
#define BLUETOOTH_SCANNER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <vector>

struct BluetoothDevice {
    String name;
    String address;
    int rssi;
};

class BluetoothScanner {
public:
    BluetoothScanner();
    std::vector<BluetoothDevice> scanDevices(int scanTime = 5);
};

#endif
