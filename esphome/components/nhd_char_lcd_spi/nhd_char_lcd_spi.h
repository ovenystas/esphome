#pragma once

#include "esphome/core/component.h"
#include "esphome/components/nhd_char_lcd/nhd_char_lcd.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace nhd_char_lcd_spi {

class NhdCharLcdSpi : public nhd_char_lcd::NhdCharLcd, public spi::SPIDevice<
    spi::BIT_ORDER_MSB_FIRST,
    spi::CLOCK_POLARITY_HIGH,
    spi::CLOCK_PHASE_TRAILING,
    spi::DATA_RATE_75KHZ> {

 public:
  void set_writer(std::function<void(NhdCharLcdSpi &)> &&writer) {
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

  std::function<void(NhdCharLcdSpi &)> writer_;
};

}  // namespace nhd_char_lcd_spi
}  // namespace esphome
