#include "nhd_char_lcd.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include <cstring>

namespace esphome {
namespace nhd_char_lcd {

static const char *const TAG = "nhd_char_lcd";

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

static const uint16_t PRINT_EXEC_TIME_US = 100u;
static const uint8_t COLUMNS_MAX = 64u;
static const uint8_t ROWS_MAX = 4u;

void NhdCharLcd::set_dimensions(uint8_t columns, uint8_t rows) {
  if (columns <= COLUMNS_MAX && rows <= ROWS_MAX) {
    this->columns_ = columns;
    this->rows_ = rows;
    this->positions_ = columns * rows;
  } else {
    ESP_LOGW(TAG, "NhdCharLcd set_dimensions, out of range!");
  }
}

void NhdCharLcd::setup() {
  this->buffer_ = new uint8_t[this->positions_];  // NOLINT
  this->clear_buffer();

  // Commands can only be sent 100ms after boot-up, so let's wait if not passed
  const uint32_t now = millis();
  if (now < 100u) {
    delay(100u - now);
  }

  if (!this->command_(COMMAND_DISPLAY_ON)) {
    ESP_LOGE(TAG, "Communication with Newhaven LCD failed!");
    this->mark_failed();
    return;
  }

  this->clear_screen();
}

float NhdCharLcd::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}

void HOT NhdCharLcd::display() {
  for (uint8_t row = 0; row < this->rows_; ++row) {
    this->set_cursor(0, row);
    this->send(&this->buffer_[row * this->columns_], this->columns_);
  }
  delayMicroseconds(PRINT_EXEC_TIME_US);
}

void NhdCharLcd::update() {
  this->clear_buffer();
  this->call_writer();
  this->display();
}

bool NhdCharLcd::command_(Command cmd, uint8_t* params, size_t length) {
  uint8_t param = 0;
  if (params != nullptr) {
    param = params[0];
  }
  ESP_LOGD(TAG,
      "command_, cmd=%u,%u, params=0x%02x",
      cmd.cmd, cmd.exec_time_us, param);
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

/**
 * 0x00-0x0F are reserved for custom chars.
 * 0x10-0x1F are blank.
 * 0x20-0x7F are as ASCII but with these three exceptions:
 * - Backslash '\' U005C is replaced by Yen '¥' U00A5
 * - Tilde '~' U007E is replaced by Right arrow '→' U2192
 * - DEL U007F is replaced by Left arrow '←' U2190
 * 0x80-0x9F are blank.
 * 0xA0-0xFF are New Haven specific char map.
 *
 * This function tries to convert from UTF-8 and map to New Haven's font table.
 * It tries to map to custom chars if any has been set with.
 * If a map can't be found it sets the specific char to a space char (blank).
 */
void NhdCharLcd::utf8ToNhdEncode(const char *in_str, char *out_str) {
  ESP_LOGD(TAG, "utf8ToNhdEncode, in=\"%s\", out=\"%s\"", in_str, out_str);
}

void NhdCharLcd::print(uint8_t column, uint8_t row, const char *str) {
  ESP_LOGD(TAG, "print, \"%s\" of length %u at pos %u,%u", str, strlen(str), column, row);
  uint8_t pos = row * this->columns_ + column;
  for (; *str != '\0'; str++) {
    if (*str == '\n') {
      pos = ((pos / this->columns_) + 1) * this->columns_;
      continue;
    }

    if (pos >= this->positions_) {
      ESP_LOGW(TAG, "print, out of range!");
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

void NhdCharLcd::clear_buffer() {
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

void NhdCharLcd::display_on() {
  this->command_(COMMAND_DISPLAY_ON);
}

void NhdCharLcd::display_off() {
  this->command_(COMMAND_DISPLAY_OFF);
}

void NhdCharLcd::set_cursor(uint8_t column, uint8_t row) {
  const uint8_t row_start_value[4] = { 0x00, 0x40, 0x14, 0x54 };

  if (row < this->rows_ && column < this->columns_) {
    uint8_t pos = row_start_value[row] + column;
    this->command_(COMMAND_SET_CURSOR, pos);
  }
  else {
    ESP_LOGW(TAG, "set_cursor, out of range!");
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

void NhdCharLcd::set_contrast(uint8_t contrast) {
  if (contrast >= 1 && contrast <= 50) {
    this->command_(COMMAND_SET_CONTRAST, contrast);
  } else {
    ESP_LOGW(TAG, "set_contrast, out of range!");
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
  } else {
    ESP_LOGW(TAG, "set_backlight, out of range!");
  }
}

void NhdCharLcd::load_custom_character(uint8_t addr, uint32_t unicode,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  if (addr < 8) {
    ESP_LOGD(TAG, "load_custom_character, %u %08x,"
        " 0x%02x %02x %02x %02x %02x %02x %02x %02x",
        addr, unicode, d0, d1, d2, d3, d4, d5, d6, d7);
    uint8_t character[9] = { addr, d0, d1, d2, d3, d4, d5, d6, d7 };
    this->command_(COMMAND_LOAD_CUSTOM_CHARACTER, character, 9);
    this->custom_char[addr] = unicode;
  } else {
    ESP_LOGW(TAG, "load_custom_character, addr out of range!");
  }
}

void NhdCharLcd::move_display_left() {
  this->command_(COMMAND_MOVE_DISPLAY_LEFT);
}

void NhdCharLcd::move_display_right() {
  this->command_(COMMAND_MOVE_DISPLAY_RIGHT);
}

/**
 * baud_rate 300, 1200, 2400, 9600, 14400, 19200, 57600 or 115200
 */
void NhdCharLcd::change_rs232_baud_rate(uint32_t baud_rate) {
  uint8_t id;
  switch (baud_rate) {
    case    300: id = 1; break;
    case   1200: id = 2; break;
    case   2400: id = 3; break;
    case   9600: id = 4; break;
    case  14400: id = 5; break;
    case  19200: id = 6; break;
    case  57600: id = 7; break;
    case 115200: id = 8; break;
    default:
      ESP_LOGW(TAG, "change_rs232_baud_rate, invalid baud rate!");
      return;
  }
  this->command_(COMMAND_CHANGE_RS232_BAUD_RATE, id);
}

void NhdCharLcd::change_i2c_address(uint8_t addr) {
  if ((addr & 0x01u) == 0) {
    this->command_(COMMAND_CHANGE_I2C_ADDRESS, addr);
  } else {
    ESP_LOGW(TAG, "change_i2c_address, invalid address!");
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
