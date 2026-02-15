#pragma once

// =============================================================================
// PINOUT REFERENCE
// Board:   Waveshare ESP32-P4-WIFI6-POE-ETH
// Display: Waveshare 1.47" Touch LCD (172x320, JD9853 + AXS5106L)
//
// ACTIVE WIRING TABLE:
//
//   LCD Pin     ESP32-P4 GPIO   Notes
//   ─────────   ─────────────   ──────────────────────────────────
//   VCC         3.3V            Power supply
//   GND         GND             Ground
//   MOSI        GPIO20          SPI data out
//   SCLK        GPIO21          SPI clock
//   LCD_CS      GPIO22          LCD chip select
//   LCD_DC      GPIO23          Data/Command control
//   LCD_RST     GPIO26          LCD reset
//   LCD_BL      GPIO27          Backlight (HIGH = on)
//   MISO        NC              Display doesn't output data
//   TP_SDA      GPIO7           Touch I2C data  (shared I2C bus)
//   TP_SCL      GPIO8           Touch I2C clock (shared I2C bus)
//   TP_INT      GPIO5           Touch interrupt
//   TP_RST      GPIO6           Touch reset
//
// WiFi:
//   ESP32-C6 co-processor connected via SDIO (managed by esp_hosted)
//   Standard WiFi.h API works transparently through the SDIO bridge
//
// RESERVED GPIOs — DO NOT USE:
//   GPIO 9-13   Onboard ES8311 audio codec (I2S + MCLK)
//   GPIO 24-25  USB-JTAG (default)
//   GPIO 28-31  Ethernet RMII (IP101GRI)
//   GPIO 34-38  Strapping pins + Ethernet
//   GPIO 39-44  SD card (SDIO)
//   GPIO 49-53  Ethernet RMII + PA enable
//
// =============================================================================

// --- SPI Display (JD9853, ST7789-compatible) ---
#define PIN_LCD_MOSI    20
#define PIN_LCD_SCLK    21
#define PIN_LCD_CS      22
#define PIN_LCD_DC      23
#define PIN_LCD_RST     26
#define PIN_LCD_BL      27

// --- I2C Touch (AXS5106L) ---
#define PIN_TP_SDA       7
#define PIN_TP_SCL       8
#define PIN_TP_INT       5
#define PIN_TP_RST       6

// --- Factory Reset Button ---
// Use BOOT button (GPIO35) for factory reset
// Hold BOOT button for 5s to wipe NVS and clear all configs
// The BOOT button is connected to GPIO35 and pulls LOW when pressed
#define PIN_RESET_BTN   35