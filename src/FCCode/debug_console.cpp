#include "debug_console.hpp"
#include <array>
#include <Arduino.h>
#include "ArduinoJson.h"
#include <ChRt.h>

std::map<debug_severity, const char *> debug_console::severity_strs{
    {debug_severity::DEBUG, "DEBUG"},   {debug_severity::INFO, "INFO"},
    {debug_severity::NOTICE, "NOTICE"}, {debug_severity::WARNING, "WARNING"},
    {debug_severity::ERROR, "ERROR"},   {debug_severity::CRITICAL, "CRITICAL"},
    {debug_severity::ALERT, "ALERT"},   {debug_severity::EMERGENCY, "EMERGENCY"},
};

// Static initialization of debugger.
systime_t debug_console::_start_time = static_cast<systime_t>(0);
bool debug_console::is_init = false;

unsigned int debug_console::_get_elapsed_time() {
    systime_t current_time = chVTGetSystemTimeX();
    if (!Serial) {
        /** Reset the start time if Serial is unconnected. We
         * do this so that the logging utility on the computer
         * can always produce a correct timestamp.
         */
        _start_time = current_time;
    }
    unsigned int elapsed_time = ST2MS(current_time - _start_time);
    return elapsed_time;
}

void debug_console::_print_json_msg(severity s, const char *msg) {
    StaticJsonDocument<50> doc;
    doc["t"] = _get_elapsed_time();
    doc["svrty"] = severity_strs.at(s);
    doc["msg"] = msg;
    serializeJson(doc, Serial);
    Serial.println();
}

bool debug_console::init() {
    is_init = true;
    Serial.begin(9600);
    pinMode(13, OUTPUT);

    Serial.println("Waiting for serial console.");
    while (!Serial)
        ;
    _start_time = chVTGetSystemTimeX();

    return is_init;
}

void debug_console::printf(severity s, const char *format, ...) {
    if (!is_init) return;
    char buf[100];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    _print_json_msg(s, buf);
    va_end(args);
}

void debug_console::println(severity s, const char *str) {
    if (!is_init) return;
    _print_json_msg(s, str);
}

void debug_console::blink_led() {
    if (!is_init) return;
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}

void debug_console::print_state_field(const SerializableStateFieldBase& field) {
    StaticJsonDocument<50> doc;
    doc["t"] = _get_elapsed_time();
    doc["field"] = field.name().c_str();
    doc["val"] = field.print();
    serializeJson(doc, Serial);
    Serial.println();
}

void debug_console::process_commands(const StateFieldRegistry& registry) {
    constexpr size_t SERIAL_BUF_SIZE = 64;
    char buf[SERIAL_BUF_SIZE] = {0};
    for(size_t i = 0; i < SERIAL_BUF_SIZE && Serial.available(); i++) {
        buf[i] = Serial.read();
    }

    // Get all chunks of the buffer that are complete JSON messages
    size_t json_msg_starts[5] = {0};
    size_t num_json_msgs_found = 0;
    bool inside_json_msg = false;
    for(size_t i = 0; i < SERIAL_BUF_SIZE; i++) {
        if (buf[i] == '{') {
            inside_json_msg = true;
            json_msg_starts[num_json_msgs_found] = i;
        }
        else if (inside_json_msg && buf[i] == '}') {
            inside_json_msg = false;
            num_json_msgs_found++;
            // Replace newline after the JSON object with a null character, so
            // that the JSON deserializer can recognize it as the end of a JSON
            // object
            if (i + 1 < SERIAL_BUF_SIZE) {
                buf[i + 1] = '\x00';
            }
        }
    }

    // Deserialize all found JSON messages and check their validity
    std::array<StaticJsonDocument<50>, 5> msgs;
    std::array<bool, 5> msg_ok;
    for(size_t i = 0; i < num_json_msgs_found; i++) {
        auto result = deserializeJson(msgs[i], &buf[json_msg_starts[i]]);
        if (result == DeserializationError::Ok) {
            msg_ok[i] = true;
        }
    }

    // For all valid messages, modify or print the relevant item in the state
    // field registry
    for(size_t i = 0; i < num_json_msgs_found; i++) {
        if (!msg_ok[i]) continue;
        JsonVariant msg_mode = msgs[i]["r/w"];
        JsonVariant field = msgs[i]["field"];
        JsonVariant val = msgs[i]["val"];

        // Check sanity of data
        if (msg_mode.isNull()) continue;
        if (field.isNull()) continue;

        if(!msg_mode.is<unsigned char>()) continue;
        const unsigned char mode = msg_mode.as<unsigned char>();
        if (mode == 'w' && val.isNull()) continue;

        // If data is ok, proceed with state field reading/writing
        switch(mode) {
            case 'r': {
                std::shared_ptr<ReadableStateFieldBase> field_ptr = registry.find_readable_field(field.as<const char*>());
                if (!field_ptr) break;
                else print_state_field(*field_ptr);
            }
            case 'w': {
                std::shared_ptr<WritableStateFieldBase> field_ptr = registry.find_writable_field(field.as<const char*>());
                if (!field_ptr) break;
                // TODO convert val into usable value for field and write to field
                print_state_field(*field_ptr);
                break;
            }
        }
    }
}