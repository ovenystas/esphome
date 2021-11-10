#pragma once

#include "esphome/core/component.h"
#include "esphome/components/nhd_char_lcd/nhd_char_lcd.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace nhd_char_lcd_i2c {

class NhdCharLcdI2c : public nhd_char_lcd::NhdCharLcd, public i2c::I2CDevice {
 public:
  void set_writer(std::function<void(NhdCharLcdI2c &)> &&writer) {
    this->writer_ = std::move(writer);
  }
  void setup() override;
  void dump_config() override;

 protected:
  bool send(uint8_t* data, uint8_t len);
  bool send_command(uint8_t cmd, uint8_t* data, uint8_t len);

  void call_writer() override {
    this->writer_(*this);
  }

  std::function<void(NhdCharLcdI2c &)> writer_;
};

}  // namespace nhd_char_lcd_i2c
}  // namespace esphome
