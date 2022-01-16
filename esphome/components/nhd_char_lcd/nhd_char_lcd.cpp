#include "nhd_char_lcd.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include <cstring>
#include <unordered_map>

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

// Table for mapping Unicode to Newhaven Display character set
static const  std::unordered_map<uint32_t, uint8_t> nhd_char_map = {
    { 0x00a5, 0x5c },  // Yen sign: ¥
    { 0x2192, 0x7e },  // Rightwards arrow: →
    { 0x2190, 0x7f },  // Leftwards arrow: ←
    { 0x2080, 0xa1 },  // Subscript zero: ₀
    { 0x30fb, 0xa5 },  // Katakana middle dot: ・
    { 0x30f2, 0xa6 },  // Katakana letter wo: ヲ
    { 0x30a1, 0xa7 },  // Katakana letter small a: ァ
    { 0x30a3, 0xa8 },  // Katakana letter small i: ィ
    { 0x30a5, 0xa9 },  // Katakana letter small u: ゥ
    { 0x30a7, 0xaa },  // Katakana letter small e: ェ
    { 0x30a9, 0xab },  // Katakana letter small o: ォ
    { 0x30f5, 0xac },  // Katakana letter small ka: ヵ
    { 0x30e5, 0xad },  // Katakana letter small yu: ュ
    { 0x30e7, 0xae },  // Katakana letter small yo: ョ
    { 0x30c3, 0xaf },  // Katakana letter small tu: ッ
    { 0x30fc, 0xb0 },  // Katakana-hiragana prolonged sound mark: ー
    { 0x30a2, 0xb1 },  // Katakana letter a: ア
    { 0x30a4, 0xb2 },  // Katakana letter i: イ
    { 0x30a6, 0xb3 },  // Katakana letter u: ウ
    { 0x30a8, 0xb4 },  // Katakana letter e: エ
    { 0x30aa, 0xb5 },  // Katakana letter o: オ
    { 0x30ab, 0xb6 },  // Katakana letter ka: カ
    { 0x30ad, 0xb7 },  // Katakana letter ki: キ
    { 0x30af, 0xb8 },  // Katakana letter ku: ク
    { 0x30b1, 0xb9 },  // Katakana letter ke: ケ
    { 0x30b3, 0xba },  // Katakana letter ko: コ
    { 0x30b5, 0xbb },  // Katakana letter sa: サ
    { 0x30b7, 0xbc },  // Katakana letter si: シ
    { 0x30b9, 0xbd },  // Katakana letter su: ス
    { 0x30bb, 0xbe },  // Katakana letter se: セ
    { 0x30bd, 0xbf },  // Katakana letter so: ソ
    { 0x30bf, 0xc0 },  // Katakana letter ta: タ
    { 0x30c1, 0xc1 },  // Katakana letter ti: チ
    { 0x30c4, 0xc2 },  // Katakana letter tu: ツ
    { 0x30c6, 0xc3 },  // Katakana letter te: テ
    { 0x30c8, 0xc4 },  // Katakana letter to: ト
    { 0x30ca, 0xc5 },  // Katakana letter na: ナ
    { 0x30cb, 0xc6 },  // Katakana letter ni: ニ
    { 0x30cc, 0xc7 },  // Katakana letter nu: ヌ
    { 0x30cd, 0xc8 },  // Katakana letter ne: ネ
    { 0x30ce, 0xc9 },  // Katakana letter no: ノ
    { 0x30cf, 0xca },  // Katakana letter ha: ハ
    { 0x30d2, 0xcb },  // Katakana letter hi: ヒ
    { 0x30d5, 0xcc },  // Katakana letter hu: フ
    { 0x30d8, 0xcd },  // Katakana letter he: ヘ
    { 0x30db, 0xce },  // Katakana letter ho: ホ
    { 0x30de, 0xcf },  // Katakana letter ma: マ
    { 0x30df, 0xd0 },  // Katakana letter mi: ミ
    { 0x30e0, 0xd1 },  // Katakana letter mu: ム
    { 0x30e1, 0xd2 },  // Katakana letter me: メ
    { 0x30e2, 0xd3 },  // Katakana letter mo: モ
    { 0x30e4, 0xd4 },  // Katakana letter ya: ヤ
    { 0x30e6, 0xd5 },  // Katakana letter yu: ユ
    { 0x30e8, 0xd6 },  // Katakana letter yo: ヨ
    { 0x30e9, 0xd7 },  // Katakana letter ra: ラ
    { 0x30ea, 0xd8 },  // Katakana letter ri: リ
    { 0x30eb, 0xd9 },  // Katakana letter ru: ル
    { 0x30ec, 0xda },  // Katakana letter re: レ
    { 0x30ed, 0xdb },  // Katakana letter ro: ロ
    { 0x30ef, 0xdc },  // Katakana letter wa: ワ
    { 0x30f3, 0xdd },  // Katakana letter n: ン
    { 0x03b1, 0xe0 },  // Greek small letter alpha: α
    { 0x00e4, 0xe1 },  // Latin small letter a with diaeresis: ä
    { 0x03b2, 0xe2 },  // Greek small letter beta: β
    { 0x03b5, 0xe3 },  // Greek small letter epsilon: ε
    { 0x00b5, 0xe4 },  // Micro sign: µ
    { 0x03bc, 0xe4 },  // Greek small letter mu: μ
    { 0x03C3, 0xe5 },  // Greek small letter sigma: σ
    { 0x03c1, 0xe6 },  // Greek small letter rho: ρ
    { 0x221a, 0xe8 },  // Square root: √
    { 0x02e3, 0xe9 },  // Modifier letter small x: ˣ
    { 0x00f6, 0xef },  // Latin small letter o with diaeresis: ö
    { 0x03b8, 0xf2 },  // Greek small letter theta: θ
    { 0x221e, 0xf3 },  // Infinity: ∞
    { 0x03a9, 0xf4 },  // Greek capital letter omega: Ω
    { 0x00fc, 0xf5 },  // Latin small letter u with diaeresis: ü
    { 0x03a3, 0xf6 },  // Greek capital letter sigma: Σ
    { 0x03C0, 0xf7 },  // Greek small letter pi: π
    { 0x00f7, 0xfd },  // Division sign: ÷
    { 0x2588, 0xff },  // Full block: █
};

