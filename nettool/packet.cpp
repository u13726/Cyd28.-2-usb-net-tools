 #include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_private/wifi.h"
#include "lwip/err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_private/wifi.h"
#include "lwip/err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
 #include <stdio.h>
#include <string>
#include <cstddef>
#include <Wire.h>
#include <Preferences.h>
using namespace std;
#define MAX_CH 14       // 1 - 14 channels (1-11 for US, 1-13 for EU and 1-14 for Japan)
#define SNAP_LEN 2324   // max len of each recieved packet
#define MAXX 320  
#define MAXY 240
#define MAXYH 200
#include <XPT2046_Touchscreen.h>
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
 
 #if CONFIG_FREERTOS_UNICORE
#define RUNNING_CORE 0
#else
#define RUNNING_CORE 1
#endif

#include "config.h"

#include "FS.h"
#include "SD.h"
//#include "Buffer.h"

esp_err_t event_handler(void* ctx, system_event_t* event) {
  return ESP_OK;
}
bool useSD = false;
bool buttonPressed = false;
bool buttonEnabled = true;
uint32_t lastDrawTime;
uint32_t lastButtonTime;
uint32_t tmpPacketCounter;
uint32_t pkts[MAXX];       // here the packets per second will be saved
uint32_t deauths = 0;       // deauth frames per second
unsigned int ch = 6;        // current 802.11 channel
int rssiSum;
 
/* ===== functions ===== */
double getMultiplicator() {
  uint32_t maxVal = 1;
  for (int i = 0; i < MAXX; i++) {
    if (pkts[i] > maxVal) maxVal = pkts[i];
  }
  if (maxVal > MAXY) return (double)MAXY / (double)maxVal;
  else return (double)MAXY / (double)maxVal;
}     
 

void wifi_promiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  if (type == WIFI_PKT_MGMT && (pkt->payload[0] == 0xA0 || pkt->payload[0] == 0xC0 )) deauths++;
  if (type == WIFI_PKT_MISC) return;             // wrong packet type
  if (ctrl.sig_len > SNAP_LEN) return;           // packet too long
  uint32_t packetLength = ctrl.sig_len;
  if (type == WIFI_PKT_MGMT) packetLength -= 4;  // fix for known bug in the IDF https://github.com/espressif/esp-idf/issues/886

  //Serial.print(".");
 // if(first==0)
  {tmpPacketCounter++;
   rssiSum += ctrl.rssi;
  }
  //if (useSD) sdBuffer.addPacket(pkt->payload, packetLength);
}

void setChannel(int newChannel) {
  ch = newChannel;
  if (ch > MAX_CH || ch < 1) ch = 1;

  esp_wifi_set_promiscuous(false);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous);
  esp_wifi_set_promiscuous(true);
 for (int i = 0; i < MAXX; i++) {
    pkts[i] =0;
  }
}
void draw() {
   double multiplicator = getMultiplicator();
  int len;
  int rssi;
      pkts[MAXX - 1] = tmpPacketCounter;

  if (pkts[MAXX - 1] > 0) rssi = rssiSum / (int)pkts[MAXX - 1];
  else rssi = rssiSum;
 canvas->fillScreen(1) ;//clear();
canvas->setTextColor(0XAAAA);
canvas->setTextSize(1.5 );
  canvas->setCursor( 10, 0);canvas->print( (String)ch);
   canvas->setCursor( 80, 0);canvas->print( (String)rssi);
   canvas->setCursor( 164, 0);canvas->print( (String)tmpPacketCounter);
   canvas->setCursor(212, 0);canvas->print( (String)tasktab[0].timout);
     canvas->setCursor( 36,  0);canvas->print( ("Pkts:"));
   canvas->drawLine(0, MAXYH - MAXY, MAXX, MAXYH - MAXY,0XAAAA);
  for (int i = 0; i < MAXX; i++) {
    len = pkts[i] * multiplicator;
    canvas->drawLine(i, MAXYH,i,  MAXYH - (len > MAXY ? MAXY : len),0xAAAA);
    if (i < MAXX - 1) pkts[i] = pkts[i + 1];
  }
   
  canvas->fillRect(10,210,60,30,0xAAAA);
  canvas->fillRect(90,210,60,30,0xAAAA);
  canvas->fillRect(160,210,60,30,0xAAAA);
  canvas->fillRect(240,210,60,30,0xAAAA);
  canvas->flush();
  tmpPacketCounter=0;
  }
void setup0(bool init)
{
ch = 6;//preferences.getUInt("channel", 6);
    nvs_flash_init();
  tcpip_adapter_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  //ESP_ERROR_CHECK(esp_wifi_set_country(WIFI_COUNTRY_EU));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);

    esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous);
  esp_wifi_set_promiscuous(true);
  setChannel(6);
tmpPacketCounter = 0;
      pkts[MAXX - 1] = tmpPacketCounter;
      draw();
      tmpPacketCounter = 0;
      deauths = 0;
      rssiSum = 0;
}

void click0(int bx)
{if (bx==0) setChannel(ch-1);
 else if (bx==1) setChannel(ch+1);
 else if (bx==2) tasktab[0].timout=max(100,tasktab[0].timout-100); 
 else if (bx==3) tasktab[0].timout=min(2000,tasktab[0].timout+100);
 
 draw();
}