#define SD_FAT_TYPE 3
 
#include "SdFat.h"
//#if SPI_DRIVER_SELECT == 2  // Must be set in SdFat/SdFatConfig.h

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
 //
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 5;
//
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 19;
const uint8_t SOFT_MOSI_PIN = 23;
const uint8_t SOFT_SCK_PIN = 18;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI


// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#define SS 5
#ifndef SDCARD_SS_PIN
//const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN
 

#define SD_FAT_TYPE 3
#if SD_FAT_TYPE == 0
SdFat sd;
SdFile file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

bool setupSD() {

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }

 char buf[20];
////// sprintf(buf,"GPS%03d.txt",random(0,999));
  if (!file.open("/info.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }

  Serial.println(F("Done."));
    file.println("azazaz");
   file.close();
  return true;
}
 