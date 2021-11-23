#include "nhd_char_lcd_uart.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nhd_char_lcd_uart {

static const char *const TAG = "nhd_char_lcd_uart";

static const uint8_t COMMAND_PREFIX = 0xFE;


void NhdCharLcdUart::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Newhaven LCD over UART ...");
  NhdCharLcd::setup();
}

void NhdCharLcdUart::dump_config() {
  ESP_LOGCONFIG(TAG, "Newhaven LCD:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  //LOG_SPI_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with LCD failed!");
  }
}

void NhdCharLcdUart::send(uint8_t* data, uint8_t len) {
  this->write_array(data, len);
}

void NhdCharLcdUart::send_command(uint8_t cmd, uint8_t* data, uint8_t len) {
  this->write_byte(COMMAND_PREFIX);
  this->write_byte(cmd);
  this->write_array(data, len);
}

}  // namespace nhd_char_lcd_uart
}  // namespace esphome
