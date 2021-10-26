#include "nhd_char_base.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include <cstring>

namespace esphome {
namespace nhd_char_base {

static const char *const TAG = "lcd";

struct Command {
  uint8_t cmd;
  uint16_t exec_time_us;
};

// First set bit determines command, bits after that are the data.
static const uint8_t COMMAND_PREFIX = 0xFE;

static const Command COMMAND_DISPLAY_ON = { 0x41, 100u };
static const Command COMMAND_DISPLAY_OFF = { 0x42, 100u };
static const Command COMMAND_SET_CURSOR = { 0x45, 100u };
static const Command COMMAND_CURSOR_HOME = { 0x46, 1500u };
static const Command COMMAND_UNDERLINE_CURSOR_ON = { 0x47, 1500u };
static const Command COMMAND_UNDERLINE_CURSOR_OFF = { 0x48, 1500u };
static const Command COMMAND_MOVE_CURSOR_LEFT = { 0x49, 100u };
static const Command COMMAND_MOVE_CURSOR_RIGHT = { 0x4A, 100u };
static const Command COMMAND_BLINKING_CURSOR_ON = { 0x4B, 100u };
static const Command COMMAND_BLINKING_CURSOR_OFF = { 0x4C, 100u };
static const Command COMMAND_BACKSPACE = { 0x4E, 100u };
static const Command COMMAND_CLEAR_SCREEN = { 0x51, 1500u };
static const Command COMMAND_SET_CONTRAST = { 0x52, 500u };
static const Command COMMAND_SET_BACKLIGHT_BRIGHTNESS = { 0x53, 100u };
static const Command COMMAND_LOAD_CUSTOM_CHARACTER = { 0x54, 200u };
static const Command COMMAND_MOVE_DISPLAY_LEFT = { 0x55, 100u };
static const Command COMMAND_MOVE_DISPLAY_RIGHT = { 0x56, 100u };
static const Command COMMAND_CHANGE_RS232_BAUD_RATE = { 0x61, 3000u };
static const Command COMMAND_CHANGE_I2C_ADDRESS = { 0x62, 3000u };
static const Command COMMAND_DISPLAY_FIRMWARE_VERSION = { 0x70, 4000u };
static const Command COMMAND_DISPLAY_RS232ADDRESS = { 0x71, 10000u };
static const Command COMMAND_DISPLAY_I2C_ADDRESS = { 0x72, 4000u };

void NhdChar::setup() {
  this->buffer_ = new uint8_t[this->positions_];  // NOLINT
  this->clear();

  // Commands can only be sent 100ms after boot-up, so let's wait if we're close
  const uint8_t now = millis();
  if (now < 100) {
    delay(100u - now);
  }

  this->command_(COMMAND_CLEAR_SCREEN);
  this->command_(COMMAND_SET_CONTRAST, 40u);
  this->set_backlight(8u);
  this->backlight();
  this->command_(COMMAND_UNDERLINE_CURSOR_OFF);

  this->print("NewHaven Display\n"
              "Serial LCD Demo");
  delay(1000u);

  this->command_(COMMAND_DISPLAY_FIRMWARE_VERSION);
  delay(1000u);

  this->command_(COMMAND_DISPLAY_I2C_ADDRESS);
  delay(1000u);

  this->command_(COMMAND_CLEAR_SCREEN);
}

float NhdChar::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}

void HOT NhdChar::display() {
  uint8_t* buf = this->buffer_;

  for (uint8_t row = 0; row < this->rows_; ++row) {
    for (uint8_t column = 0; column < this->columns_; ++column) {
      this->send(*buf++);
    }
  }
}

void NhdChar::update() {
  this->clear();
  this->call_writer();
  this->display();
}

void NhdChar::command_(Command cmd, uint8_t* params, size_t length) {
  this->send(COMMAND_PREFIX);
  this->send(cmd.cmd);
  while (length--) {
    this->send(*params++);
  };
  delayMicroseconds(cmd.exec_time_us);
}

void NhdChar::command_(Command cmd, uint8_t param1) {
  this->command_(cmd, &param1, 1);
}

void NhdChar::command_(Command cmd) {
  this->command_(cmd, nullptr, 0);
}

void NhdChar::print(uint8_t column, uint8_t row, const char *str) {
  uint8_t pos = row * this->columns_ + column;
  for (; *str != '\0'; str++) {
    if (*str == '\n') {
      pos = ((pos / this->columns_) + 1) * this->columns_;
      continue;
    }

    if (pos >= this->positions_) {
      ESP_LOGW(TAG, "NhdChar writing out of range!");
      break;
    }

    this->buffer_[pos++] = *reinterpret_cast<const uint8_t*>(str);
  }
}

void NhdChar::print(uint8_t column, uint8_t row, const std::string &str) {
  this->print(column, row, str.c_str());
}

void NhdChar::print(const char *str) {
  this->print(0, 0, str);
}

void NhdChar::print(const std::string &str) {
  this->print(0, 0, str.c_str());
}

void NhdChar::printf(uint8_t column, uint8_t row, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0) {
    this->print(column, row, buffer);
  }
}

void NhdChar::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0) {
    this->print(0, 0, buffer);
  }
}

void NhdChar::clear() {
  ::memset(this->buffer_, ' ', this->positions_);
}

#ifdef USE_TIME
void NhdChar::strftime(uint8_t column, uint8_t row, const char *format, time::ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);

  if (ret > 0) {
    this->print(column, row, buffer);
  }
}

void NhdChar::strftime(const char *format, time::ESPTime time) {
  this->strftime(0, 0, format, time);
}
#endif

void NhdChar::backlight() {
  this->command_(COMMAND_SET_BACKLIGHT_BRIGHTNESS, this->backlight_value_);
}

void NhdChar::no_backlight() {
  this->command_(COMMAND_SET_BACKLIGHT_BRIGHTNESS, 1);
}

void NhdChar::set_backlight(uint8_t value) {
  if (value >= 1 && value <= 8) {
    this->backlight_value_ = value;
  }
}

}  // namespace nhd_char_base
}  // namespace esphome
