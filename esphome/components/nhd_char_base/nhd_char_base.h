#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome {
namespace nhd_char_base {

struct Command;

class NhdChar : public PollingComponent {
 public:
  virtual ~NhdChar() { }

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

  void backlight();
  void no_backlight();

  // Valid brightness values are 1-8, default = 8
  // 1 = Backlight OFF, 8 = Backlight ON (100%)
  void set_backlight(uint8_t value);

  void command_(Command cmd);
  void command_(Command cmd, uint8_t param1);
  void command_(Command cmd, uint8_t* params, size_t length);

 protected:
  virtual void send(uint8_t value) = 0;
  virtual void call_writer() = 0;


  uint8_t columns_;
  uint8_t rows_;
  uint8_t positions_;
  uint8_t *buffer_ { nullptr };
  uint8_t backlight_value_ { 8 };
};

}  // namespace nhd_char_base
}  // namespace esphome
