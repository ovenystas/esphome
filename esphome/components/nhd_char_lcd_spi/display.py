import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import nhd_char_lcd, spi
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ["spi"]
AUTO_LOAD = ["nhd_char_lcd"]

nhd_char_lcd_spi_ns = cg.esphome_ns.namespace("nhd_char_lcd_spi")
NhdCharLcdSpi = nhd_char_lcd_spi_ns.class_(
    "NhdCharLcdSpi", nhd_char_lcd.Lcd, spi.SPIDevice
)

CONFIG_SCHEMA = nhd_char_lcd.LCD_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(NhdCharLcdSpi),
    }
).extend(spi.spi_device_schema(cs_pin_required=True))

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    "nhd_char_lcd_spi", require_miso=True, require_mosi=True
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await nhd_char_lcd.setup_lcd_display(var, config)
    await spi.register_spi_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA],
            [(NhdCharLcdSpi.operator("ref"), "it")],
            return_type=cg.void,
        )
        cg.add(var.set_writer(lambda_))
