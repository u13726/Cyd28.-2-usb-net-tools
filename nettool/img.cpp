#define SD_FAT_TYPE 3
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
//#include "SD.h"
#include "SdFat.h"
#include "SPI.h"
#include "sd_fat32_fs_wrapper.h"
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

extern fs::FS sdfilesys;// = fs::FS(fs::FSImplPtr(new SdFat32FSImpl(sd))); 
#define BACK_C 0xFFFFFF
#define TEXT_C 0X0000FF 
#include "config.h"
  #include <SD.h>
 #include <PNGdec.h>
PNG png;
int16_t wi, he, xOffset, yOffset;
 File pngFile;
void *myOpen(const char *filename, int32_t *size)
{
 pngFile = sdfilesys.open(filename, "r");
  
  if (!pngFile || pngFile.isDirectory())
  {
    Serial.println(F("ERROR: Failed to open  file for reading"));
    display->println(F("ERROR: Failed to open  file for reading"));
  }
  else
  {
    *size = pngFile.size();
    Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
  return pngFile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
  return pngFile.seek(position);
}


  uint16_t usPixels[320];
  uint8_t usMask[320];
  // Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{

  //Serial.printf("Draw pos = 0,%d. size = %d x 1\n", pDraw->y, pDraw->iWidth);
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  display->draw16bitRGBBitmapWithMask(xOffset, yOffset + pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
}

void setup444(bool init)
{
   Serial.println("Arduino_GFX PNG Image Viewer example");
   if (!display->begin())
  {
    Serial.println("display->begin() failed!");
  }
  display->fillScreen(BLACK);

  wi = display->width(), he = display->height();
  display->fillScreen(BLACK);
  for (int16_t x = 0; x < wi; x += 5)
  {
    display->drawFastVLine(x, 0, he, PALERED);
  }

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

    
  }
int ipng=1;
void click444(int a)
{ipng+=100;
}
void loop444()
{    unsigned long start = millis();
    int rc;
    if (digitalRead(0)==0)
    ipng+=100;
    else
    ipng++;
    char buf[30];
    sprintf(buf,"/444/555/%d.png",ipng);
    rc = png.open(buf, myOpen, myClose, myRead, mySeek, PNGDraw);
    if (rc == PNG_SUCCESS)
    {
      int16_t pw = png.getWidth();
      int16_t ph = png.getHeight();
      display->fillScreen(0);

      xOffset = (wi - pw) / 2;
      yOffset = (he - ph) / 2;

      rc = png.decode(NULL, 0);

//      Serial.printf("Draw offset: (%d, %d), time used: %lu\n", xOffset, yOffset, millis() - start);
//      Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      png.close();
    }
}
