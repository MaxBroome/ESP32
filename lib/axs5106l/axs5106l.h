#pragma once

#include <Arduino.h>
#include <Wire.h>

#define AXS5106L_I2C_ADDR   0x63
#define AXS5106L_MAX_POINTS 2

struct TouchPoint {
    uint16_t x;
    uint16_t y;
};

struct TouchData {
    TouchPoint points[AXS5106L_MAX_POINTS];
    uint8_t count;
};

class AXS5106L {
public:
    AXS5106L(TwoWire &wire, int8_t rst_pin, int8_t int_pin = -1);

    bool begin();
    bool read(TouchData &data);

private:
    TwoWire &_wire;
    int8_t   _rst_pin;
    int8_t   _int_pin;

    bool i2cRead(uint8_t reg, uint8_t *buf, uint8_t len);
};
