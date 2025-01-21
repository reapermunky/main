#include "BluetoothScanner.h"
#include "BLEDevice.h"
#include "MyAdvertisedDeviceCallbacks.h"

std::vector<BluetoothDevice> BluetoothScanner::scanDevices(int scanTime) {
    std::vector<BluetoothDevice> devices;
    BLEScan* scanner = BLEDevice::getScan();
    scanner->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(&devices));
    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(99);
    scanner->start(scanTime, false);
    scanner->clearResults();
    return devices;
}