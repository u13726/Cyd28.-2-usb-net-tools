/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int sct = 5; //In seconds
BLEScan* pBLS;
#include "config.h"

char scr[800]="";
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //display->printf("Device: %s \n", advertisedDevice.toString().c_str());
      strcat(scr,advertisedDevice.toString().c_str());
      strcat(scr,"\n");
    }
};

void setupBTS(bool init) {
  if(init)
  {BLEDevice::init("");
  pBLS = BLEDevice::getScan(); //create new scan
  pBLS->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLS->setActiveScan(true); //active scan uses more power, but get results faster
  pBLS->setInterval(100);
  pBLS->setWindow(99);  // less or equal setInterval value
 display->fillScreen(0xFFFFFF);
 display->setTextSize(1);
 display->setTextColor(0);
 display->setFont(&FreeMono8pt7b);
 display->setCursor(10,10);
  }
  else
  {pBLS->stop();
  }
}

void loopBTS() {
  // put your main code here, to run repeatedly:
 strcpy(scr,"");
  BLEScanResults foundDevices = pBLS->start(sct, false);
  display->print("Devices found: ");
display->println(foundDevices.getCount());
  for(int i=0;i<1;i++)
    //display->println(foundDevices.dump());
    foundDevices.dump();
  Serial.println("Scan done!");
  pBLS->clearResults();   // delete results fromBLEScan buffer to release memory
 display->print(scr);
}