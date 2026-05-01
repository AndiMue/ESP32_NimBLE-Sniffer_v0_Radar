#include <Arduino.h>
#include <NimBLEDevice.h>
#include <map>

struct DeviceInfo {
  int count = 0;
  int rssiSum = 0;
  String name = "";
};

std::map<std::string, DeviceInfo> devices;

unsigned long lastPrint = 0;

// --- Callback ---
class MyCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    std::string addr = advertisedDevice->getAddress().toString();
    int rssi = advertisedDevice->getRSSI();
    String name = advertisedDevice->getName().c_str();

    auto& d = devices[addr];
    d.count++;
    d.rssiSum += rssi;

    if (name.length() > 0) {
      d.name = name;
    }
  }
};

void printRadar() {
  Serial.println("\n---- BLE RADAR ----");

  for (auto& it : devices) {
    auto& addr = it.first;
    auto& d = it.second;

    int avg = d.count ? d.rssiSum / d.count : 0;

    Serial.printf("%s | RSSI avg: %d | seen: %d | name: %s\n",
                  addr.c_str(),
                  avg,
                  d.count,
                  d.name.length() ? d.name.c_str() : "-");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("BLE Radar starting...");

  NimBLEDevice::init("");

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(new MyCallbacks(), true);

  // 🔥 aggressive scan
  scan->setActiveScan(true);
  scan->setInterval(160);
  scan->setWindow(160);
  scan->setDuplicateFilter(false);

  scan->start(0, false); // Dauer-Scan
}

void loop() {
  if (millis() - lastPrint > 5000) { // alle 5 Sekunden
    printRadar();
    lastPrint = millis();
  }
}
