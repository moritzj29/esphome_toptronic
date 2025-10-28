#include "toptronic.h"
#include "esphome/core/log.h"

#include <sstream>
#include <string>
#include <iomanip>
namespace esphome {
namespace toptronic {

static const char *const TAG = "tt";

static const uint8_t RESPONSE = 0x42;
static const uint8_t GET_REQ = 0x40;
static const uint8_t SET_REQ = 0x46;

// https://stackoverflow.com/a/14051107/3140799
std::string hex_str(const uint8_t *data, int len)
{
     std::stringstream ss;
     ss << std::hex;

     for( int i(0) ; i < len; ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data[i];

     return ss.str();
}

uint32_t TopTronicBase::get_device_id() {
    return device_type_ | device_addr_;
}

uint32_t build_can_id(uint16_t sender_id, uint16_t receiver_mask) {
    return (0x7F << 22) | (sender_id << 11) | receiver_mask;
}

std::vector<uint8_t> build_get_request(uint8_t function_group, uint8_t function_number, uint32_t datapoint) {
    return {
        0x01,       // message length
        GET_REQ,    // GET_REQUEST = 0x40
        function_group,
        function_number,
        (uint8_t)(datapoint >> 8),
        (uint8_t)(datapoint),
    };
}

std::vector<uint8_t> build_set_request(uint8_t function_group, uint8_t function_number, uint32_t datapoint, std::vector<uint8_t> value) {
    std::vector<uint8_t> data = {
        0x01,       // message length
        SET_REQ,    // SET_REQUEST = 0x46
        function_group,
        function_number,
        (uint8_t)(datapoint >> 8),
        (uint8_t)(datapoint),
    };

    for (int i = 0; i < value.size(); i++) {
        data.push_back(value[i]);
    }
    
    return data;
}

uint32_t TopTronicBase::get_id() {
    return function_group_
        + (function_number_ << 8)
        + (datapoint_ << 16);
}

std::vector<uint8_t> TopTronicBase::get_request_data() {
    return build_get_request(function_group_, function_number_, datapoint_);
}

void TopTronicBase::add_on_set_callback(std::function<void(std::vector<uint8_t>)> &&callback) {
    this->set_callback_.add(std::move(callback));
}

void TopTronicBase::add_on_update_callback(std::function<void()> &&callback) {
    this->update_callback_.add(std::move(callback));
}

void TopTronicBase::update() {
    this->update_callback_.call();
}

template <typename T> 
T bytesToNumber(std::vector<uint8_t> value) {
    T a = 0;
    for(int i = 0; i < value.size(); i++) {
        a = (a << 8) + value[i];
    }
    return a;
}

float bytesToFloat(std::vector<uint8_t> value, TypeName type) {
    switch(type) {
        case U8:
            {
                return (float)bytesToNumber<uint8_t>(value);
            }
        case U16:
            {
                return (float)bytesToNumber<uint16_t>(value);
            }
        case U32:
            {
                return (float)bytesToNumber<uint32_t>(value);
            }
        case S8:
            {
                return (float)bytesToNumber<int8_t>(value);
            }
        case S16:
            {
                return (float)bytesToNumber<int16_t>(value);
            }
        case S32:
            {
                return (float)bytesToNumber<int32_t>(value);
            }
        case S64:
            {
                return (float)bytesToNumber<int64_t>(value);
            }            
    }
    return 0.0f;
}

template <typename T> 
std::vector<uint8_t> numberToBytes(T value) {
    std::vector<uint8_t> a;
    uint size = sizeof(value);
    for(int i = 0; i < size; i++) {
        a.push_back((uint8_t)(value >> 8*(size-i-1)));
    }
    return a;
}

std::vector<uint8_t> floatToBytes(float value, TypeName type) {
    switch(type) {
        case U8:
            {
                return numberToBytes((uint8_t)value);
            }
        case U16:
            {
                return numberToBytes((uint16_t)value);
            }
        case U32:
            {
                return numberToBytes((uint32_t)value);
            }
        case S8:
            {
                return numberToBytes((int8_t)value);
            }
        case S16:
            {
                return numberToBytes((int16_t)value);
            }
        case S32:
            {
                return numberToBytes((int32_t)value);
            }
        case S64:
            {
                return numberToBytes((int64_t)value);
            }            
    }
    return {};
}

float TopTronicSensor::parse_value(std::vector<uint8_t> value) {
    return bytesToFloat(value, type_);
}

bool TopTronicBinarySensor::parse_value(std::vector<uint8_t> value) {
    // Interpret any non-zero as true
    float v = bytesToFloat(value, type_);
    return v != 0.0f;
}

void TopTronicNumber::control(float value) {
    float v = multiplier_ * value;
    std::vector<uint8_t> bytes = floatToBytes(v, type_);

    std::vector<uint8_t> data = build_set_request(function_group_, function_number_, datapoint_, bytes);
    set_callback_.call(data);

    ESP_LOGI(TAG, "[SET] %s: %f, Data: 0x%s", get_name().c_str(), v, hex_str(&data[0], data.size()).c_str());
}

void TopTronicSwitch::control(bool state) {
    float v = state ? 1.0f : 0.0f;
    std::vector<uint8_t> bytes = floatToBytes(v, type_);
    std::vector<uint8_t> data = build_set_request(function_group_, function_number_, datapoint_, bytes);
    set_callback_.call(data);
    ESP_LOGI(TAG, "[SET] %s: %s, Data: 0x%s", get_name().c_str(), state ? "ON" : "OFF", hex_str(data.data(), static_cast<int>(data.size())).c_str());
}

void TopTronicSwitch::write_state(bool state) {
    control(state);
    this->publish_state(state);
}

std::string TopTronicTextSensor::parse_value(std::vector<uint8_t> value) {
    uint8_t intValue = bytesToNumber<uint8_t>(value);
    return toText_[intValue];
}

void TopTronicSelect::control(const std::string &text) {
    uint8_t value = toValue_[text];
    
    std::vector<uint8_t> data = build_set_request(function_group_, function_number_, datapoint_, {value});
    set_callback_.call(data);

    ESP_LOGI(TAG, "[SET] %s: %s, Data: 0x%s", get_name().c_str(), text.c_str(), hex_str(&data[0], data.size()).c_str());
}

TopTronicDevice* TopTronic::get_or_create_device(uint32_t device_id) {
    if (devices_.count(device_id) <= 0) {
        devices_[device_id] = new TopTronicDevice();
    }
    return devices_[device_id];
}

void TopTronic::add_sensor(TopTronicBase *sensor) {
    TopTronicDevice *device = get_or_create_device(sensor->get_device_id());
    device->sensors[sensor->get_id()] = sensor;
}

void TopTronic::add_input(TopTronicBase *input) {
    TopTronicDevice *device = get_or_create_device(input->get_device_id());
    device->inputs[input->get_id()] = input;
}

void TopTronic::register_sensor_callbacks() {
    for (const auto &d : devices_) {
        auto device = d.second;
        for (const auto &s : device->sensors) {
            auto sensor = s.second;
            auto canbus = canbus_;
            uint32_t can_id = build_can_id(device_type_ | device_addr_, sensor->get_device_id());
            
            sensor->add_on_update_callback([canbus, sensor, can_id]() -> void {
                auto data = sensor->get_request_data();
                canbus->send_data(can_id, true, data);

                ESP_LOGD(TAG, "[GET] Data: 0x%s", hex_str(&data[0], data.size()).c_str());
            });
        }
    }
}

void TopTronic::register_input_callbacks() {
    for (const auto &d : devices_) {
        auto device = d.second;
        for (const auto &i : device->inputs) {
            auto input = i.second;
            auto canbus = canbus_;
            uint32_t can_id = build_can_id(device_type_ | device_addr_, input->get_device_id());
            
            input->add_on_set_callback([canbus, input, can_id](std::vector<uint8_t> data) -> void {               
                canbus->send_data(can_id, true, data);
            });
        }
    }
}

TopTronicBase* TopTronic::get_sensor(uint32_t device_id, uint32_t sensor_id) {
    if (devices_.count(device_id) == 0) {
        return NULL;
    }
    TopTronicDevice* device = devices_[device_id];

    if (device->sensors.count(sensor_id) == 0) {
        return NULL;
    }
    return device->sensors[sensor_id];
}

void TopTronic::link_inputs() {
    for (const auto &d : devices_) {
        auto device = d.second;
        for (const auto &i : device->inputs) {
            auto inputBase = i.second;
            auto sensorBase = get_sensor(inputBase->get_device_id(), inputBase->get_id());
            if (sensorBase == NULL) continue;
            if (sensorBase->type() == SENSOR && inputBase->type() == SENSOR) {
                auto sensor = (TopTronicSensor*) sensorBase;
                auto input = (TopTronicNumber*) inputBase;
                sensor->add_on_raw_state_callback([input](float state) -> void {
                    float divider = input->get_multiplier();
                    input->publish_state(state / divider);
                });
            } else if (sensorBase->type() == TEXTSENSOR && inputBase->type() == TEXTSENSOR) {
                auto sensor = (TopTronicTextSensor*) sensorBase;
                auto input = (TopTronicSelect*) inputBase;
                sensor->add_on_raw_state_callback([input](std::string state) -> void {
                    input->publish_state(state);
                });
            } else if (sensorBase->type() == BINARYSENSOR && inputBase->type() == SWITCHTYPE) {
                auto sensor = (TopTronicBinarySensor*) sensorBase;
                auto input = (TopTronicSwitch*) inputBase;
                sensor->add_on_state_callback([input](bool state) -> void {
                    input->publish_state(state);
                });
            }
        }
    }
}

void TopTronic::setup() {
    link_inputs();
    register_sensor_callbacks();
    register_input_callbacks();
}

void TopTronic::loop() {

}

void TopTronic::dump_config() {

}

void log_response_frame(std::vector<uint8_t> data, uint32_t can_id, std::string sensor_name) {
    ESP_LOGI(TAG, "[RES] Can-ID: 0x%08X, Sensor: %s, Data: 0x%s", can_id, sensor_name.c_str(), hex_str(&data[0], data.size()).c_str());
}

void TopTronic::parse_frame(std::vector<uint8_t> data, uint32_t can_id, bool remote_transmission_request) {
    uint8_t msg_id = can_id >> 24;
    uint8_t device_type = (can_id >> 8) & 0xFF;
    uint8_t device_id = can_id & 0xFF;

    if (msg_id == 0x1f) {
        // Start of a message
        uint8_t msg_len = data[0] >> 3;
        if (msg_len == 0) {
            // Full message, remove size
            interpret_message(std::vector<uint8_t>(data.begin() + 1, data.end()), can_id, remote_transmission_request);
        } else {
            // We expect further messages
            u_int8_t msg_header = data[1];
            ESP_LOGD(TAG, "     - Start of message with id: %d with length %d", msg_header, msg_len);
            pending_messages_[msg_header] = std::make_pair(std::vector<uint8_t>(data.begin() + 2, data.end()), msg_len -1);
        }
    } else {
        u_int8_t msg_header = data[0];
        auto it = pending_messages_.find(msg_header);
        if (it != pending_messages_.end()) {
            auto pending_msg = it->second;
            auto msg_len = pending_msg.second - 1;
            ESP_LOGD(TAG, "     - Part of message with id: %d with remaining length %d", msg_header, msg_len);
            pending_msg.first.insert(pending_msg.first.end(), data.begin() + 1, data.end());
            if (msg_len == 0) {
                pending_messages_.erase(msg_header);
                // Delete the last two bytes (CRC?) 
                auto real_msg = std::vector<uint8_t>(pending_msg.first.begin(), pending_msg.first.end() - 2);
                interpret_message(real_msg, can_id, remote_transmission_request);
            } else {
                pending_messages_[msg_header] = std::make_pair(pending_msg.first, msg_len);
            }
        }
    }
}

void TopTronic::interpret_message(std::vector<uint8_t> data, uint32_t can_id, bool remote_transmission_request) {
    // check if operation is of type RESPONSE
    // GET and SET messages do not contain any information -> log only

    if (data[0] == GET_REQ) {
        ESP_LOGD(TAG, "[GET] Can-ID: 0x%08X, Data: 0x%s", can_id, hex_str(&data[0], data.size()).c_str());
        return;
    }

    if (data[0] == SET_REQ) {
        ESP_LOGI(TAG, "[SET] Can-ID: 0x%08X, Data: 0x%s", can_id, hex_str(&data[0], data.size()).c_str());
        return;
    }

    if (data[0] != RESPONSE) {
        ESP_LOGD(TAG, "[UNK] Can-ID: 0x%08X, Data: 0x%s", can_id, hex_str(&data[0], data.size()).c_str());
        return;
    }

    // extract device_id and check if device exists
    uint32_t device_id = (can_id >> 11) & 0x7FF;

    if (devices_.count(device_id) <= 0) {
        return;
    }
    TopTronicDevice *device = devices_[device_id];   

    // check if sensor exists for the received value
    uint32_t datapoint = data[4] + (data[3] << 8);
    uint32_t id = data[1] // function_group
        + (data[2] << 8)  // function_number
        + (datapoint << 16);

    if (device->sensors.count(id) <= 0) {
        // device known, but sensor not found -> possibly a new sensor
        ESP_LOGD(TAG, "[NEW] Can-ID: 0x%08X (%u), Data: 0x%s, Datapoint: 0x%04X (%u), ID: 0x%06X (%u), function_group: 0x%02X (%u), function_number: 0x%02X (%u)", 
            can_id, can_id, 
            hex_str(&data[0], data.size()).c_str(), 
            datapoint, datapoint, 
            id, id, 
            data[1], data[1], 
            data[2], data[2]
        );
        return;
    }
    ESP_LOGD(TAG, "[EXS] Can-ID: 0x%08X (%u), Data: 0x%s, Datapoint: 0x%04X (%u), ID: 0x%06X (%u), function_group: 0x%02X (%u), function_number: 0x%02X (%u)", 
            can_id, can_id, 
            hex_str(&data[0], data.size()).c_str(), 
            datapoint, datapoint, 
            id, id, 
            data[1], data[1], 
            data[2], data[2]
        );
    TopTronicBase *sensorBase = device->sensors[id];

    // check sensor type -> float or string
    if (sensorBase->type() == SENSOR) {
        TopTronicSensor *sensor = (TopTronicSensor*)sensorBase;
        float value = sensor->parse_value(std::vector<uint8_t>(data.begin() + 5, data.end()));
        sensor->publish_state(value);
        log_response_frame(data, can_id, sensor->get_name());
    } else if (sensorBase->type() == TEXTSENSOR) {
        TopTronicTextSensor *sensor = (TopTronicTextSensor*)sensorBase;
        std::string value = sensor->parse_value(std::vector<uint8_t>(data.begin() + 5, data.end()));
        sensor->publish_state(value);
        log_response_frame(data, can_id, sensor->get_name());
    } else if (sensorBase->type() == BINARYSENSOR) {
        TopTronicBinarySensor *sensor = (TopTronicBinarySensor*)sensorBase;
        bool value = sensor->parse_value(std::vector<uint8_t>(data.begin() + 5, data.end()));
        sensor->publish_state(value);
        log_response_frame(data, can_id, sensor->get_name());
    }
}

}  // namespace toptronic
}  // namespace esphome