void NhdCharLcd::set_dimensions(uint8_t columns, uint8_t rows) {
  if (columns <= COLUMNS_MAX && rows <= ROWS_MAX) {
    this->columns_ = columns;
    this->rows_ = rows;
    this->positions_ = columns * rows;
    ESP_LOGD(TAG, "set_dimensions: %ux%u", columns, rows);
  } else {
    ESP_LOGW(TAG, "set_dimensions, out of range!");
  }
}

void NhdCharLcd::setup() {
  this->buffer_ = new uint8_t[this->positions_];
  ESP_LOGD(TAG, "setup, Buffer of size %u created", this->positions_);
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

  this->load_all_custom_characters();
  this->clear_screen();
  ESP_LOGD(TAG, "setup, Done");
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
  ESP_LOGD(TAG, "update, Begin");
  this->do_update_();
  this->display();
  ESP_LOGD(TAG, "update, End");
}

bool NhdCharLcd::command_(Command cmd, uint8_t* params, size_t length) {
  uint8_t param = 0;
  if (params != nullptr) {
    param = params[0];
  }
//  ESP_LOGD(TAG,
//      "command_, cmd=%u,%u, params=0x%02x",
//      cmd.cmd, cmd.exec_time_us, param);
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
 * It tries to map to custom chars if any has been set with load_custom_character().
 * If a map can't be found it sets the specific char to a space char (blank).
 */
uint8_t NhdCharLcd::unicodeToNhdCode(uint32_t codePoint) {
  // Directly use custom character
  if (codePoint <= 0x0F) {
    return static_cast<uint8_t>(codePoint);
  }

  // Check if codePoint is stored as a custom character
  for (uint8_t i = 0; i < 8; i++) {
    if (codePoint == this->custom_chars[i].unicode) {
      return i;
    }
  }

  // Check if codePoint is in NHD extended character set
  auto it = nhd_char_map.find(codePoint);
  if (it != nhd_char_map.end()) {
    return it->second;
  }

  // These are blank in NHD char map
  if (codePoint >= 0x10 && codePoint <= 0x1F) {
    return '?';
  }

  // These are almost ASCII with three exceptions
  if (codePoint >= 0x20 && codePoint <= 0x7F) {
    return static_cast<uint8_t>(codePoint);
  }

  // These are blank in NHD char map
  if (codePoint >= 0x80 && codePoint <= 0x9F) {
    return '?';
  }

  // Directly use NHD extended character set
  if (codePoint >= 0xA0 && codePoint <= 0xFF) {
    return '?';
  }

  // Could not determine character
  return '?';
}

/**
 * Decode 1-4 bytes in string to it's unicode representation.
 * Sets codePoint to decoded unicode value.
 * Returns: Number of bytes used up in string, 0 if invalid UTF-8 string.
 */
uint8_t NhdCharLcd::utf8Decode(const char* str, uint32_t* codePoint) {
  uint32_t cp = '?'; // Default to space char ('?' during debug)
  uint8_t num_bytes = 0;

  if ((str[0] & 0x80) == 0x00) {
    // Single byte UTF-8 code
    cp = str[0];
    num_bytes = 1;
  } else if ((str[0] & 0xE0) == 0xC0 && (str[1] & 0xC0) == 0x80) {
    // 2-byte of UTF-8 code
    cp = ((str[0] & 0x1F) << 6) + (str[1] & 0x3F);
    num_bytes = 2;
  } else if ((str[0] & 0xF0) == 0xE0 && (str[1] & 0xC0) == 0x80 && (str[2] & 0xC0) == 0x80) {
    // 3-byte of UTF-8 code
    cp = ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) << 6) + (str[2] & 0x3F);
    num_bytes = 3;
  } else if ((str[0] & 0xF8) == 0xF0 && (str[1] & 0xC0) == 0x80 && (str[2] & 0xC0) == 0x80 && (str[3] & 0xC0) == 0x80) {
    // 4-byte of UTF-8 code
    cp = ((str[0] & 0x07) << 18) + ((str[1] & 0x3F) << 12) + ((str[2] & 0x3F) << 6) + (str[3] & 0x3F);
    num_bytes = 4;
  } else {
    // Invalid UTF-8 code
    ESP_LOGW(TAG, "utf8Decode, Invalid UTF-8 chars 0x%02x%02x%02x%02x",
        str[0], str[1], str[2], str[3]);
    return 0;
  }

