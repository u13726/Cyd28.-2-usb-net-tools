#include <WiFi.h>
 
#include "esp_wifi.h"

void update_screen();   
void update_screen_text(String);
void add_meg_to_array(String msg);
 String maclist[64][3]; 
int listcount = 0;
#include <Arduino_GFX_Library.h>
 extern Arduino_GFX *display ;

String KnownMac[10][2] = {  // Put devices you want to be reconized
  {"Will-Phone","EC1F7ffffffD"},
  {"Will-PC","E894Fffffff3"},
  {"bbbo","96101B8C5B3C"},
  {"tobias","5C628B85BD5C"},
  {"NAME","MACADDRESS"},
  {"NAME","MACADDRESS"},
  {"NAME","MACADDRESS"},
  {"NAME","MACADDRESS"},
  {"NAME","MACADDRESS"}
  
};

String defaultTTL = "60"; // Maximum time (Apx seconds) elapsed before device is consirded offline

const wifi_promiscuous_filter_t filt={ //Idk what this does
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};

typedef struct { // or this
  uint8_t mac[6];
} __attribute__((packed)) MacAddr;

typedef struct { // still dont know much about this
  int16_t fctl;
  int16_t duration;
  MacAddr da;
  MacAddr sa;
  MacAddr bssid;
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;


  
#define maxCh 13 //max Channel -> US = 11, EU = 13, Japan = 14


int curChannel = 1;


void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) { //This is where packets end up after they get sniffed
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf; // Dont know what these 3 lines do
  int len = p->rx_ctrl.sig_len;
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  len -= sizeof(WifiMgmtHdr);
  if (len < 0){
    Serial.println("Receuved 0");
    return;
  }
  String packet;
  String mac;
  int fctl = ntohs(wh->fctl);
  for(int i=8;i<=8+6+1;i++){ // This reads the first couple of bytes of the packet. This is where you can read the whole packet replaceing the "8+6+1" with "p->rx_ctrl.sig_len"
     packet += String(p->payload[i],HEX);
  }
  for(int i=4;i<=15;i++){ // This removes the 'nibble' bits from the stat and end of the data we want. So we only get the mac address.
    mac += packet[i];
  }
  mac.toUpperCase();

  
  int added = 0;
  for(int i=0;i<=63;i++){ // checks if the MAC address has been added before
    if(mac == maclist[i][0]){
      maclist[i][1] = defaultTTL;
      if(maclist[i][2] == "OFFLINE"){
        maclist[i][2] = "0";
      }

      added = 1;
    }
  }
  
  if(added == 0){ // If its new. add it to the array.
    maclist[listcount][0] = mac;
    maclist[listcount][1] = defaultTTL;
    //Serial.println(mac);
    listcount ++;
    if(listcount >= 64){
      Serial.println("Too many addresses");
      listcount = 0;
    }
  }
}



//===== SETUP =====//
void setup4(bool init)
{if(init)
 {wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_filter(&filt);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
  display->fillScreen(0XFF);
  display->setCursor(140,200);
  display->setTextColor(0XFFFFFF);
  display->setTextSize(2);
  display->print("Scanning");
 }
 else
 {esp_wifi_stop();
 }
}

void purge(){ // This maanges the TTL
  for(int i=0;i<=63;i++){
    if(!(maclist[i][0] == "")){
      int ttl = (maclist[i][1].toInt());
      ttl --;
      if(ttl <= 0){
        //Serial.println("OFFLINE: " + maclist[i][0]);
        maclist[i][2] = "OFFLINE";
        maclist[i][1] = defaultTTL;
      }else{
        maclist[i][1] = String(ttl);
      }
    }
  }
}

void updatetime(){ // This updates the time the device has been online for
  for(int i=0;i<=63;i++){
    if(!(maclist[i][0] == "")){
      if(maclist[i][2] == "")maclist[i][2] = "0";
      if(!(maclist[i][2] == "OFFLINE")){
          int timehere = (maclist[i][2].toInt());
          timehere ++;
          maclist[i][2] = String(timehere);
}

//Serial.println(maclist[i][0] + " : " + maclist[i][2]);

}
}
}
String forScreen = "";

void showpeople(){ // This checks if the MAC is in the reckonized list and then displays it on the OLED and/or prints it to serial.
forScreen = "";
  for(int i=0;i<=63;i++){
    String tmp1 = maclist[i][0];
    if(!(tmp1 == "")){
      for(int j=0;j<=9;j++){
        String tmp2 = KnownMac[j][1];
        if(tmp1 == tmp2){
         if((forScreen.indexOf(maclist[i][0]))<0)
            forScreen += (maclist[i][0] + "   " +maclist[i][1]+ "   " +maclist[i][2]+ "\n");
        Serial.print(KnownMac[j][0] + " : " + tmp1 + " : " + maclist[i][2] + "\n -- \n");
        }
        else
        {// forScreen += (KnownMac[j][0] + " : " + maclist[i][2] + "\n");
         Serial.print(KnownMac[j][0] + " : " + tmp1 + " : " + maclist[i][2] + "\n -- \n");
        }
      }
       if((forScreen.indexOf(maclist[i][0] ))<0)
         forScreen += (maclist[i][0] + "   " +maclist[i][1]+ "   " +maclist[i][2]+ "\n");
         
    }
  }
  update_screen_text(forScreen);
}

//===== LOOP =====//
void loop4() {
    //Serial.println("Changed channel:" + String(curChannel));
    if(curChannel > maxCh){ 
      curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(100);
    updatetime();
    purge();
    showpeople();
    curChannel++;
}
 
 const int available_lines = 8;
String textarray[available_lines];
int char_per_line = 21;

void setupOLED(){
   display->setTextSize(1);
  display->setTextColor(WHITE);
  display->setCursor(0,0);
}

void update_screen(){
  display->fillScreen(0xFF);
  display->setCursor(0,0);
  for(int i = 1; i <= 8 ; i++){
    display->println(textarray[i-1]);
  }
}

void update_screen_text(String msg){
  display->fillScreen(0xFFFFFF);
  display->setCursor(0,0);
  display->setTextSize(1);
  display->setTextColor(0xFF);
  display->println(msg);
}
