#ifndef GLobal
#define Global 9
#include <Preferences.h>
// Display
#include <Arduino_GFX_Library.h>
#define GFX_BL 21
extern Arduino_DataBus *bus;
extern Arduino_GFX *display;
extern Arduino_GFX *canvas;
#include "font.h"
#include <XPT2046_Touchscreen.h> // Library for XPT2046 touch controller
extern  XPT2046_Touchscreen ts;
// menu & task
extern void setup0(bool);
extern void setupSD();
extern void draw();
extern void setup2(bool);
extern void loop2();
extern void click0(int);
extern void clickBT(int);
extern void setup4(bool);
extern void loop4();
extern void setup6(bool);
extern void loop6();
extern void setup7(bool);
extern void loop7();
extern void loop3();
extern void setup3(bool);
extern void loopIPS();
extern void setupIPS(bool);
extern void loopBTS();
extern void setupBTS(bool);
extern void loop444();
extern void setup444(bool);
extern void loopTTY();
extern void setupTTY(bool);
extern void loopSYS();
extern void setupSYS(bool);
extern void loopBTCLI();
extern void setupBTCLI(bool);
extern void loopCLO();
extern void setupCLO(bool);
extern void loopCAL();
extern void setupCAL(bool);
extern void butCLO(int);
extern void loopWeb();
extern void setupWeb(bool);
extern void click0(int);
extern void click444(int);

typedef struct
{
  char nam[5];
  int x, y;
  int timout;
  unsigned long sta;
  void (*setupt)(bool);
  void (*loopt)();
  void (*click)(int);
  short buttons;
} Task;
#define TFT_WHITE 0xFFFFFF
#define TFT_BLACK 0x00
extern Task tasktab[];
extern Preferences preferences;

#endif