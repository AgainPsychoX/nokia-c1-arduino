#pragma once

#include "lcd.hpp"
#include "font8x16.hpp"

namespace LCD {
    void printCharVertical_8x16(char c, uint8_t x, uint8_t y) {
        setArea(x, y, x + 8 - 1, y + 16 - 1);
        c -= ' ';
        for (uint8_t y = 0; y < 16; y++) {
            uint8_t v = pgm_read_byte(font8x16 + static_cast<size_t>(c) * 16 + y);
            for (uint8_t x = 0; x < 8; x++) {
                if (v & 1) {
                    write<Data>(foregroundColorPart[1]);
                    write<Data>(foregroundColorPart[0]);
                }
                else {
                    write<Data>(backgroundColorPart[1]);
                    write<Data>(backgroundColorPart[0]);
                }
                v >>= 1;
            }
        }
    }
    void printCharHorizontal_8x16(char c, uint8_t x, uint8_t y) {
        c -= ' ';
        for (uint8_t i = 0; i < 16; i++) {
            setArea(x + i, y, x + i, y + 8 - 1);
            uint8_t v = pgm_read_byte(font8x16 + static_cast<size_t>(c) * 16 + i);
            for (uint8_t j = 0; j < 8; j++) {
                if (v & 0b10000000) {
                    write<Data>(foregroundColorPart[1]);
                    write<Data>(foregroundColorPart[0]);
                }
                else {
                    write<Data>(backgroundColorPart[1]);
                    write<Data>(backgroundColorPart[0]);
                }
                v <<= 1;
            }
        }
    }
    void printStringHorizontal_8x16(const char* string, uint8_t x, uint8_t y) {
        while (*string) {
            printCharVertical_8x16(*string++, x, y);
            x += 8;
        }
    }
    void printStringVertical_8x16(const char* string, uint8_t x, uint8_t y) {
        while (*string) {
            y -= 8;
            printCharHorizontal_8x16(*string++, x, y);
        }
    }

    template <unsigned short scale = 2>
    void printBigCharVertical_8x16(char c, uint8_t x, uint8_t y) {
        setArea(x, y, x + 8 * scale - 1, y + 16 * scale - 1);
        c -= ' ';
        for (uint8_t y = 0; y < 16; y++) {
            for (uint8_t t = 0; t < scale; t++) {
                uint8_t v = pgm_read_byte(font8x16 + static_cast<size_t>(c) * 16 + y);
                for (uint8_t x = 0; x < 8; x++) {
                    for (uint8_t t = 0; t < scale; t++) {
                        if (v & 1) {
                            write<Data>(foregroundColorPart[1]);
                            write<Data>(foregroundColorPart[0]);
                        }
                        else {
                            write<Data>(backgroundColorPart[1]);
                            write<Data>(backgroundColorPart[0]);
                        }
                    }
                    v >>= 1;
                }
            }
        }
    }
    template <unsigned short scale = 2>
    void printBigStringHorizontal_8x16(const char* string, uint8_t x, uint8_t y) {
        while (*string) {
            printBigCharVertical_8x16<scale>(*string++, x, y);
            x += 8 * scale;
        }
    }
}
