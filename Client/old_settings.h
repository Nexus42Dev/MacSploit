#include <iostream>
#include <unordered_map>
#include <string>

#include "exploit.h"

struct setting_value_t {
    std::string key;
    union {
        int n;
        bool b;
        std::string s;
    } data;
};

namespace settings {
    std::unordered_map<std::string, setting_value_t> master_settings;

    bool get_boolean(std::string name) {
        return master_settings[name].data.b;
    }

    std::string get_string(std::string name) {
        return master_settings[name].data.s;
    }

    int get_number(std::string name) {
        return master_settings[name].data.n;
    }

    bool is_boolean(const std::string& value) {
        return value == "false" || value == "true";
    }

    bool convert_boolean(const std::string& key, const std::string& value) {
        return value == "false" ? false : true;
    }

    setting_value_t parse_setting(const std::string& key, const std::string& value) {
        setting_value_t new_setting{key};
        if (is_boolean(value)) {
            new_setting.data.b = convert_boolean(key, value);
            return 
        }

        int num = std::stoi(value);
        if (std::to_string(num) != value) {
            new_setting.data.s = value;
            return new_setting;
        }

        new_setting.data.n = num;
        return new_setting;
    }

    void handle_setting(std::string key, std::string value) {
        setting_value_t setting = parse_setting(key, value);
        master_settings[key] = setting;
    }
}