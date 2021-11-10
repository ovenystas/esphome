#pragma once

#include "esphome/core/component.h"
#include "esphome/components/nhd_char_lcd/nhd_char_lcd.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace nhd_char_lcd_uart {

class NhdCharLcdUart : public nhd_char_lcd::NhdCharLcd, public uart::UARTDevice {

 public:
  void set_writer(std::function<void(NhdCharLcdUart &)> &&writer) {
    this->writer_ = std::move(writer);
  }
  void setup() override;
  void dump_config() override;

 protected:
  void send(uint8_t* data, uint8_t len);
  void send_command(uint8_t cmd, uint8_t* data, uint8_t len);

  void call_writer() override {
    this->writer_(*this);
  }

  std::function<void(NhdCharLcdUart &)> writer_;
};

}  // namespace nhd_char_lcd_uart
}  // namespace esphome
