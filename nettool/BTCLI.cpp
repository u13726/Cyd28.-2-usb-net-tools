// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Victor Tchistiak - 2019
//
// This example demonstrates master mode Bluetooth connection to a slave BT device using PIN (password)
// defined either by String "slaveName" by default "OBDII" or by MAC address
//
// This example creates a bridge between Serial and Classical Bluetooth (SPP)
// This is an extension of the SerialToSerialBT example by Evandro Copercini - 2018
//
// DO NOT try to connect to phone or laptop - they are master
// devices, same as the ESP using this code - it will NOT work!
//
// You can try to flash a second ESP32 with the example SerialToSerialBT - it should
// automatically pair with ESP32 running this code
#define SD_FAT_TYPE 3
#include "FS.h"
//#include "SD.h"
#include "SdFat.h"
#include "BluetoothSerial.h"
#include "TinyGPSPlus.h"
#define BACK_C 0xFFFFFF
#define TEXT_C 0X0000FF 
#include "config.h"
bool btlog=false;
// File system object.
#if SD_FAT_TYPE == 0
extern SdFat sd;
extern SdFile file;
#elif SD_FAT_TYPE == 1
extern SdFat32 sd;
extern File32 file;
#elif SD_FAT_TYPE == 2
extern SdExFat sd;
extern ExFile file;
#elif SD_FAT_TYPE == 3
extern SdFs sd;
extern FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

 TinyGPSPlus gps;
unsigned long last = 0UL;
bool cooked=true;

//#define USE_NAME // Comment this to use MAC address instead of a slaveName
const char *pin = "0000"; // Change this to reflect the pin expected by the real slave BT device
char keys[5][10]={"cook","record"};
void news()
{  short b=2;
 short bw=(320-(20*b))/b;
  display->fillScreen(0xFFFFFF);
 for(int i=0;i<b;i++)
{   display->fillRect(10+(20+bw)*i,210,bw-20,30,(i==0 && cooked)||(i==1 && btlog)?RGB565_RED:RGB565_GREEN);
    display->setCursor(10+(bw/3)+(20+bw)*i,230);
    display->print(keys[i]);
  }
}

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

#ifdef USE_NAME
  String slaveName = "HOLUX GPSlim236"; // Change this to reflect the real name of your slave BT device
#else
  String MACadd = "AA:BB:CC:11:22:33"; // This only for printing
  uint8_t address[6]  = {0x00, 0x0B, 0x0D, 0x14, 0x61, 0xD5}; // Change this to reflect real MAC address of your slave BT device
#endif

String myName = "ESP32-BT-Master";
int fnr=1;
void setupBTCLI(bool init) {
  bool connected;
  if (init)
  {display->fillScreen(BACK_C);
   display->setTextSize(1);
   display->setTextColor(0);
   display->setFont(&FreeMono8pt7b);
   display->setCursor(0,0);
   news();
  char buf[20];
  fnr=0;
    do
  {if(file.isOpen())file.close();
    fnr++;
    sprintf(buf,"/GPS%03d.txt",fnr);
  } while (file.open(buf, O_READ));
   SerialBT.begin(myName, true);
  Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", myName.c_str());
  SerialBT.unpairDevice(address);
  SerialBT.enableSSP();
  //delay(5000);
  #ifndef USE_NAME
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
#ifdef USE_NAME
  Serial.printf("Connecting to slave BT device named \"%s\"\n", slaveName.c_str());
   connected = SerialBT.connect(slaveName,1,18,ESP_SPP_ROLE_MASTER);
     //SerialBT.
  #else
    Serial.print("Connecting to slave BT device with MAC "); Serial.println(MACadd);
connected = SerialBT.connect(address,1,18,ESP_SPP_ROLE_MASTER);
      #endif
  if(connected) {
    Serial.println("Connected Successfully!");
  } else {
      for(int k=0;(k<5)&&(!SerialBT.connected(10000));k++) {delay(211);
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
  }
  else
  {SerialBT.disconnect();
   SerialBT.unpairDevice(address);
  SerialBT.end();
  }
  
}
char logchr(uint8_t c){
if(!btlog  || !file) return (char )c;
    if(file.print(c)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    //file.close();
  
 return(char) c;
}
void loopBTCLI() {String scr;
  /*if (Serial.available()) {
    SerialBT.write(Serial.read());
  }*/
  if (cooked)
  { String b="";
   unsigned char c;
   while (SerialBT.available() > 0)    
    {gps.encode(c=(SerialBT.read()));
     b+=(char)c;
    }
    if(b!=""&&btlog) file.print(b.c_str());
    static double plo=3.1;
   static double pla=9.3;
   
  if (gps.location.isUpdated()||plo!=gps.location.lng()||pla!=gps.location.lat())
  {display->setCursor(100,40);
   display->fillRect(100, 45, 130, -40, 0xFFFFFF);
   display->setFont(&FreeSerifBoldItalic12pt7b);
   display->setTextSize(1.5);
   display->print(pla=gps.location.lat(), 2);
   display->print(" ");
   display->print(plo=gps.location.lng(), 2);
  }
  if (gps.time.isUpdated())
  {display->setCursor(240, 10);
   display->setTextSize(1);
   display->setFont(&FreeMono8pt7b);
   display->fillRect(240, 20, 80, -20, 0xFFFFFF);
   display->print(gps.time.hour());
   display->print(":");
   display->print(gps.time.minute());
   display->print(":");
   display->print(gps.time.second());
   }
if (gps.satellites.isUpdated())
  {
   display->setCursor(20, 180);
   display->setTextSize(1);
   display->setFont(&FreeMono8pt7b);
   display->fillRect(20, 180, 80, -20, 0xFFFFFF);
   display->print(gps.satellites.value());
   display->print(" Sat");
   }
    else if (gps.speed.isUpdated())
  {display->setCursor(100,100);
   display->fillRect(100, 105, 130, -40, 0xFFFFFF);
   display->setFont(&FreeSerifBoldItalic12pt7b);
   display->setTextSize(1.5);
   display->print(gps.speed.kmph(), 2);
   display->print(" km/h");
    }
}
  else
  {scr="";
  char c;
   while (SerialBT.available()) {
    c=(char) (SerialBT.read());
    scr+=c;
    gps.encode(c);
   }
   display->print(scr);
  if(btlog) file.print(scr.c_str());
   if(display->getCursorY()>200) {news();display->setCursor(0, 0);}
 }
 // delay(20);
}

void clickBT(int bt)
{if (bt==0 )
 {cooked=! cooked;
news();
}
 else if (bt==1)
 { btlog=!btlog;
    if(btlog)
    {if(!file.isOpen())
     {char buf[20];
      sprintf(buf,"/GPS%03d.txt",fnr++);
      if (!file.open(buf, O_RDWR | O_CREAT)) 
      {Serial.printf("Failed to open file for writing %s %d",buf,file.getError());
      }
      else
      {Serial.printf("Log for writing %s\n",buf);
      }
     }
    }
    else
    {if(!file.close())
       Serial.println("close failed");
    }
 }
 news();
 }