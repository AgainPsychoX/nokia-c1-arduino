#include <Arduino.h>
#include "lcd.hpp"
#include "lcd_text.hpp"

#define BUTTON_LEFT     A7
#define BUTTON_TOP      PC2
#define BUTTON_RIGHT    PC1
#define BUTTON_BOTTOM   PC0
#define BUZZER_PIN      PD3

constexpr auto backgroundColor = RGB(0, 0, 0);  // Kolor tła
constexpr auto frameColor = RGB(0, 0, 255);     // Kolor ramki
uint16_t snakeColors[] = {              // Lista kolorów węża
    // RGB(89, 38, 19),
    // RGB(198, 81, 37),
    // RGB(235, 120, 39),
    // RGB(246, 208, 45),
    // RGB(127, 194, 54),
    RGB(38, 153, 0),
    RGB(230, 92, 0),
    RGB(230, 92, 0),
    RGB(38, 153, 0),
};
constexpr auto appleColor = RGB(255, 0, 0);     // Kolor jabłka
//constexpr auto obstacleColor = RGB(

constexpr uint8_t squareSize = 5; // Wielkość 

constexpr long moveDelay = 150; // Czas pomiędzy ruchami

uint8_t snakeColorIndex = 0;
enum direction_t {
    LEFT,
    TOP,
    RIGHT,
    BOTTOM,
};
constexpr size_t maxLength = 300;
struct position_t {
    uint8_t x;
    uint8_t y;
};
position_t body[maxLength];
size_t headIndex;
size_t tailIndex;
direction_t direction;
position_t apple;
unsigned short score;
char scoreStringBuffer[5];

void drawSquare(position_t pos) {
    const uint8_t x = LCD::left + 1 + pos.x * squareSize;
    const uint8_t y = LCD::top + 1 + pos.y * squareSize;
    LCD::setArea(x, y, x + squareSize - 1, y + squareSize - 1);
    for (uint8_t i = 0; i < squareSize * squareSize; i++) {
        LCD::nextPixel();
    }
}

void start() {
    LCD::clear();
    // LCD::setForegroundColor(backgroundColor);
    // LCD::drawFilledRectangle(LCD::left + 1, LCD::top + 1, LCD::right - 1, LCD::bottom - 1);
    LCD::setForegroundColor(frameColor);
    LCD::drawRectangle(
        LCD::left,
        LCD::top, 
        LCD::left + ((LCD::right - LCD::left - 1) / squareSize) * squareSize + 1, 
        LCD::top + ((LCD::bottom - LCD::top - 1) / squareSize) * squareSize + 1
    );

    direction = RIGHT;

    headIndex = 10 - 1;
    body[headIndex].x = random(10, (LCD::right - 1) / squareSize - 10);
    body[headIndex].y = random(10, (LCD::bottom - 1) / squareSize - 10);

    for (uint8_t i = 0; i < 10 - 1; i++) {
        body[i].x = body[headIndex].x - (10 - 1) + 1 + i;
        body[i].y = body[headIndex].y;
        snakeColorIndex = (snakeColorIndex + 1) % (sizeof(snakeColors) / sizeof(snakeColors[0]));
        LCD::setForegroundColor(snakeColors[snakeColorIndex]);
        drawSquare(body[i]);
    }

    tailIndex = 0;

    spawnApple();
    score = 0;
}

bool checkIsBody(position_t position) {
    size_t length = tailIndex < headIndex 
        ? headIndex - tailIndex + 1 
        : maxLength - tailIndex + headIndex + 1
    ;
    for (size_t i = 0; i < length - 1; i++) {
        if (
            body[tailIndex + i].x == position.x &&
            body[tailIndex + i].y == position.y
        ) {
            return true;
        }
    }
    return false;
}

bool move() {
    position_t next = body[headIndex];
    switch (direction) {
        case LEFT:   next.x -= 1; break;
        case TOP:    next.y -= 1; break;
        case RIGHT:  next.x += 1; break;
        case BOTTOM: next.y += 1; break;
    }
    headIndex = (headIndex + 1) % maxLength;
    body[headIndex] = next;

    snakeColorIndex = (snakeColorIndex + 1) % (sizeof(snakeColors) / sizeof(snakeColors[0]));
    LCD::setForegroundColor(snakeColors[snakeColorIndex]);
    drawSquare(next);

    // Sprawdzanie kolizji z ciałem
    if (checkIsBody(next)) {
        return false;
    }

    // Sprawdzanie kolizji z ramką
    if (
        (next.x + 1) * squareSize >= LCD::right ||
        (next.y + 1) * squareSize >= LCD::bottom
    ) {
        return false;
    }

    if (apple.x == next.x && apple.y == next.y) {
        digitalWrite(BUZZER_PIN, HIGH);
        score += 1;
        snprintf(scoreStringBuffer, sizeof(scoreStringBuffer) / sizeof(scoreStringBuffer[0]), "%u", score);
        LCD::printStringHorizontal_8x16(scoreStringBuffer, LCD::left + 3, LCD::top + 2);
        spawnApple();
        delay(50);
        digitalWrite(BUZZER_PIN, LOW);
    }
    else {
        LCD::setForegroundColor(backgroundColor);
        drawSquare(body[tailIndex]);
        tailIndex = (tailIndex + 1) % maxLength;
    }

    return true;
}

void spawnApple() {
    do {
        apple.x = random(0, (LCD::right - 1) / squareSize - 1);
        apple.y = random(0, (LCD::bottom - 1) / squareSize - 1);
    }
    while (checkIsBody(apple));
    LCD::setForegroundColor(appleColor);
    drawSquare(apple);
}

void setup() {
    pinMode(BUTTON_LEFT,   INPUT_PULLUP);
    pinMode(BUTTON_TOP,    INPUT_PULLUP);
    pinMode(BUTTON_RIGHT,  INPUT_PULLUP);
    pinMode(BUTTON_BOTTOM, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    Serial.begin(9600);
    LCD::init();

    digitalWrite(BUZZER_PIN, HIGH);
    LCD::setForegroundColor(RGB(255, 255, 255));
    LCD::printStringHorizontal_8x16("S N A K E", LCD::left + 5, LCD::top + 5);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);

    start();
}

bool gameOver = true;
void loop() {
    delay(moveDelay);

    /**/ if (direction != RIGHT  && digitalRead(BUTTON_LEFT)   == LOW) direction = LEFT;
    else if (direction != BOTTOM && digitalRead(BUTTON_TOP)    == LOW) direction = TOP;
    else if (direction != TOP    && digitalRead(BUTTON_BOTTOM) == LOW) direction = BOTTOM;
    else if (direction != LEFT   && digitalRead(BUTTON_RIGHT)  == LOW) direction = RIGHT;

    if (!move()) {
        delay(50);
        digitalWrite(BUZZER_PIN, HIGH);
        LCD::setForegroundColor(RGB(255, 255, 255));
        LCD::printStringHorizontal_8x16("GAME OVER", LCD::left + 5, LCD::top + 5);
        LCD::printStringHorizontal_8x16("Wynik:", LCD::left + 5, LCD::top + 5 + 16);
        snprintf(scoreStringBuffer, sizeof(scoreStringBuffer), "%u", score);
        LCD::printStringHorizontal_8x16(scoreStringBuffer, LCD::left + 5 + 6 * 8 + 4, LCD::top + 5 + 16);
        delay(1000);
        digitalWrite(BUZZER_PIN, LOW);
        start();
    }
}
