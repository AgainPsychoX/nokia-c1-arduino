#pragma once
// Library for Nokia C1 LCD.
// (Patryk Ludwikowski 2019)

#include <Arduino.h>

#define LCD_RST_PIN         (4)
#define LCD_CS_PIN          (5)
#define LCD_SDA_PIN         (11)
#define LCD_SCK_PIN         (13)

#define LCD_RST_High()  do { digitalWrite(LCD_RST_PIN, HIGH); } while(0)
#define LCD_RST_Low()   do { digitalWrite(LCD_RST_PIN, LOW);  } while(0)
#define LCD_CS_High()   do { digitalWrite(LCD_CS_PIN,  HIGH); } while(0)
#define LCD_CS_Low()    do { digitalWrite(LCD_CS_PIN,  LOW);  } while(0)
#define LCD_SDA_High()  do { digitalWrite(LCD_SDA_PIN, HIGH); } while(0)
#define LCD_SDA_Low()   do { digitalWrite(LCD_SDA_PIN, LOW);  } while(0)
#define LCD_SCK_High()  do { digitalWrite(LCD_SCK_PIN, HIGH); } while(0)
#define LCD_SCK_Low()   do { digitalWrite(LCD_SCK_PIN, LOW);  } while(0)

#define RGB(r,g,b) (((static_cast<uint16_t>(r)&0xF8)<<8)|((static_cast<uint16_t>(g)&0xFC)<<3)|((static_cast<uint16_t>(b)&0xF8)>>3)) // 5 red | 6 green | 5 blue

namespace LCD {
    enum writeType {
        Command = 0,
        Data = 1
    };
    template <unsigned int isItDataTypeWrite>
    void write(uint8_t data) {
        LCD_CS_Low();
        if (isItDataTypeWrite) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b10000000) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b01000000) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00100000) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00010000) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00001000) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00000100) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00000010) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        if (data & 0b00000001) LCD_SDA_High(); else LCD_SDA_Low(); LCD_SCK_High(); LCD_SCK_Low();
        LCD_CS_High();
    }

    constexpr uint8_t left = 2;
    constexpr uint8_t top = 1;
    constexpr uint8_t right = 129;
    constexpr uint8_t bottom = 160;

    void setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
        // set X range
        write<Command>(0x2a);
        write<Data>(0);
        write<Data>(x0);
        write<Data>(0);
        write<Data>(x1);
        // set Y range
        write<Command>(0x2b);
        write<Data>(0);
        write<Data>(y0);
        write<Data>(0);
        write<Data>(y1);
        // memory write
        write<Command>(0x2c);
    }
    inline void setCursor(uint8_t x, uint8_t y) {
        setArea(x, y, right, bottom);
    }

    static union {
        uint16_t foregroundColor;
        uint8_t foregroundColorPart[2];
    };
    static union {
        uint16_t backgroundColor;
        uint8_t backgroundColorPart[2];
    };
    inline void setForegroundColor(uint16_t color) {
        foregroundColor = color;
    }
    inline void setBackgroundColor(uint16_t color) {
        backgroundColor = color;
    }

    inline void nextPixel() {
        write<Data>(foregroundColorPart[1]);
        write<Data>(foregroundColorPart[0]);
    }
    inline void pushColor(uint16_t color) {
        write<Data>(color >> 0x8);
        write<Data>(color & 0xFF);
    }

    ////////////////////////////////////////////////////////////////////////////////

    void drawHorizontalLine(uint8_t x0, uint8_t x1, uint8_t y) {
        setArea(x0, y, x1, y);
        for (; x0 <= x1; x0++) {
            nextPixel();
        }
    }
    void drawVerticalLine(uint8_t x, uint8_t y0, uint8_t y1) {
        setArea(x, y0, x, y1);
        for (; y0 <= y1; y0++) {
            nextPixel();
        }
    }
    void drawRectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
        drawHorizontalLine(x0, x1, y0);
        drawHorizontalLine(x0, x1, y1);
        drawVerticalLine(x0, y0 + 1, y1 - 1);
        drawVerticalLine(x1, y0 + 1, y1 - 1);
    }
    void drawFilledRectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
        setArea(x0, y0, x1, y1);
        for (uint8_t y = y0; y <= y1; y++) {
            for (uint8_t x = x0; x <= x1; x++) {
                nextPixel();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    void clear() {
        if (foregroundColor != backgroundColor) {
            foregroundColor ^= backgroundColor;
            backgroundColor ^= foregroundColor;
            foregroundColor ^= backgroundColor;
        }
        drawFilledRectangle(left, top, right, bottom);
        if (foregroundColor != backgroundColor) {
            foregroundColor ^= backgroundColor;
            backgroundColor ^= foregroundColor;
            foregroundColor ^= backgroundColor;
        }
    }

    void init() {
        pinMode(LCD_RST_PIN, OUTPUT);
        pinMode(LCD_CS_PIN,  OUTPUT);
        pinMode(LCD_SDA_PIN, OUTPUT);
        pinMode(LCD_SCK_PIN, OUTPUT);
        LCD_SCK_Low();

        // Reset
        LCD_RST_Low();
        delay(10);
        LCD_RST_High();
        delay(10);
        //write<Command>(0x01); // Software Reset 
        //delay(10);

        // Sleep out
        write<Command>(0x11);
        delay(10);

        // Interface pixel format
        write<Command>(0x3a); 
        write<Data>(0x05); // 565 color format

        // Memory access controll
        //write<Command>(0x36);
        //write<Data>(0xc0); // mirror x and y

        // Display on
        write<Command>(0x29);

        // Initial colors
        setBackgroundColor(RGB(0, 0, 0));
        setForegroundColor(RGB(255, 255, 255));
    }
}
