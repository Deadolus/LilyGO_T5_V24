// GxEPD_Example : test example for e-Paper displays from Waveshare and from Dalian Good Display Inc.
//
// Created by Jean-Marc Zingg based on demo code from Good Display,
// available on http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// The e-paper displays are available from:
//
// https://www.aliexpress.com/store/product/Wholesale-1-54inch-E-Ink-display-module-with-embedded-controller-200x200-Communicate-via-SPI-interface-Supports/216233_32824535312.html
//
// http://www.buy-lcd.com/index.php?route=product/product&path=2897_8363&product_id=35120
// or https://www.aliexpress.com/store/product/E001-1-54-inch-partial-refresh-Small-size-dot-matrix-e-paper-display/600281_32815089163.html
//

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare SPI e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

// mapping suggestion for Arduino MEGA
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 53, CLK -> 52, DIN -> 51

// mapping suggestion for Arduino DUE
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 77, CLK -> 76, DIN -> 75
// SPI pins are also on 6 pin 2x3 SPI header

// include library, include base class, make path known
#include <GxEPD.h>
#include "SD.h"
#include "SPI.h"
#include <Button2.h>
#include <Ticker.h>

// select the display class to use, only one
// #include <GxGDEP015OC1/GxGDEP015OC1.h>    // 1.54" b/w
// #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
// #include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
// #include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w
// #include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r
// #include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
// #include <GxGDEW029T5/GxGDEW029T5.h>      // 2.9" b/w IL0373
// #include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
// #include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r
// #include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w
//#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w
//#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
//#include <GxGDEW0583T7/GxGDEW0583T7.h>    // 5.83" b/w
//#include <GxGDEW075T8/GxGDEW075T8.h>      // 7.5" b/w
//#include <GxGDEW075Z09/GxGDEW075Z09.h>    // 7.5" b/w/r

#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17
#define ELINK_SS 5

#define SPI_MOSI 23
#define SPI_MISO -1 //elink no use
#define SPI_CLK 18

#define SDCARD_SS 13
#define SDCARD_MOSI 15
#define SDCARD_MISO 2
#define SDCARD_CLK 14

#define BUTTON_1 37
#define BUTTON_2 38
#define BUTTON_3 39

#define SPEAKER_OUT 25
#define AMP_POWER_CTRL 19
#define CHANNEL_0 0
#define BUTTONS_MAP \
    {               \
        37, 38, 39  \
    }

Button2 *pBtns = nullptr;
uint8_t g_btns[] = BUTTONS_MAP;

GxIO_Class io(SPI, /*CS=5*/ ELINK_SS, /*DC=*/ ELINK_DC, /*RST=*/ ELINK_RESET); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ ELINK_RESET, /*BUSY=*/ ELINK_BUSY); // arbitrary selection of (16), 4
Ticker btnTicker;



void button_callback(Button2 &b)
{
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        if (pBtns[i] == b) {
            Serial.printf("Button: %u Press\n", pBtns[i].getAttachPin());
            ledcWriteTone(CHANNEL_0, 1000);
            delay(200);
            ledcWriteTone(CHANNEL_0, 0);
        }
    }
}

void button_init()
{
    uint8_t args = sizeof(g_btns) / sizeof(g_btns[0]);
    pBtns = new Button2[args];
    for (int i = 0; i < args; ++i) {
        pBtns[i] = Button2(g_btns[i]);
        pBtns[i].setPressedHandler(button_callback);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
    display.init(); // enable diagnostic output on Serial


    pinMode(AMP_POWER_CTRL, OUTPUT);
    digitalWrite(AMP_POWER_CTRL, HIGH);

    ledcSetup(CHANNEL_0, 1000, 8);
    ledcAttachPin(SPEAKER_OUT, CHANNEL_0);
    int i = 3;
    while (i--) {
        ledcWriteTone(CHANNEL_0, 1000);
        delay(200);
        ledcWriteTone(CHANNEL_0, 0);
    }

    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 0);


    SPIClass sdSPI(VSPI);
    sdSPI.begin(SDCARD_CLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_SS);
    if (!SD.begin(SDCARD_SS, sdSPI)) {
        display.setCursor(0, 30);
        display.println("SDCard MOUNT FAIL");
        Serial.println("SDCard MOUNT FAIL");
    } else {
        display.setCursor(0, 30);
        display.println("SDCard MOUNT PASS");
        Serial.println("SDCard MOUNT PASS");
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        display.println("SDCard Size: " + String(cardSize) + "MB");
    }
    display.update();

    button_init();

    btnTicker.attach_ms(30, []() {
        for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
            pBtns[i].loop();
        }
    });

    Serial.println("setup done");
}


