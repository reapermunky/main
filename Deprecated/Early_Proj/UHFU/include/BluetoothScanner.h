#ifndef BLUETOOTH_SCANNER_H
#define BLUETOOTH_SCANNER_H

#include <vector>
#include "BluetoothDevice.h"

class BluetoothScanner {
public:
    std::vector<BluetoothDevice> scanDevices(int scanTime);
};

#endif // BLUETOOTH_SCANNER_H