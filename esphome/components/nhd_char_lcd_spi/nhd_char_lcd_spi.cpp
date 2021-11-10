#include "nhd_char_lcd_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nhd_char_lcd_spi {

static const char *const TAG = "nhd_char_lcd_spi";

static const uint8_t COMMAND_PREFIX = 0xFE;


void NhdCharLcdSpi::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Newhaven LCD...");
  NhdCharLcd::setup();
}

void NhdCharLcdSpi::dump_config() {
  ESP_LOGCONFIG(TAG, "Newhaven LCD:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  //LOG_SPI_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with LCD failed!");
  }
}

void NhdCharLcdSpi::send(uint8_t* data, uint8_t len) {
  enable();
  for (uint8_t index = 0; index < len; index++) {
    transfer_byte(data[index]);
  }
  disable();
}

void NhdCharLcdSpi::send_command(uint8_t cmd, uint8_t* data, uint8_t len) {
  enable();
  transfer_byte(COMMAND_PREFIX);
  transfer_byte(cmd);
  for (uint8_t index = 0; index < len; index++) {
    transfer_byte(data[index]);
  }
  disable();
}

}  // namespace nhd_char_lcd_spi
}  // namespace esphome
