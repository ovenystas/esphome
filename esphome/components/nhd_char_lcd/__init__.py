import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display

# from esphome.const import CONF_DIMENSIONS, CONF_CUSTOM_CHARS, CONF_CHAR, CONF_PIXEL_DATA
from esphome.const import CONF_DIMENSIONS

CONF_CUSTOM_CHARS = "custom_chars"
CONF_CHAR = "char"
CONF_PIXEL_DATA = "pixel_data"

nhd_char_lcd_ns = cg.esphome_ns.namespace("nhd_char_lcd")
NhdCharLcd = nhd_char_lcd_ns.class_("LCDDisplay", cg.PollingComponent)


def validate_lcd_dimensions(value):
    value = cv.dimensions(value)
    if value[0] > 0x40:
        raise cv.Invalid("LCD displays can't have more than 64 columns")
    if value[1] > 4:
        raise cv.Invalid("LCD displays can't have more than 4 rows")
    return value


LCD_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.Required(CONF_DIMENSIONS): validate_lcd_dimensions,
        cv.Optional(CONF_CUSTOM_CHARS): cv.All(
            cv.ensure_list(
                {
                    cv.Required(CONF_CHAR): cv.string_strict,
                    cv.Required(CONF_PIXEL_DATA): cv.All(
                        cv.ensure_list(
                            cv.hex_int,
                        ),
                        cv.Length(min=8, max=8),
                    ),
                }
            ),
            cv.Length(min=1, max=8),
        ),
    }
).extend(cv.polling_component_schema("1s"))


async def setup_lcd_display(var, config):
    await cg.register_component(var, config)
    await display.register_display(var, config)

    cg.add(var.set_dimensions(config[CONF_DIMENSIONS][0], config[CONF_DIMENSIONS][1]))

    for idx, char in enumerate(config[CONF_CUSTOM_CHARS]):
        print(f"char[CONF_CHAR] = {char[CONF_CHAR]}")
        cg.add(
            var.set_custom_character(
                idx,
                ord(char[CONF_CHAR]),
                char[CONF_PIXEL_DATA][0],
                char[CONF_PIXEL_DATA][1],
                char[CONF_PIXEL_DATA][2],
                char[CONF_PIXEL_DATA][3],
                char[CONF_PIXEL_DATA][4],
                char[CONF_PIXEL_DATA][5],
                char[CONF_PIXEL_DATA][6],
                char[CONF_PIXEL_DATA][7],
            )
        )
