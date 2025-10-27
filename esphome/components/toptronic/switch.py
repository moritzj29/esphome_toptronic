import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch as _switch
from esphome.const import CONF_TYPE
from . import (
    toptronic,
    CONF_TT_ID,
    TopTronicComponent,
    CONFIG_SCHEMA_BASE,
    CONF_DEVICE_TYPE,
    CONF_DEVICE_ADDR,
    CONF_FUNCTION_GROUP,
    CONF_FUNCTION_NUMBER,
    CONF_DATAPOINT,
    TT_TYPE_OPTIONS,
    get_device_type,
)

TopTronicSwitch = toptronic.class_("TopTronicSwitch", _switch.Switch)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_TT_ID): cv.use_id(TopTronicComponent),
    cv.Required(CONF_TYPE): cv.enum(TT_TYPE_OPTIONS),
}).extend(_switch.switch_schema(TopTronicSwitch)).extend(CONFIG_SCHEMA_BASE)

async def to_code(config):
    tt = await cg.get_variable(config[CONF_TT_ID])
    var = await _switch.new_switch(config)

    device_type = get_device_type(config[CONF_DEVICE_TYPE])
    cg.add(var.set_device_type(device_type))
    cg.add(var.set_device_addr(config[CONF_DEVICE_ADDR]))
    cg.add(var.set_function_group(config[CONF_FUNCTION_GROUP]))
    cg.add(var.set_function_number(config[CONF_FUNCTION_NUMBER]))
    cg.add(var.set_datapoint(config[CONF_DATAPOINT]))
    cg.add(var.set_type(config[CONF_TYPE]))

    cg.add(tt.add_input(var))