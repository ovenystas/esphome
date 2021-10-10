#include "nhd_char_i2c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nhd_char_i2c {

static const char *const TAG = "nhd_char_i2c";

static const uint8_t LCD_DISPLAY_BACKLIGHT_ON = 0x08;
static const uint8_t LCD_DISPLAY_BACKLIGHT_OFF = 0x01;

void NhdCharI2cLcd::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Newhaven LCD...");
  this->backlight_value_ = LCD_DISPLAY_BACKLIGHT_ON;
  if (!this->write_bytes(this->backlight_value_, nullptr, 0)) {
    this->mark_failed();
    return;
  }

  LCDDisplay::setup();
}

void NhdCharI2cLcd::dump_config() {
  ESP_LOGCONFIG(TAG, "Newhaven LCD:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with LCD failed!");
  }
}

void NhdCharI2cLcd::write_n_bits(uint8_t value, uint8_t n) {
  if (n == 4) {
    // Ugly fix: in the super setup() with n == 4 value needs to be shifted left
    value <<= 4;
  }
  uint8_t data = value | this->backlight_value_;  // Set backlight state
  this->write_bytes(data, nullptr, 0);
  // Pulse ENABLE
  this->write_bytes(data | 0x04, nullptr, 0);
  delayMicroseconds(1);  // >450ns
  this->write_bytes(data, nullptr, 0);
  delayMicroseconds(100);  // >37us
}

void NhdCharI2cLcd::send(uint8_t value, bool rs) {
  this->write_n_bits((value & 0xF0) | rs, 0);
  this->write_n_bits(((value << 4) & 0xF0) | rs, 0);
}

void NhdCharI2cLcd::backlight() {
  this->backlight_value_ = LCD_DISPLAY_BACKLIGHT_ON;
  this->write_bytes(this->backlight_value_, nullptr, 0);
}

void NhdCharI2cLcd::no_backlight() {
  this->backlight_value_ = LCD_DISPLAY_BACKLIGHT_OFF;
  this->write_bytes(this->backlight_value_, nullptr, 0);
}

void NhdCharI2cLcd::backlight(uint8_t brightness) {
  if (brightness >= LCD_DISPLAY_BACKLIGHT_OFF && brightness <= LCD_DISPLAY_BACKLIGHT_ON) {
    this->backlight_value_ = brightness;
    this->write_bytes(this->backlight_value_, nullptr, 0);
  }
}

}  // namespace nhd_char_i2c
}  // namespace esphome
