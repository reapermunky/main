#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <vector>
#include <string>

struct BluetoothDevice {
    std::string name;
    std::string address;
    int rssi;
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        BluetoothDevice device;
        device.name = advertisedDevice.getName().c_str();
        device.address = advertisedDevice.getAddress().toString().c_str();
        device.rssi = advertisedDevice.getRSSI();
    }
};

class BluetoothScanner {
public:
    BluetoothScanner() {
        BLEDevice::init("");
    }

    std::vector<BluetoothDevice> scanDevices(int scanTime) {
        BLEScan* scanner = BLEDevice::getScan();
        scanner->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        scanner->setActiveScan(true);
        BLEScanResults scanResults = scanner->start(scanTime, false);
        // Process scan results and return as needed
        std::vector<BluetoothDevice> devices;
        return devices;
    }
};
