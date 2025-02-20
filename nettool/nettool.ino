 #include "freertos/FreeRTOS.h"
# include <stdio.h>
#include <string>
#include <cstddef>
#include <Preferences.h>
using namespace std;
#define TOUCH_CS 33     // Chi

#define MAXX 320  
#define MAXY 240
#define MAXYH 200
#include <XPT2046_Touchscreen.h>
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

 #if CONFIG_FREERTOS_UNICORE
#define RUNNING_CORE 0
#else
#define RUNNING_CORE 1
#endif
#include "config.h"
 
#include <Arduino_GFX_Library.h>
 
Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, 12 /* MISO */, HSPI /* spi_num */);
Arduino_GFX *display = new Arduino_ST7789(bus, GFX_NOT_DEFINED /* RST */, 3 /* rotation */);
Arduino_GFX  *canvas = new Arduino_Canvas_Mono(320 /* width */, 240  /* height */, display,0,0);

short first=11;
short pfirst=11;
Task tasktab[12]={
  {"pack",0,0,2000,0,setup0,draw,click0,4},
  {"chan",0,0,3500,0,setup2,loop2,NULL,0},
  {"ssid",0,0,2000,0,setup3,loop3,NULL,0},
  {"MAC",0,0,2000,0,setup4,loop4,NULL,0},
  {"IP",0,0,100,0,setupIPS,loopIPS,NULL,0}, 
  {"web",0,0,10,0,setupWeb,loopWeb,NULL,0},
  {"GPS",0,0,20,0,setupBTCLI,loopBTCLI,clickBT,2},
  {"SYS",0,0,2000,0,setupSYS,loopSYS,NULL,0},
  {"BTS",0,0,2000,0,setupBTS,loopBTS,NULL,0},
  {"NTP",0,0,2000,0,setupCLO,loopCLO,butCLO,1},
  {"Calc",0,0,20,0,setupCAL,loopCAL,NULL,0},
  {"Menu",0,0,2000,0,setup6,loop6,NULL,0}
};
Preferences preferences;
short menucnt=0;
 void setup() {
  Serial.begin(115200);
  setupSD();
  preferences.begin("NetTool", false);
  canvas->begin();//init();
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#ifdef FLIP_display->
  display->flipScreenVertically();
#endif
  canvas->setTextSize(1);
  canvas->fillScreen(RGB565_BLACK);
  canvas->setTextColor(RGB565_RED);
  canvas->setCursor(0, 0);
  canvas->print("ESP");
  canvas->setTextColor(RGB565_WHITE);
  canvas->print(" Network tools");
  canvas->flush();
  delay(1000);
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(3);
  xTaskCreatePinnedToCore(
    coreTask,               /* Function to implement the task */
    "coreTask",             /* Name of the task */
    2500,                   /* Stack size in words */
    NULL,                   /* Task input parameter */
    0,                      /* Priority of the task */
    NULL,                   /* Task handle. */
    RUNNING_CORE);          /* Core where the task should run */

    first=preferences.getInt("action",11);
    if(first<0 || first>11) first=11;
    tasktab[first].setupt(true);
  }

void loop() {
  vTaskDelay(portMAX_DELAY);
}
const uint16_t touchScreenMinimumX = 300, touchScreenMaximumX = 3665, touchScreenMinimumY = 367,touchScreenMaximumY = 3550;
int tto=0;
void coreTask( void * p ) {
  uint32_t currentTime;
  static unsigned long ttime;
  while (true) {
    currentTime = millis();
    if ((millis()-tasktab[first].sta)>tasktab[first].timout)
    {tasktab[first].loopt();
     delay(5);
     yield();
     tasktab[first].sta=millis();
     tto++;
     if(tto==3)preferences.putInt("action",11);
     continue;
    }
    if (ts.tirqTouched() && ts.touched()) 
    {if((millis()-ttime)<300){delay(100);continue;}
     ttime=millis();
     TS_Point p = ts.getPoint();
    // Serial.printf("%d %d\n",p.x,p.y);
    int x = map(p.x,touchScreenMinimumX,touchScreenMaximumX,0,320);
    int y = map(p.y,touchScreenMinimumY,touchScreenMaximumY,0,240);
//    Serial.printf("%d %d\n",x,y);
    if(first==11)
    {int d=9999;int in=11;
     for(int i=0;i<12;i++)
     if ((abs(tasktab[i].x-x) +abs(tasktab[i].y-y))<d)
     {d=abs(tasktab[i].x-x) +abs(tasktab[i].y-y);
      in=i;
     }
     if(in==11) {if(menucnt++==5) {first=7;tasktab[9].loopt=loop444;tasktab[9].click=click444;tasktab[9].setupt=setup444;strcpy(tasktab[9].nam,"PIC");} else
                                  {tasktab[9].loopt=loopCLO;tasktab[9].setupt=setupCLO;tasktab[9].click=butCLO;strcpy(tasktab[9].nam,"NTP");}}else menucnt=0;
     pfirst=first;
     first=in;
    }
    else if (tasktab[first].buttons && (y>200))
    {int bx=x/(320/tasktab[first].buttons);
     Serial.println((320/tasktab[first].buttons));
     Serial.println(bx);
     tasktab[first].click(bx);
     continue;
    }
    else
      {pfirst=first;
       first=11;
      }
    if(pfirst!=first )
    {tasktab[pfirst].setupt(false);
     tto=0;preferences.putInt("action",first);
     tasktab[first].setupt(true);
    }
    }
  }
}
