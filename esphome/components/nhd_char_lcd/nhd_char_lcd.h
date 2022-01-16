#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/display/display_buffer.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome {
namespace nhd_char_lcd {

struct Command;

class NhdCharLcd : public PollingComponent,
                   public display::DisplayBuffer {
 public:
  virtual ~NhdCharLcd() { }

  // Set screen dimensions.
  void set_dimensions(uint8_t columns, uint8_t rows);

  void setup() override;

  float get_setup_priority() const override;

  void update() override;

  void display();

  // Clear screen buffer.
  void clear_buffer();

  // Print the given text at the specified column and row.
  void print(uint8_t column, uint8_t row, const char* str);

  // Print the given string at the specified column and row.
  void print(uint8_t column, uint8_t row, const std::string &str);

  // Print the given text at column=0 and row=0.
  void print(const char* str);

  // Print the given string at column=0 and row=0.
  void print(const std::string &str);

  // Evaluate the printf-format and print the text at the specified column and row.
  void printf(uint8_t column, uint8_t row, const char *format, ...)
      __attribute__((format(printf, 4, 5)));

  // Evaluate the printf-format and print the text at column=0 and row=0.
  void printf(const char* format, ...)
      __attribute__((format(printf, 2, 3)));

#ifdef USE_TIME
  // Evaluate the strftime-format and print the text at the specified column and row.
  void strftime(uint8_t column, uint8_t row, const char *format, time::ESPTime time)
      __attribute__((format(strftime, 4, 0)));

  // Evaluate the strftime-format and print the text at column=0 and row=0.
  void strftime(const char *format, time::ESPTime time)
      __attribute__((format(strftime, 2, 0)));
#endif

  // Turn the display on. Default: on.
  void display_on(void);

  // Turn the display off. Default: on.
  void display_off(void);

  // Put cursor at location specified by column and row.
  // column 0 to (number-of-columns - 1).
  // row 0 to (number-of-rows - 1).
  void set_cursor(uint8_t column, uint8_t row);

  // Position cursor at column=0, row=0.
  void cursor_home();

  // Turn on underline cursor. Default: off.
  void underline_cursor_on();

  // Turn off underline cursor. Default: off.
  void underline_cursor_off();

  // Move cursor left one step without altering the text.
  void move_cursor_left();

  // Move cursor left one step without altering the text.
  void move_cursor_right();

  // Turn on blinking cursor. Default: off.
  void blinking_cursor_on();

  // Turn off blinking cursor. Default: off.
  void blinking_cursor_off();

  // Move cursor back one space, delete last character.
  void backspace();

  // Clear screen and move cursor to home position at column=0, row=0.
  void clear_screen();

  // Set display contrast, value from 1 to 50.
  void set_contrast(uint8_t contrast);

  // Turn on backlight at previously set brightness.
  // Default: on at max brightness.
  void backlight_on();

  // Turn off backlight.
  // Default: on at max brightness.
  void backlight_off();

  // Set brightness of backlight.
  // Valid brightness values are 1 to 8. 1=Backlight OFF, 8=Backlight ON (100%).
  // Default: 8 (max brightness).
  void set_backlight(uint8_t value);

  // Sets a custom character defined by bit map data in d0 to d7 into space
  // given by addr. 8 custom characters can be loaded.
  // Valid values for addr is 0 to 7.
  // Also stores the corresponding unicode value for auto encoding when printing.
  // If no unicode mapping shall be done set unicode=0.
  // Example: '¿' (Spanish inverted question mark) is:
  //   unicode=0x00BF d0-d7=0x04, 0x00, 0x04, 0x08, 0x10, 0x11, 0x0E, 0x00.
  void set_custom_character(uint8_t addr, uint32_t unicode,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  // Move the whole display left one step.
  void move_display_left();

  // Move the whole display right one step.
  void move_display_right();

  // Set the RS-232 Baud rate.
  // valid baud_rates are:
  //   300, 1200, 2400, 9600, 14400, 19200, 57600 or 115200
  void change_rs232_baud_rate(uint32_t baud_rate);


  // Sets the I2C address. addr must be an even number.
  void change_i2c_address(uint8_t addr);

  // Display the firmware version number on screen.
  void display_firmware_version();

  // Display the current RS-232 Baud Rate on screen.
  void display_rs232_baud_rate();

  // Display the current I2C address on screen.
  void display_i2c_address();


 protected:
  // Loads a custom character into LCD memory.
  void load_custom_character(uint8_t idx);

  // Loads all set custom characters into LCD memory.
  void load_all_custom_characters();

  // Sends a command with no parameters.
  bool command_(Command cmd);

  // Sends a command with one parameter.
  bool command_(Command cmd, uint8_t param1);

  // Sends a command with parameters in a given buffer.
  bool command_(Command cmd, uint8_t* params, size_t length);

  uint8_t unicodeToNhdCode(uint32_t codePoint);
  uint8_t utf8Decode(const char* str, uint32_t* codePoint);

  // Interface for sending a buffer with data bytes.
  virtual bool send(uint8_t* data, uint8_t len) = 0;

  // Interface for sending a command along with a buffer with data bytes.
  virtual bool send_command(uint8_t cmd, uint8_t* data, uint8_t len) = 0;

  // Interface for calling writer.
  virtual void call_writer() = 0;

  struct CustomChar {
    uint32_t unicode;
    uint8_t pixel_data[8];
  };

  uint8_t columns_ { };
  uint8_t rows_ { };
  uint8_t positions_ { };
  uint8_t* buffer_ { };

  // Stores the current value of the backlight.
  uint8_t backlight_value_ { 8 };  // 1-8, 1=OFF, 2=Lowest brightness, 8=Highest brightness

  // Holds pexel_data for custom chars.
  struct CustomChar custom_chars[8] { };
};

}  // namespace nhd_char_lcd
}  // namespace esphom
