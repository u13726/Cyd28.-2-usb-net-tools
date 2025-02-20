
#define BACK_C 0xFFFFFF
#define TEXT_C 0X0000FF 
#include <WiFi.h>
#include "config.h"
                              
void setup6(bool init) {
  display->fillScreen(BACK_C);
 display->setTextSize(1);
 display->setTextColor(0xFFFFFF);
 display->setFont(&FreeMono8pt7b);
  for(int i=0;i<4;i++)
  {for(int j=0;j<3;j++)
    {display->fillRoundRect(10+(i*80),10+(j*80),60,60,5,0x0909F0);
     display->setCursor(20+(i*80),40+(j*80));
     display->print(tasktab[i*3+j].nam);
    tasktab[i*3+j].x=35+(i*80);
     tasktab[i*3+j].y=40+(j*80);
     //display->fillCircle(tasktab[i*3+j].x, tasktab[i*3+j].y,3,0xFF0000);
    }
   }
 display->setTextColor(0);
  if (WiFi.status() == WL_CONNECTED) {
 display->setCursor(10, 240);
   display->println(WiFi.localIP());
 }
}

 void loop6() { 
   }
