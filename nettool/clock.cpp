#include "Arduino.h"
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Time.h>
#include "config.h"
void
settim ()
{
  configTzTime ("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00",
		"pool.ntp.org");
  Serial.print ("Waiting for NTP time sync: ");
  time_t now = time (nullptr);
  for (short k=0;(now < 8 * 3600 * 2) && k<100;k++)
    {
      delay (100);
      Serial.print (".");
      now = time (nullptr);
    }
  Serial.println ();

  struct tm timeinfo;
  gmtime_r (&now, &timeinfo);
  Serial.printf ("%s %s", tzname[0], asctime (&timeinfo));
}
 
 /*
int 
gettim ( )
{
  tm rtcTime;
  char buf[80];
  unsigned long dif=0;
  getLocalTime (&rtcTime, 20);
  strftime (buf, sizeof (buf), "%H:%M", &rtcTime);	 
  display->fillScreen (0x0000);
  //display->setFont (&DSEG7_Classic_Regular_90); 
  display->setTextColor (OxFAAF, 0x00);
  int h, s, m, ph = 99, pm = 99;
  h=rtcTime.tm_hour;m=rtcTime.tm_min;
  for (s = 0; s < 3;)
    {
      if (ph != h || pm != m)
	{
	  sprintf (buf, "%02d:%02d", 0 % 24, 0 % 60);
	  display->fillRect (5, 65, 235, 170, 0x00);
	  sprintf (buf, "%02.2d:%02.2d", h % 24, m % 60);
	  display->drawString (buf, 5, 165, TFT_ORANGE);
	  ph = h;
	  pm = m;
	}
      if (touch (PinL))
	{
	  delay (500);
	  if (touch (PinL))
	    {
	      s == 0 ? h++ : s == 1 ? m++ : m;
	    }
	  else
	    {
	      if (s < 1)
		s++;
	      else
		break;
	    }
	}
      delay (50);
    }
  return((h%24)*60+(m%60));
}*/
void settim(int houmin)
  {ESP32Time rtc (0);		 
   rtc.setTime (0, houmin%60, houmin/60, 1, 1, 2025);
  }


char pbuf[20]=""; 
void
panelt (bool force = false)
{
  tm rtcTime;
  char buf[80];
  getLocalTime (&rtcTime, 20);
  strftime (buf, sizeof (buf), "%H:%M", &rtcTime);	// hh:mm:ss
   if (strcmp (buf, pbuf) || force)
    {
      strcpy (pbuf, buf);
      //psta = 888;
      display->fillScreen (0x0000);	// BLACK
      display->setFont (&Free7seg);	//FreeMonoBold12pt7b); // LovyanGFX Fonts
       display->setTextSize (1.0);	// text H pos
      display->setTextColor (RGB565_RED);
      display->setCursor( 5, 165);//, int16_t y)
      display->print (buf);
    }
  
}
extern void initWiFi();
 static bool atrest=false;   
void setupCLO (bool init)
{ if(init)
  {initWiFi();atrest=false;
    panelt();settim ();panelt();WiFi.disconnect(true,true);
  }
  else
  if(atrest)
  { display->displayOn();
    digitalWrite(GFX_BL,HIGH);
  }
} 
void butCLO(int i)
{{atrest=!atrest;
 if(atrest)
 {display->displayOff();
 }
 else
 {display->displayOn();
 }
 digitalWrite(GFX_BL, atrest?LOW:HIGH);
 }
 }
void
loopCLO ()
{
 if (digitalRead(0)==0)
 {atrest=!atrest;
 if(atrest)
 {display->displayOff();
 }
 else
 {display->displayOn();
 }
 digitalWrite(GFX_BL, atrest?LOW:HIGH);
 }
 else
 if(!atrest) panelt(false);
}