void loop()
{
    showBitmapExample();
    delay(2000);
#if !defined(__AVR)
    //drawCornerTest();
    showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    showFont("FreeMonoBold12pt7b", &FreeMonoBold12pt7b);
    //showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
    //showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
#else
    display.drawCornerTest();
    delay(2000);
    display.drawPaged(showFontCallback);
#endif
    delay(10000);
}

#if defined(_GxGDEP015OC1_H_)
void showBitmapExample()
{
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(5000);
    showBoat();
}
#endif

#if defined(_GxGDEW0154Z04_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
#if !defined(__AVR)
    display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red), GxEPD::bm_normal);
    delay(5000);
#endif
    display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
}
#endif

#if defined(_GxGDEW0154Z17_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
    display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
    display.drawExamplePicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
    //display.drawBitmap(BitmapExample2, sizeof(BitmapExample2));
}
#endif

#if defined(_GxGDE0213B1_H_)
void showBitmapExample()
{
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
#if !defined(__AVR)
    display.drawBitmap(first, sizeof(first));
    delay(5000);
    display.drawBitmap(second, sizeof(second));
    delay(5000);
    display.drawBitmap(third, sizeof(third));
    delay(5000);
#endif
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(5000);
    showBoat();
}
#endif

#if defined(_GxGDEW0213I5F_H_)
void showBitmapExample()
{
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1), GxEPD::bm_invert);
    delay(5000);
    display.drawBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
#if !defined(__AVR)
    display.drawBitmap(BitmapExample3, sizeof(BitmapExample3));
    delay(5000);
    display.drawBitmap(BitmapExample4, sizeof(BitmapExample4));
    delay(5000);
#endif
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(5000);
}
#endif

#if defined(_GxGDEW0213Z16_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
    display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red));
    delay(5000);
    display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
#if !defined(__AVR)
    display.drawExamplePicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample3), sizeof(BitmapExample4));
    delay(5000);
#endif
    display.drawExampleBitmap(BitmapWaveshare_black, sizeof(BitmapWaveshare_black));
    delay(2000);
    // example bitmaps for b/w/r are normal on b/w, but inverted on red
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2), GxEPD::bm_invert);
    delay(2000);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}
#endif

#if defined(_GxGDEH029A1_H_)
void showBitmapExample()
{
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(5000);
    showBoat();
}
#endif

#if defined(_GxGDEW029T5_H_)
void showBitmapExample()
{
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.drawExampleBitmap(BitmapExample3, sizeof(BitmapExample3));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    delay(5000);
//  showBoat();
}
#endif

#if defined(_GxGDEW029Z10_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
#if defined(__AVR)
    display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
#else
    display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red));
    delay(5000);
    display.drawExamplePicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
    display.drawExamplePicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample3), sizeof(BitmapExample4));
    delay(5000);
    display.drawExampleBitmap(BitmapWaveshare_black, sizeof(BitmapWaveshare_black));
    delay(2000);
    // example bitmaps for b/w/r are normal on b/w, but inverted on red
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2), GxEPD::bm_invert);
    delay(2000);
#endif
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}
#endif

#if defined(_GxGDEW027C44_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
    // draw black and red bitmap
    display.drawPicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
    return;
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(0, 0, BitmapExample1, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}
#endif

#if defined(_GxGDEW027W3_H_)
void showBitmapExample()
{
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
#if !defined(__AVR)
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(2000);
    display.drawExampleBitmap(BitmapExample3, sizeof(BitmapExample3));
    delay(2000);
    display.drawExampleBitmap(BitmapExample4, sizeof(BitmapExample4));
    delay(2000);
    display.drawExampleBitmap(BitmapExample5, sizeof(BitmapExample5));
    delay(2000);
#endif
    display.drawExampleBitmap(BitmapWaveshare, sizeof(BitmapWaveshare));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}
