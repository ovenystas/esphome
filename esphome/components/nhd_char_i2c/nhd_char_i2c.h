#pragma once

#include "esphome/core/component.h"
#include "esphome/components/nhd_char_base/nhd_char_base.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace nhd_char_i2c {

class NhdCharI2cLcd : public nhd_char_base::LCDDisplay, public i2c::I2CDevice {
 public:
  void set_writer(std::function<void(NhdCharI2cLcd &)> &&writer) { this->writer_ = std::move(writer); }
  void setup() override;
  void dump_config() override;
  void backlight();
  void no_backlight();

 protected:
  bool is_four_bit_mode() override { return true; }
  void write_n_bits(uint8_t value, uint8_t n) override;
  void send(uint8_t value, bool rs) override;

  void call_writer() override { this->writer_(*this); }

  // Stores the current state of the backlight.
  uint8_t backlight_value_;  // 1-8, 1=OFF, 2=Lowest brightness, 8=Highest brightness
  std::function<void(NhdCharI2cLcd &)> writer_;
};

}  // namespace nhd_char_i2c
}  // namespace esphome
