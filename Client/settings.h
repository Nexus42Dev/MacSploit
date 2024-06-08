#include <iostream>
#include <unordered_map>
#include <string>

#include "exploit.h"

namespace settings {
    std::unordered_map<std::string, std::string> master_settings;

    bool get_boolean(std::string name) {
        std::string val = master_settings[name];
        return val == "false" ? false : true;
    }

    std::string get_string(std::string name) {
        return master_settings[name];
    }

    int get_number(std::string name) {
        std::string s = master_settings[name];
        return std::stoi(s);
    }

    void handle_setting(const std::string& key, const std::string& value) {
        if (key == "serverTeleports") {
            prevent_server_teleports = value == "false" ? false : true;
            return;
        }

        if (key == "placeRestrictions") {
            place_restrictions = value == "false" ? false : true;
            return;
        }

        settings::master_settings[key] = value;
    }
}