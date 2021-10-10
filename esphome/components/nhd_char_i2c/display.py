import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import nhd_char_base, i2c
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["nhd_char_base"]

nhd_char_i2c_ns = cg.esphome_ns.namespace("nhd_char_i2c")
NhdCharI2cLcd = nhd_char_i2c_ns.class_(
    "NhdCharI2cLcd", nhd_char_base.Lcd, i2c.I2CDevice
)

CONFIG_SCHEMA = nhd_char_base.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(NhdCharI2cLcd),
    }
).extend(i2c.i2c_device_schema(0x28))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await nhd_char_base.setup_lcd_display(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(NhdCharI2cLcd.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
