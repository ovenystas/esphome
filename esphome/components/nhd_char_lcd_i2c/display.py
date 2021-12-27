import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import nhd_char_lcd, i2c
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["nhd_char_lcd"]

nhd_char_lcd_i2c_ns = cg.esphome_ns.namespace("nhd_char_lcd_i2c")
NhdCharLcdI2c = nhd_char_lcd_i2c_ns.class_(
    "NhdCharLcdI2c", nhd_char_lcd.NhdCharLcd, i2c.I2CDevice
)

CONFIG_SCHEMA = nhd_char_lcd.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(NhdCharLcdI2c),
    }
).extend(i2c.i2c_device_schema(0x28))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await i2c.register_i2c_device(var, config)
    await nhd_char_lcd.setup_lcd_display(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(NhdCharLcdI2c.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
