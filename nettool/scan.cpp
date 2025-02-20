#include <WiFi.h>

#include "config.h"

void setup3(bool init)
{if(init)
 {display->fillScreen(0xFFFFFF);
  display->setTextColor(0XFF);
  display->setTextSize(2);
  display->setCursor(160, 200);
  display->println("Scanning");
 }
 else
 {}
}
void loop3(){
  //display->fillScreen(2) ;//clear();
   display->setTextColor(0X00FF);
    display->setTextSize(2);
   display->setCursor(160, 200);
   display->println("Scanning");
  
 int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("no networks found");
    } else {
  display->fillScreen(0xFFFFFF) ;//clear();
   display->setTextColor(0XFF);
    display->setTextSize(1);
         display->setCursor(0,10);
         display->println(" SSID               |RSSI|CH|Sec");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            //display->printf("%2d",i + 1);
            //display->print(" | ");
            display->printf(" %-19.19s", WiFi.SSID(i).c_str());
            display->print("|");
            display->printf("%3d", WiFi.RSSI(i));
            display->print("|");
            display->printf("%2d", WiFi.channel(i));
            display->print("|");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                display->print("N/A");
                break;
            case WIFI_AUTH_WEP:
                display->print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                display->print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                display->print("WP2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                display->print("A+2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                display->print("EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                display->print("WP3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                display->print("2+3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                display->print("WPI");
                break;
            default:
                display->print("???");
            }
             display->println();
            delay(10);
        }
    }
    Serial.println("");
 
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
 }