//  ESP_LOGD(TAG, "utf8Decode, 0x%02x%02x%02x%02x -> U%08X using %u bytes",
//      str[0], str[1], str[2], str[3], cp, num_bytes);

  *codePoint = cp;

  return num_bytes;
}

void NhdCharLcd::print(uint8_t column, uint8_t row, const char* str) {
  ESP_LOGD(TAG, "print, \"%s\" at pos %u,%u", str, column, row);
  if (this->buffer_ == nullptr) {
    ESP_LOGE(TAG, "buffer_ is not allocated!");
    return;
  }
  uint8_t pos = row * this->columns_ + column;
  while (*str != '\0') {
    if (*str == '\n') {
      pos = ((pos / this->columns_) + 1) * this->columns_;
      ++str;
      continue;
    }

    if (pos >= this->positions_) {
      ESP_LOGW(TAG, "print, out of range!");
      break;
    }

    uint32_t codePoint;
    uint8_t decodedBytes = utf8Decode(str, &codePoint);

    if (decodedBytes == 0) {
      ESP_LOGW(TAG, "print, Could not decode utf-8 string!");
      break;
    }

    uint8_t c = unicodeToNhdCode(codePoint);

//    ESP_LOGD(TAG, "print, Putting U%08X 0x%02X %c in buffer_[%u]",
//        codePoint, c, (c >= 20 && c < 128) ? static_cast<char>(c) : '?', pos);
    this->buffer_[pos] = c;

    str += decodedBytes;
    ++pos;
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

void NhdCharLcd::printf(uint8_t column, uint8_t row, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0) {
    this->print(column, row, buffer);
  }
}

void NhdCharLcd::printf(const char* format, ...) {
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

void NhdCharLcd::set_custom_character(uint8_t addr, uint32_t unicode,
    uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  if (addr < 8) {
//    ESP_LOGD(TAG, "set_custom_character, %u %08x,"
//        " 0x%02x %02x %02x %02x %02x %02x %02x %02x",
//        addr, unicode, d0, d1, d2, d3, d4, d5, d6, d7);

    struct CustomChar* cc = &this->custom_chars[addr];
    cc->unicode = unicode;
    cc->pixel_data[0] = d0;
    cc->pixel_data[1] = d1;
    cc->pixel_data[2] = d2;
    cc->pixel_data[3] = d3;
    cc->pixel_data[4] = d4;
    cc->pixel_data[5] = d5;
    cc->pixel_data[6] = d6;
    cc->pixel_data[7] = d7;
  } else {
    ESP_LOGW(TAG, "set_custom_character, addr out of range!");
  }
}

void NhdCharLcd::load_custom_character(uint8_t idx) {
  if (idx < 8) {
    struct CustomChar* cc = &this->custom_chars[idx];

//    ESP_LOGD(TAG, "load_custom_character, %u %08x,"
//        " 0x%02x %02x %02x %02x %02x %02x %02x %02x",
//        idx, cc->unicode,
//        cc->pixel_data[0],
//        cc->pixel_data[1],
//        cc->pixel_data[2],
//        cc->pixel_data[3],
//        cc->pixel_data[4],
//        cc->pixel_data[5],
//        cc->pixel_data[6],
//        cc->pixel_data[7]);

    uint8_t character[9];
    character[0] = idx;
    memcpy(&character[1], cc->pixel_data, 8);
    this->command_(COMMAND_LOAD_CUSTOM_CHARACTER, character, 9);
  } else {
    ESP_LOGW(TAG, "load_custom_character, idx out of range!");
  }
}

void NhdCharLcd::load_all_custom_characters() {
  for (uint8_t i = 0; i < 8; i++) {
    if (this->custom_chars[i].unicode != 0) {
      this->load_custom_character(i);
    }
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
  ESP_LOGI(TAG, "change_rs232_baud_rate, Changed baud rate to %u", baud_rate);
}

void NhdCharLcd::change_i2c_address(uint8_t addr) {
  if ((addr & 0x01u) == 0) {
    this->command_(COMMAND_CHANGE_I2C_ADDRESS, addr);
    ESP_LOGI(TAG, "change_i2c_address, Changed I2C address to 0x%02X", addr);
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
