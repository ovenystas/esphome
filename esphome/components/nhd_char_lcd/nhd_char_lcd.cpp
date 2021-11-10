#include "nhd_char_lcd.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include <cstring>

namespace esphome {
namespace nhd_char_lcd {

static const char *const TAG = "lcd";

struct Command {
  uint8_t cmd;            // Command byte
  uint16_t exec_time_us;  // Execution time in us
};

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
static const Command COMMAND_DISPLAY_RS232_BAUD_RATE = { 0x71, 10000u };
static const Command COMMAND_DISPLAY_I2C_ADDRESS = { 0x72, 4000u };

void NhdCharLcd::setup() {
  this->buffer_ = new uint8_t[this->positions_];  // NOLINT
  this->clear();

  // Commands can only be sent 100ms after boot-up, so let's wait if not passed
  const uint8_t now = millis();
  if (now < 100) {
    delay(100u - now);
  }

  if (!this->command_(COMMAND_DISPLAY_ON)) {
    ESP_LOGE(TAG, "Communication with Newhaven LCD failed!");
    this->mark_failed();
    return;
  }

  this->clear_screen();
  this->set_contrast(40u);
  this->set_backlight(8u);
  this->underline_cursor_off();

  this->print("NewHaven Display\n"
              "Serial LCD Demo");
  delay(1000u);

  this->display_firmware_version();
  delay(1000u);

  this->display_i2c_address();
  delay(1000u);

  this->clear_screen();
}

float NhdCharLcd::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}

void HOT NhdCharLcd::display() {
  this->send(this->buffer_, this->positions_);
}

void NhdCharLcd::update() {
  this->clear();
  this->call_writer();
  this->display();
}

bool NhdCharLcd::command_(Command cmd, uint8_t* params, size_t length) {
  if (!this->send_command(cmd.cmd, params, static_cast<uint8_t>(length))) {
    return false;
  }
  delayMicroseconds(cmd.exec_time_us);
  return true;
}

bool NhdCharLcd::command_(Command cmd, uint8_t param1) {
  return this->command_(cmd, &param1, 1);
}

bool NhdCharLcd::command_(Command cmd) {
  return this->command_(cmd, nullptr, 0);
}

void NhdCharLcd::print(uint8_t column, uint8_t row, const char *str) {
  uint8_t pos = row * this->columns_ + column;
  for (; *str != '\0'; str++) {
    if (*str == '\n') {
      pos = ((pos / this->columns_) + 1) * this->columns_;
      continue;
    }

    if (pos >= this->positions_) {
      ESP_LOGW(TAG, "NhdCharLcd writing out of range!");
      break;
    }

    this->buffer_[pos++] = *reinterpret_cast<const uint8_t*>(str);
  }
}

void NhdCharLcd::print(uint8_t column, uint8_t row, const std::string &str) {
  this->print(column, row, str.c_str());
}

void NhdCharLcd::print(const char *str) {
  this->print(0, 0, str);
}

void NhdCharLcd::print(const std::string &str) {
  this->print(0, 0, str.c_str());
}

void NhdCharLcd::printf(uint8_t column, uint8_t row, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0) {
    this->print(column, row, buffer);
  }
}

void NhdCharLcd::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0) {
    this->print(0, 0, buffer);
  }
}

void NhdCharLcd::clear() {
  ::memset(this->buffer_, ' ', this->positions_);
}

#ifdef USE_TIME
void NhdCharLcd::strftime(uint8_t column, uint8_t row, const char *format, time::ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);

  if (ret > 0) {
    this->print(column, row, buffer);
  }
}

void NhdCharLcd::strftime(const char *format, time::ESPTime time) {
  this->strftime(0, 0, format, time);
}
#endif

void NhdCharLcd::display_on(void) {
  this->command_(COMMAND_DISPLAY_ON);
}

void NhdCharLcd::display_off(void) {
  this->command_(COMMAND_DISPLAY_OFF);
}

/**
 * row 1 to number-of-rows
 * column 1 to number-of-columns
 */
void NhdCharLcd::set_cursor(uint8_t row, uint8_t column) {
  if (row > 0 && row <= this->rows_ &&
      column > 0 && column <= this->columns_) {
    uint8_t pos = (row - 1) * 0x40u + (column - 1);
    this->command_(COMMAND_SET_CURSOR, pos);
  }
}

