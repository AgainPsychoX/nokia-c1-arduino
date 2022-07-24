#include <Arduino.h>
#include "lcd.hpp"
#include "lcd_text.hpp"

#define BUTTON_LEFT     A7
#define BUTTON_TOP      PC2
#define BUTTON_RIGHT    PC1
#define BUTTON_BOTTOM   PC0

char* text = "09AZaz";
constexpr uint8_t len = 6;
uint8_t pos = 0;

void setup() {
    pinMode(BUTTON_LEFT,   INPUT_PULLUP);
    pinMode(BUTTON_TOP,    INPUT_PULLUP);
    pinMode(BUTTON_RIGHT,  INPUT_PULLUP);
    pinMode(BUTTON_BOTTOM, INPUT_PULLUP);

    Serial.begin(9600);
    LCD::init();
    LCD::clear();

    LCD::setForegroundColor(RGB(255, 255, 255));
    LCD::printStringHorizontal_8x16("Horizontal", LCD::left + 5, LCD::top + 5);
    LCD::printStringVertical_8x16("Vertical", LCD::left + 5, LCD::bottom - 5);

    LCD::printBigStringHorizontal_8x16("BIG123!", LCD::left + 5, LCD::top + 5 + 16);
    LCD::printBigStringHorizontal_8x16<3>("@#$%", LCD::left + 5 + 16, LCD::top + 5 + 16 + 32);

    draw();
}

void draw() {
    LCD::printBigStringHorizontal_8x16(text, LCD::left + 5 + 16, LCD::top + 5 + 16 + 32 + 48);
}

void loop() {
    LCD::setForegroundColor(RGB(0, 0, 0));
    LCD::drawHorizontalLine(LCD::left + 5 + 16, LCD::left + 5 + 16 + len * 16, LCD::top + 5 + 16 + 32 + 48 + 32);
    LCD::setForegroundColor(RGB(255, 255, 255));
    /**/ if (digitalRead(BUTTON_LEFT)   == LOW) { pos = pos == 0 ? len - 1 : pos - 1; draw(); }
    else if (digitalRead(BUTTON_RIGHT)  == LOW) { pos = (pos + 1) % len; draw(); }
    else if (digitalRead(BUTTON_TOP)    == LOW) { text[pos] += 1; draw(); }
    else if (digitalRead(BUTTON_BOTTOM) == LOW) { text[pos] -= 1; draw(); }
    LCD::drawHorizontalLine(LCD::left + 5 + 16 + pos * 16, LCD::left + 5 + 16 + pos * 16 + 16, LCD::top + 5 + 16 + 32 + 48 + 32);
    delay(100);
}
