#include "axs5106l.h"

static constexpr uint8_t REG_ID    = 0x08;
static constexpr uint8_t REG_TOUCH = 0x01;

AXS5106L::AXS5106L(TwoWire &wire, int8_t rst_pin, int8_t int_pin)
    : _wire(wire), _rst_pin(rst_pin), _int_pin(int_pin) {}

bool AXS5106L::begin() {
    pinMode(_rst_pin, OUTPUT);
    if (_int_pin >= 0) {
        pinMode(_int_pin, INPUT);
    }

    // Hardware reset
    digitalWrite(_rst_pin, LOW);
    delay(200);
    digitalWrite(_rst_pin, HIGH);
    delay(300);

    // Verify communication
    uint8_t id[3] = {0};
    return i2cRead(REG_ID, id, sizeof(id));
}

bool AXS5106L::read(TouchData &data) {
    uint8_t buf[14] = {0};
    if (!i2cRead(REG_TOUCH, buf, sizeof(buf))) {
        return false;
    }

    data.count = buf[1];
    if (data.count == 0 || data.count > AXS5106L_MAX_POINTS) {
        data.count = 0;
        return false;
    }

    for (uint8_t i = 0; i < data.count; i++) {
        uint8_t off = 2 + (i * 6);
        data.points[i].x = ((uint16_t)(buf[off]     & 0x0F) << 8) | buf[off + 1];
        data.points[i].y = ((uint16_t)(buf[off + 2] & 0x0F) << 8) | buf[off + 3];
    }
    return true;
}

bool AXS5106L::i2cRead(uint8_t reg, uint8_t *buf, uint8_t len) {
    // Separate write and read transactions for ESP32-P4 compatibility
    _wire.beginTransmission(AXS5106L_I2C_ADDR);
    _wire.write(reg);
    if (_wire.endTransmission(true) != 0) {  // true = send stop bit
        return false;
    }
    
    delay(1);  // Small delay between transactions
    
    uint8_t received = _wire.requestFrom((uint8_t)AXS5106L_I2C_ADDR, len, (uint8_t)true);
    if (received != len) {
        return false;
    }
    
    for (uint8_t i = 0; i < len; i++) {
        if (_wire.available()) {
            buf[i] = _wire.read();
        } else {
            return false;
        }
    }
    return true;
}