void NhdCharLcd::cursor_home() {
  this->command_(COMMAND_CURSOR_HOME);
}

void NhdCharLcd::underline_cursor_on() {
  this->command_(COMMAND_UNDERLINE_CURSOR_ON);
}

void NhdCharLcd::underline_cursor_off() {
  this->command_(COMMAND_UNDERLINE_CURSOR_OFF);
}

void NhdCharLcd::move_cursor_left() {
  this->command_(COMMAND_MOVE_CURSOR_LEFT);
}

void NhdCharLcd::move_cursor_right() {
  this->command_(COMMAND_MOVE_CURSOR_RIGHT);
}

void NhdCharLcd::blinking_cursor_on() {
  this->command_(COMMAND_BLINKING_CURSOR_ON);
}

void NhdCharLcd::blinking_cursor_off() {
  this->command_(COMMAND_BLINKING_CURSOR_OFF);
}

void NhdCharLcd::backspace() {
  this->command_(COMMAND_BACKSPACE);
}

void NhdCharLcd::clear_screen() {
  this->command_(COMMAND_CLEAR_SCREEN);
}

/**
 * contrast 1-50
 */
void NhdCharLcd::set_contrast(uint8_t contrast) {
  if (contrast >= 1 && contrast <= 50) {
    this->command_(COMMAND_SET_CONTRAST, contrast);
  }
}

void NhdCharLcd::backlight_on() {
  this->command_(COMMAND_SET_BACKLIGHT_BRIGHTNESS, this->backlight_value_);
}

void NhdCharLcd::backlight_off() {
  this->command_(COMMAND_SET_BACKLIGHT_BRIGHTNESS, 1);
}

void NhdCharLcd::set_backlight(uint8_t value) {
  if (value >= 1 && value <= 8) {
    this->backlight_value_ = value;
    this->backlight_on();
  }
}

void NhdCharLcd::load_custom_character(uint8_t addr,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  uint8_t character[8] = { d0, d1, d2, d3, d4, d5, d6, d7 };
  this->command_(COMMAND_LOAD_CUSTOM_CHARACTER, character, 9);
}

void NhdCharLcd::move_display_left() {
  this->command_(COMMAND_MOVE_DISPLAY_LEFT);
}

void NhdCharLcd::move_display_right() {
  this->command_(COMMAND_MOVE_DISPLAY_RIGHT);
}

/**
 * baud_rate_id 1-8 maps to one of
 *              300, 1200, 2400, 9600, 14400, 19200, 57600, 115200
 */
void NhdCharLcd::change_rs232_baud_rate(uint8_t baud_rate_id) {
  if (baud_rate_id >= 1 && baud_rate_id <= 8) {
    this->command_(COMMAND_CHANGE_RS232_BAUD_RATE, baud_rate_id);
  }
}

/**
 * baud_rate 300, 1200, 2400, 9600, 14400, 19200, 57600 or 115200
 */
void NhdCharLcd::change_rs232_baud_rate(uint32_t baud_rate) {
  uint8_t id;
  if (baud_rate == 300) { id = 1; }
  else if (baud_rate == 1200) { id = 2; }
  else if (baud_rate == 2400) { id = 3; }
  else if (baud_rate == 9600) { id = 4; }
  else if (baud_rate == 14400) { id = 5; }
  else if (baud_rate == 19200) { id = 6; }
  else if (baud_rate == 57600) { id = 7; }
  else if (baud_rate == 115200) { id = 8; }
  else { return; }
  this->command_(COMMAND_CHANGE_RS232_BAUD_RATE, id);
}

/**
 * addr must be an even number
 */
void NhdCharLcd::change_i2c_address(uint8_t addr) {
  if ((addr & 0x01u) == 0) {
    this->command_(COMMAND_CHANGE_I2C_ADDRESS, addr);
  }
}

void NhdCharLcd::display_firmware_version() {
  this->command_(COMMAND_DISPLAY_FIRMWARE_VERSION);
}

void NhdCharLcd::display_rs232_baud_rate() {
  this->command_(COMMAND_DISPLAY_RS232_BAUD_RATE);
}

void NhdCharLcd::display_i2c_address() {
  this->command_(COMMAND_DISPLAY_I2C_ADDRESS);
}

}  // namespace nhd_char_lcd
}  // namespace esphome
