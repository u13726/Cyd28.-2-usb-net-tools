
#include "config.h"// Library for XPT2046 touch controller
 
// Keypad start position, key size, and spacing
#define KEY_X 20
#define KEY_Y 70
#define KEY_W 45
#define KEY_H 40
#define KEY_SPACING_X 10
#define KEY_SPACING_Y 10
#define KEY_TEXTSIZE 2
// Numeric display box size and location
#define DISP_X 10
#define DISP_Y 10
#define DISP_W 220
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing input
#define NUM_LEN 32
char numberBuffer[NUM_LEN + 1] = ""; // Zero terminate

// We have 5 rows of 4 keys
char keyLabel[5][4] = {
  {'7','8','9','/'},
  {'4','5','6','*'},
  {'1','2','3','-'},
  {'0','.','%','+'},
  {'C','(',')','='}
};

void drawKeypad() {
  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 4; col++) {
      // Draw the key outline
      display->drawRoundRect(KEY_X + col * (KEY_W + KEY_SPACING_X), KEY_Y + row * (KEY_H + KEY_SPACING_Y), KEY_W, KEY_H,5,0xff);
      // Draw the key label
      display->setTextColor(TFT_WHITE);
     // display->setFreeFont(LABEL2_FONT);
     display->setFont(&FreeMono8pt7b);
      //display ->drawCentreString(String(keyLabel[row][col]), KEY_X + col * (KEY_W + KEY_SPACING_X) + KEY_W / 2, KEY_Y + row * (KEY_H + KEY_SPACING_Y) + 10, 2);
    display ->setCursor( -5+KEY_X + col * (KEY_W + KEY_SPACING_X) + KEY_W / 2, 10+KEY_Y + row * (KEY_H + KEY_SPACING_Y) + 10);
    display ->print(String(keyLabel[row][col]));//, KEY_X + col * (KEY_W + KEY_SPACING_X) + KEY_W / 2, KEY_Y + row * (KEY_H + KEY_SPACING_Y) + 10, 2);
    }
  }
}
void setupCAL(bool init) {
if (init)
{display->setRotation(0); // Set display to portrait mode
 display->fillScreen(TFT_BLACK);
 display->fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, 0xFFFFFF);
 drawKeypad();
 ts.setRotation(0);
 numberBuffer[0] = '\0';
}
 else
  {display->setRotation(3);
   ts.setRotation(3);
   }
}

float evaluateExpression(String expr) {
  float result = 0;
  float num = 0;
  char lastOperator = '+';
  String number = "";

  for (int i = 0; i < expr.length(); i++) {
    char c = expr.charAt(i);

    if (isdigit(c) || c == '.') {
      number += c; // Build the number string
    } else {
      num = number.toFloat(); // Convert the string to a float
      number = ""; // Reset the number string

      switch (lastOperator) {
        case '+': result += num; break;
        case '-': result -= num; break;
        case '*': result *= num; break;
        case '/': result /= num; break;
        case '%': result = (int)result % (int)num; break;
      }

      lastOperator = c; // Update the operator
    }
  }

  // Apply the last pending operation
  num = number.toFloat(); // Convert the last number
  switch (lastOperator) {
    case '+': result += num; break;
    case '-': result -= num; break;
    case '*': result *= num; break;
    case '/': result /= num; break;
    case '%': result = (int)result % (int)num; break;
  }

  return result;
}

void loopCAL() {
  for(;;)
  {if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Map from touch coordinates to screen coordinates
    p.x = map(p.x, 200, 3700, 0, 240); // Map for portrait mode
    p.y = map(p.y, 240, 3800, 0, 320); // Map for portrait mode
    if(p.y<50)
      {return;
      }
    // Check which key is pressed
    for (uint8_t row = 0; row < 5; row++) {
      for (uint8_t col = 0; col < 4; col++) {
        if ((p.x > KEY_X + col * (KEY_W + KEY_SPACING_X)) && (p.x < KEY_X + col * (KEY_W + KEY_SPACING_X) + KEY_W) &&
            (p.y > KEY_Y + row * (KEY_H + KEY_SPACING_Y)) && (p.y < KEY_Y + row * (KEY_H + KEY_SPACING_Y) + KEY_H)) {
          char key = keyLabel[row][col];
          if (key == 'C') {
            // Clear the number buffer
            numberBuffer[0] = '\0';
          } else if (key == '=') {
            // Evaluate the expression and display the result
            float result = evaluateExpression(String(numberBuffer));
            snprintf(numberBuffer, sizeof(numberBuffer), "%.2f", result); // Show result with 2 decimal places
          } else if (strlen(numberBuffer) < NUM_LEN) {
            // Add the key to the number buffer
            numberBuffer[strlen(numberBuffer) + 1] = '\0';
            numberBuffer[strlen(numberBuffer)] = key;
          }
          // Update the display
          display->fillRect(DISP_X + 2, DISP_Y + 2, DISP_W - 4, DISP_H - 4, 0x00);
          display->setTextColor(0Xff0f);
          //display->drawCentreString(numberBuffer, DISP_X + DISP_W / 2, DISP_Y + 10, 4);
          display->setCursor(DISP_X +10/*+ DISP_W / 2*/, DISP_Y + 25);
          display->print(numberBuffer);//, DISP_X + DISP_W / 2, DISP_Y + 10, 4);
          delay(300); // Debounce delay
        }
      }
    }
  }
  }
}