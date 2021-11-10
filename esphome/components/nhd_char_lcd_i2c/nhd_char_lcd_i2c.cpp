#include "nhd_char_lcd_i2c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nhd_char_lcd_i2c {

static const char *const TAG = "nhd_char_lcd_i2c";

static const uint8_t COMMAND_PREFIX = 0xFE;


void NhdCharLcdI2c::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Newhaven LCD...");
  NhdCharLcd::setup();
}

void NhdCharLcdI2c::dump_config() {
  ESP_LOGCONFIG(TAG, "Newhaven LCD:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with LCD failed!");
  }
}

bool NhdCharLcdI2c::send(uint8_t* data, uint8_t len) {
  return this->write(data, len) == i2c::ERROR_OK;
}

bool NhdCharLcdI2c::send_command(uint8_t cmd, uint8_t* data, uint8_t len) {
  return (this->write(&COMMAND_PREFIX, 1) == i2c::ERROR_OK) &&
    (this->write_register(cmd, data, len) == i2c::ERROR_OK);
}

}  // namespace nhd_char_lcd_i2c
}  // namespace esphome
