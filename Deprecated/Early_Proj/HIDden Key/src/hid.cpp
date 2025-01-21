#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_TinyUSB.h>

// Create HID object
Adafruit_USBD_HID usb_hid;

// Initialize HID functionality
void setupHID() {
    usb_hid.begin();
    Serial.println("HID initialized.");
}

// Send keystrokes using HID
void sendKeystrokes(const char *payload) {
    for (int i = 0; payload[i] != '\0'; i++) {
        usb_hid.keyboardPress(payload[i]);
        delay(50); // Simulate typing delay
        usb_hid.keyboardRelease();
    }
    Serial.println("Keystrokes sent.");
}
