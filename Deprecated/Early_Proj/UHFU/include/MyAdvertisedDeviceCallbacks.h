#ifndef MY_ADVERTISED_DEVICE_CALLBACKS_H
#define MY_ADVERTISED_DEVICE_CALLBACKS_H

#include <vector>
#include "BluetoothDevice.h"
#include "BLEAdvertisedDevice.h"

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
    MyAdvertisedDeviceCallbacks(std::vector<BluetoothDevice>* devices) : devices(devices) {}
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        // Add the advertised device to the devices vector
    }
private:
    std::vector<BluetoothDevice>* devices;
};

#endif // MY_ADVERTISED_DEVICE_CALLBACKS_H