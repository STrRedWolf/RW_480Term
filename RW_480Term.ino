/***************************************************
  RedWolf 480px serial write-only terminal
  by Kelly Price
  MIT License

  Uses VT52 terminal escapes.
  Needs a Feather M0 (uses 8K of RAM!)
  Uses Adafruit's built-in font (it actually has all the chars from
    Window's 6x8 terminal font!)

  Based on the below from Adafruit:
  
  This is our library for the Adafruit 3.5" TFT (HX8357) FeatherWing
  ----> http://www.adafruit.com/products/3651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include "RW_480term.h"

#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif
#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif
#ifdef __AVR_ATmega32U4__
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif
#ifdef ARDUINO_SAMD_FEATHER_M0
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif
#ifdef TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#endif
#ifdef ARDUINO_FEATHER52
   #define STMPE_CS 30
   #define TFT_CS   13
   #define TFT_DC   11
   #define SD_CS    27
#endif

#define TFT_RST -1

// Use hardware SPI and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// Globals we will need
uint16_t cx,cy,fg,bg,mode,tmp;
char scr[2400];
char colors[2400];

void setup() {
  int i;
  Serial.begin(115200);
  //Serial.println("OK"); 
  
  tft.begin(HX8357D);
  tft.setRotation(1);
  
  for(i=0;i<2400;i++) {
    scr[i]=32;
    colors[i]=0x70;
  }
  repaintScreen();
  fg=7;
  cx=cy=0;
  bg=0;
  mode=0;
  //while(!Serial.available());
  
  Serial.println("OK"); 

}

void loop(void) {
  char inchar;

  if(Serial.available() > 0) {
    inchar = (char)Serial.read();

    switch(mode) {
      case 0:
        procNormal(inchar);
        break;
      case 1:
        procEscape(inchar);
        break;
      case 2: // Y first char
        if(inchar<32)
          mode=255;
        else {
          tmp=(int)inchar-32;
          mode++;
        }
        break;
      case 3: // Y second char
        if(inchar>31) {
          cy=tmp;
          cx=(int)inchar - 32;
        }
        mode=0;
        break;
      case 4: // b char
        if(inchar>31 && inchar<48)
          fg=(int)inchar-32;
        mode=0;
        break;
      case 5: // c char
        if(inchar>31 && inchar<48)
          bg=(int)inchar-32;
        mode=0;
        break;
      default:
        mode=0;
        break;
    }    
  }
  yield();
}

void plopChar(uint16_t x,uint16_t y, uint16_t f, uint16_t b, char c) {
  //Serial.print("Drawing '");
  //Serial.print((int)c);
  //Serial.print("' at ");
  //Serial.print(x);
  //Serial.print(",");
  //Serial.println(y);
  if(f==0 && b==0) f=7;
  tft.drawChar(x*6,y*8,c,f,b,1);  
}

void repaintScreen() {
  int x,y;
  for(y=0;y<NUMROWS;y++) {
    for(x=0;x<NUMCOLS; x++) {
      redrawChar(x,y);
    }
  }
}

void redrawChar(uint16_t x,uint16_t y) {
  int i,f,b;
  char ch,cl;
  
  i=(int)(y*80+x);
  //Serial.println(i);
  //Serial.println((int)scr[i]);
  ch=scr[i];
  cl=colors[i];
  f=cl>>4;
  b=cl & 15;
  //Serial.println(f);
  //Serial.println(b);
  plopChar(x,y,cpal[f],cpal[b],ch);
  yield();
  
}

void procNormal(char in) {
  int i,p;
  switch(in) {
    case 13:  // CR
      cx=0;
      break;
    case 10: // LF
      cy++;
      break;
    case 8:  //backspace 
      if(cx>0) {
        p=cy*NUMCOLS;
        for(i=cx;i<NUMCOLS;i++) {
          scr[p+i-1]=scr[p+i];
          colors[p+i-1]=colors[p+i-1];
          redrawChar(i-1,cy);
        }
        scr[p+MAXCOLS]=32;
        colors[p+MAXCOLS]=(char)((fg<<4)+bg);
        redrawChar(MAXCOLS,cy);
        cx--;     
      }
      
      break;
    case 9:  // tab
      cx=(cx+8) & 0xfff8;
      break;
    case 27: // ESC!
      mode=1;
      break;
    default:
      printChar(in);
      break;
  }
}

void printChar(char in) {
  int i;

  scr[cx+cy*NUMCOLS]=in;
  colors[cx+cy*NUMCOLS]=(char)((fg<<4) + bg);
  plopChar(cx++,cy,cpal[fg],cpal[bg],in);

  // Fix the cursor if it's off screen.
  if(cx > MAXCOLS) {
    cx=0;
    cy++;
  }
  if(cy > MAXROWS) {
    // Ugh. scroll the screen up.
    for(i=80;i<2400;i++) {
      scr[i-80]=scr[i];
      colors[i-80]=colors[i];
    }
    for(i=2320;i<2400;i++){
      scr[i]=32;
      colors[i]=0;
    }

    repaintScreen();
  }
}

void procEscape(char in) {
  int i,p;
  mode=0;
  switch(in) {
    case 'A':
      if(cy>0)
        cy--;
      break;
    case 'B':
      if(cy<MAXROWS)
        cy++;
      break;
    case 'C':
      if(cx<MAXCOLS)
        cx++;
      break;
    case 'D':
      if(cx>0)
        cx--;
      break;
    case 'E':
      for(i=0;i<2400;i++) {
        scr[i]=32;
        colors[i]=0;
      }
      repaintScreen();
      cx=cy=0;
      break;
    case 'H':
      cx=cy=0;
      break;
    case 'I':
      if(cy==0) {
        // Ugh, scroll down.
        for(i=0;i<2320;i++) {
          scr[2399-i]=scr[2319-i];
          colors[2399-i]=colors[2319-i];
        }
        for(i=0;i<80;i++){
          scr[i]=32;
          colors[i]=0;
        }
    
        repaintScreen();
      } else {
        cy--;
      }
      break;
    case 'J':
      for(i=(cx+cy*80);i<2400;i++) {
        scr[i]=32;
        colors[i]=0;
      }
      repaintScreen();
      break;
    case 'K':
      p=cy*NUMCOLS;
      for(i=cx;i<NUMCOLS;i++)  {
        scr[i]=32;
        colors[i]=0;
        redrawChar(i,cy);
      }
      break;
    case 'L':
      if(cy<MAXROWS) {
        for(i=cy*80;i<2320;i++) {
          scr[2399-i]=scr[2319-i];
          colors[2399-i]=colors[2319-i];
        }
      }
      for(i=0;i<80;i++){
        scr[cy*80+i]=32;
        colors[cy*80+i]=0;
      }
  
      repaintScreen();
      break;
    case 'M':
      if(cy<MAXROWS) {
        for(i=(cy+1)*80;i<2400;i++) {
          scr[i-80]=scr[i];
          colors[i-80]=colors[i];
        }
      }
      for(i=2320;i<2400;i++){
        scr[i]=32;
        colors[i]=0;
      }
  
      repaintScreen();
      break;
    case 'Y':
      mode=2;
      break;
    case 'Z':
      Serial.print((char)27);
      Serial.print("/K");
      break;
    case 'b':
      mode=4;
      break;
    case 'c':
      mode=5;
      break;

    // special commands
    case 'R':
      Serial.print((char)27);
      Serial.print("/R");
      break;

    // Default is to print the char
    default:
      printChar(in);
      break;      
  }
}

