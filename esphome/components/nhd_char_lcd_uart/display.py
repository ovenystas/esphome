import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import nhd_char_lcd, uart
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["nhd_char_lcd"]

nhd_char_lcd_uart_ns = cg.esphome_ns.namespace("nhd_char_lcd_uart")
NhdCharLcdUart = nhd_char_lcd_uart_ns.class_(
    "NhdCharLcdUart", nhd_char_lcd.Lcd, uart.UARTDevice
)

CONFIG_SCHEMA = nhd_char_lcd.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(NhdCharLcdUart),
    }
).extend(uart.UART_DEVICE_SCHEMA)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "nhd_char_lcd_uart", baud_rate=9600, require_rx=False
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await nhd_char_lcd.setup_lcd_display(var, config)
    await uart.register_uart_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(NhdCharLcdUart.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