#endif

#if defined(_GxGDEW042T2_H_) || defined(_GxGDEW042T2_FPU_H_)
void showBitmapExample()
{
#if defined(__AVR)
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#else
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
#endif
}
#endif

#if defined(_GxGDEW042Z15_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
#if defined(__AVR)
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#else
    // draw black and red bitmap
    display.drawPicture(BitmapExample1, BitmapExample2, sizeof(BitmapExample1), sizeof(BitmapExample2));
    delay(5000);
    display.drawPicture(BitmapExample3, BitmapExample4, sizeof(BitmapExample3), sizeof(BitmapExample4));
    delay(5000);
    display.drawPicture(BitmapWaveshare_black, BitmapWaveshare_red, sizeof(BitmapWaveshare_black), sizeof(BitmapWaveshare_red));
    delay(5000);
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
#endif
}
#endif

#if defined(_GxGDEW0583T7_H_)
void showBitmapExample()
{
#if defined(__AVR)
    //display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#else
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
#endif
}
#endif

#if defined(_GxGDEW075T8_H_)
void showBitmapExample()
{
#if defined(__AVR)
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#else
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
#endif
}
#endif

#if defined(_GxGDEW075Z09_H_)
#define HAS_RED_COLOR
void showBitmapExample()
{
#if defined(__AVR)
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#elif defined(ARDUINO_GENERIC_STM32F103C)
    display.drawBitmap(BitmapExample1, sizeof(BitmapExample1));
#elif defined(ARDUINO_GENERIC_STM32F103V)
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C));
#else
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1));
    delay(2000);
    display.drawExampleBitmap(BitmapExample2, sizeof(BitmapExample2));
    delay(5000);
    display.drawExamplePicture_3C(BitmapPicture_3C, sizeof(BitmapPicture_3C));
#endif
}
#endif

void showFont(const char name[], const GFXfont *f)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
#if defined(HAS_RED_COLOR)
    display.setTextColor(GxEPD_RED);
#endif
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
    display.update();
    delay(5000);
}

void showFontCallback()
{
    const char *name = "FreeMonoBold9pt7b";
    const GFXfont *f = &FreeMonoBold9pt7b;
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
#if defined(HAS_RED_COLOR)
    display.setTextColor(GxEPD_RED);
#endif
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
}

void drawCornerTest()
{
    display.drawCornerTest();
    delay(5000);
    uint8_t rotation = display.getRotation();
    for (uint16_t r = 0; r < 4; r++) {
        display.setRotation(r);
        display.fillScreen(GxEPD_WHITE);
        display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
        display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
        display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
        display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
        display.update();
        delay(5000);
    }
    display.setRotation(rotation); // restore
}

#if defined(_GxGDEP015OC1_H_) || defined(_GxGDE0213B1_H_) || defined(_GxGDEH029A1_H_)
#include "IMG_0001.h"
void showBoat()
{
    // thanks to bytecrusher: http://forum.arduino.cc/index.php?topic=487007.msg3367378#msg3367378
    uint16_t x = (display.width() - 64) / 2;
    uint16_t y = 5;
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK);
    display.update();
    delay(500);
    uint16_t forward = GxEPD::bm_invert | GxEPD::bm_flip_x;
    uint16_t reverse = GxEPD::bm_invert | GxEPD::bm_flip_x | GxEPD::bm_flip_y;
    for (; y + 180 + 5 <= display.height(); y += 5) {
        display.fillScreen(GxEPD_WHITE);
        display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK, forward);
        display.updateWindow(0, 0, display.width(), display.height());
        delay(500);
    }
    delay(1000);
    for (; y >= 5; y -= 5) {
        display.fillScreen(GxEPD_WHITE);
        display.drawExampleBitmap(gImage_IMG_0001, x, y, 64, 180, GxEPD_BLACK, reverse);
        display.updateWindow(0, 0, display.width(), display.height());
        delay(1000);
    }
    display.update();
    delay(1000);
}
#endif
