#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome {
namespace nhd_char_lcd {

struct Command;

class NhdCharLcd : public PollingComponent {
 public:
  virtual ~NhdCharLcd() { }

  void set_dimensions(uint8_t columns, uint8_t rows) {
    this->columns_ = columns;
    this->rows_ = rows;
    this->positions_ = columns * rows;
  }

  void setup() override;
  float get_setup_priority() const override;
  void update() override;

  void display();

  //// Clear LCD display
  void clear();

  /// Print the given text at the specified column and row.
  void print(uint8_t column, uint8_t row, const char *str);

  /// Print the given string at the specified column and row.
  void print(uint8_t column, uint8_t row, const std::string &str);

  /// Print the given text at column=0 and row=0.
  void print(const char *str);

  /// Print the given string at column=0 and row=0.
  void print(const std::string &str);

  /// Evaluate the printf-format and print the text at the specified column and row.
  void printf(uint8_t column, uint8_t row, const char *format, ...)
      __attribute__((format(printf, 4, 5)));

  /// Evaluate the printf-format and print the text at column=0 and row=0.
  void printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

#ifdef USE_TIME
  /// Evaluate the strftime-format and print the text at the specified column and row.
  void strftime(uint8_t column, uint8_t row, const char *format, time::ESPTime time)
      __attribute__((format(strftime, 4, 0)));

  /// Evaluate the strftime-format and print the text at column=0 and row=0.
  void strftime(const char *format, time::ESPTime time)
      __attribute__((format(strftime, 2, 0)));
#endif

  void display_on(void);
  void display_off(void);
  /**
   * row 1 to number-of-rows
   * column 1 to number-of-columns
   */
  void set_cursor(uint8_t row, uint8_t column);
  void cursor_home();
  void underline_cursor_on();
  void underline_cursor_off();
  void move_cursor_left();
  void move_cursor_right();
  void blinking_cursor_on();
  void blinking_cursor_off();
  void backspace();
  void clear_screen();
  /**
   * contrast 1-50
   */
  void set_contrast(uint8_t contrast);
  void backlight_on();
  void backlight_off();
  /**
   *  Valid brightness values are 1-8, default = 8
   *  1 = Backlight OFF, 8 = Backlight ON (100%)
   */
  void set_backlight(uint8_t value);
  void load_custom_character(uint8_t addr,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  void move_display_left();
  void move_display_right();
  /**
   * baud_rate_id 1-8 maps to one of
   *              300, 1200, 2400, 9600, 14400, 19200, 57600, 115200
   */
  void change_rs232_baud_rate(uint8_t baud_rate_id);
  /**
   * baud_rate 300, 1200, 2400, 9600, 14400, 19200, 57600 or 115200
   */
  void change_rs232_baud_rate(uint32_t baud_rate);
  /**
   * addr must be an even number
   */
  void change_i2c_address(uint8_t addr);
  void display_firmware_version();
  void display_rs232_baud_rate();
  void display_i2c_address();

  bool command_(Command cmd);
  bool command_(Command cmd, uint8_t param1);
  bool command_(Command cmd, uint8_t* params, size_t length);

 protected:
  bool send(uint8_t* data, uint8_t len) = 0;
  bool send_command(uint8_t cmd, uint8_t* data, uint8_t len) = 0;
  virtual void call_writer() = 0;


  uint8_t columns_;
  uint8_t rows_;
  uint8_t positions_;
  uint8_t *buffer_ { nullptr };
  // Stores the current state of the backlight.
  uint8_t backlight_value_ { 8 };  // 1-8, 1=OFF, 2=Lowest brightness, 8=Highest brightness
};

}  // namespace nhd_char_lcd
}  // namespace esphome
