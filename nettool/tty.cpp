//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#include "config.h"
extern BluetoothSerial SerialBT;

void setupTTY(bool init) {
  if(init)
  {SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
 display->fillScreen(0xFFFFFF);
 display->setTextSize(1);
 display->setTextColor(0);
 display->setFont(&FreeMono8pt7b);
 display->setCursor(0,10);
  }
  else
  {SerialBT.end();
  }
}

void loopTTY() {
char t;
 while(Serial.available()||SerialBT.available()){
  if (Serial.available()) {
    SerialBT.write(t=Serial.read());
  display->print(t);}
  if (SerialBT.available()) {
    Serial.write(t=SerialBT.read());
    display->print(t);
  }
 }
}