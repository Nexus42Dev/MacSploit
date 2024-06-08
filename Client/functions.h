//
// Created by Nexus Pancakes on 9/11/2022.
//

#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <sstream>

#include <ApplicationServices/ApplicationServices.h>

#include "bitop.h"
#include "exploit.h"
#include "http_request.h"
#include "websocket.h"
#include "crypt.h"

#define WHITE0BIT 0
#define WHITE1BIT 1
#define bitmask(b) (1 << (b))
#define bit2mask(b1, b2) (bitmask(b1) | bitmask(b2))

#define testbits(x, m) ((x) & (m))
#define testbit(x, b) testbits(x, bitmask(b))
#define WHITEBITS bit2mask(WHITE0BIT, WHITE1BIT)
#define otherwhite(g) (*(uint8_t*)(g + offsets::global::white_offset) ^ WHITEBITS)

namespace filelibrary {
    const std::string workspace_path = std::string(getenv("HOME")) + "/Documents/Macsploit Workspace/";
    const std::string autoexec_path = std::string(getenv("HOME")) + "/Documents/Macsploit Automatic Execution/";

    int readfile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("readfile requires string argument.", 0);
        }

        std::string filename = rbx_tolstring(rl, -1, nullptr);
        std::string filedata;
        std::string chunk;

        sanitiseFileInput(filename);
        std::ifstream file(workspace_path + filename);
        if (!file.good()) {
            (*rbx_error)("The requested file does not exist.");
            return 0;
        }

        while (getline(file, chunk)) {
            filedata.append(chunk + '\n');
        }

        filedata.pop_back();
        file.close();

        rbx_pushstring(rl, filedata);
        return 1;
    }

    int writefile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 2);
        std::string filename = rbx_tolstring(rl, 1, nullptr);
        sanitiseFileInput(filename);
        
        std::istringstream path(filename);
        std::vector<std::string> pathing;
        std::string folder;

        while (getline(path, folder, '/')) {
            pathing.push_back(folder);
        }

        std::string pathing_path = "";
        for (int i = 0; i < pathing.size() - 1; i++) {
            std::string folder = pathing[i];
            std::string current = pathing_path + folder;
            if (std::filesystem::exists(workspace_path + current)) continue;
            if (!std::filesystem::create_directory(workspace_path + current)) {
                (*rbx_error)("Failed to create leading directories.");
                return 0;
            }

            pathing_path += folder + "/";
        }

        size_t datasize;
        const char* data = rbx_tolstring(rl, 2, &datasize);

        std::ofstream file(workspace_path + filename);
        if (!file.good()) {
            file.close();
            (*rbx_error)("Failed to create file stream.");
            return 0;
        }

        file << std::string(data, datasize);
        file.close();
        return 0;
    }

    int appendfile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 2);
        std::string filename = rbx_tolstring(rl, 1, nullptr);
        sanitiseFileInput(filename);

        size_t datasize;
        const char* data = rbx_tolstring(rl, 2, &datasize);

        std::ofstream file(std::string(workspace_path + filename), std::ios_base::app);
        if (!file.good()) {
            (*rbx_error)("The requested file does not exist.", 0);
            return 0;
        }

        file << std::string(data, datasize);

        file.close();
        return 0;
    }

    int loadfile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        std::string filename = rbx_tolstring(rl, 1, nullptr);
        sanitiseFileInput(filename);
        
        std::ifstream strm(std::string(workspace_path + filename));
        if (!strm.good()) {
            (*rbx_error)("The file you are trying to load quite simply does not exist lol.");
            return 0;
        }

        std::string filescript;
        std::string chunk;

        while (getline(strm, chunk)) {
            filescript.append(chunk + '\n');
        }

        filescript.pop_back();
        std::string filebytecode = Luau::compile(filescript);
        if ((*rbx_deserialize)(rl, script_author.c_str(), filebytecode.c_str(), static_cast<int>(filebytecode.size()), 0) != 0) {
            rbx_pushnil(rl);
            (*rbx_insert)(rl, -2);
            return 2; // (*rbx_error)(rbx_tolstring(rl, -1, nullptr));
        }

        return 1;
    }

    int makefolder(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid argument for makefolder", 0);
        }

        std::string filename = rbx_tolstring(rl, -1, nullptr);
        sanitiseFileInput(filename);

        std::istringstream path(filename);
        std::vector<std::string> pathing;
        std::string folder;

        while (getline(path, folder, '/')) {
            pathing.push_back(folder);
        }

        std::string pathing_path = "";
        for (int i = 0; i < pathing.size() - 1; i++) {
            std::string folder = pathing[i];
            std::string current = pathing_path + folder;
            if (std::filesystem::exists(workspace_path + current)) continue;
            if (!std::filesystem::create_directory(workspace_path + current)) {
                (*rbx_error)("Failed to create leading directories.");
                return 0;
            }

            pathing_path += folder + "/";
        }

        std::filesystem::create_directory(workspace_path + filename);
        return 0;
    }

    int delfolder(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid argument for delfolder", 0);
        }

        std::string path = rbx_tolstring(rl, -1, nullptr);
        sanitiseFileInput(path);

        std::filesystem::remove_all(workspace_path + path);
        return 0;
    }

    int delfile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid argument for delfile", 0);
        }

        std::string path = rbx_tolstring(rl, -1, nullptr);
        sanitiseFileInput(path);

        std::filesystem::remove(workspace_path + path);
        return 0;
    }

    int isfile(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid argument for isfile", 0);
        }

        std::string path = rbx_tolstring(rl, -1, nullptr);
        sanitiseFileInput(path);

        if (std::filesystem::exists(workspace_path + path)) {
            rbx_pushboolean(rl, !std::filesystem::is_directory(workspace_path + path));
        } else { rbx_pushboolean(rl, false); }

        return 1;
    }

    int isfolder(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid argument for isfolder", 0);
        }

        std::string path = rbx_tolstring(rl, -1, nullptr);
        sanitiseFileInput(path);

        rbx_pushboolean(rl, std::filesystem::is_directory(workspace_path + path));
        return 1;
    }

    int listfiles(uint64_t rl) {
        if (!settings::get_boolean("fileSystem")) {
            (*rbx_error)("Filesystem Disabled.");
            return 0;
        }

        std::string path = workspace_path;
        std::string directory;

        if (rbx_gettop(rl) == 1) {
            if (rbx_gettype(rl, -1) != LUA_TSTRING) {
                (*rbx_error)("Invalid argument for listfiles", 0);
            }

            directory = rbx_tolstring(rl, -1, nullptr);
            sanitiseFileInput(directory);

            if (!std::filesystem::is_directory(workspace_path + directory)) {
                (*rbx_error)("The directory does not exist", 0);
            }

            path = workspace_path + directory;
        }   

        std::vector<std::string> file_names;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::string name = entry.path();

            std::string filename2 = "";
            std::istringstream path2(name);
            while (getline(path2, filename2, '/')) { }
            file_names.push_back(directory + "\\" + filename2);
        }

        int table_index = 0;
        (*rbx_createtable)(rl, file_names.size(), 0);
        for (auto name : file_names) {
            rbx_pushstring(rl, name);
            (*rbx_rawseti)(rl, -2, ++table_index);
        }

        return 1;
    }
}

int identifyexecutor(uint64_t rl) {
    rbx_pushstring(rl, "macsploit is the best fucking exploit ever made.");
    rbx_pushstring(rl, current_version);
    return 2;
}

int getclipboard(uint64_t rl) {
    (*rbx_checkany)(rl, 0);
    char data[150000];
    FILE* p = popen("/usr/bin/pbpaste", "r");
    fread(data, sizeof(data), 1, p);
    pclose(p);

    (*rbx_pushlstring)(rl, data, sizeof(data));
    return 1;
}

int setclipboard(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TSTRING) {
        (*rbx_error)("Invalid argument for setclipboard", 0);
    }

    const char* cdata = rbx_tolstring(rl, 1, nullptr);
    FILE* p = popen("/usr/bin/pbcopy", "w");
    fputs(cdata, p);
    pclose(p);
    return 0;
}

int queue_on_teleport(uint64_t rl) {
    std::cout << "Added Queue\n";
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, -1) != LUA_TSTRING) {
        (*rbx_error)("Invalid arguments for queue_on_teleport", 0);
    }

    size_t size;
    const char* cscript = rbx_tolstring(rl, -1, &size);
    std::string script(cscript, size);

    teleport_script_queue.push_back(script);
    return 0;
}

typedef std::unordered_map<std::string, std::string> string_container;
void http_request_handler(uint64_t rl, string_container headers, string_container cookies, const char* body, const char* method, const char* url) {
    http_response response = execute_request(url, method, body, headers, cookies);
    if (!response.succeeded) {
        rbx_pushstring(rl, "Failed to make request to server.");
        rbx_continue(rl, 1); return;
    }

    (*rbx_createtable)(rl, 0, 6);
    rbx_pushboolean(rl, response.response_success);
    (*rbx_setfield)(rl, -2, "Success");

    (*rbx_pushnumber)(rl, response.status_code);
    (*rbx_setfield)(rl, -2, "StatusCode");

    int status_code = response.status_code;
    std::string message = status_messages.count(status_code) ? status_messages[status_code] : std::to_string(status_code);

    rbx_pushstring(rl, message);
    (*rbx_setfield)(rl, -2, "StatusMessage");

    rbx_pushstring(rl, response.response_body);
    (*rbx_setfield)(rl, -2, "Body");

    (*rbx_createtable)(rl, 0, response.headers.size());
    for (auto pair : response.headers) {
        rbx_pushstring(rl, pair.second);
        (*rbx_setfield)(rl, -2, pair.first.c_str());
    }

    (*rbx_setfield)(rl, -2, "Headers");
    (*rbx_createtable)(rl, 0, response.cookies.size());
    for (auto pair : response.cookies) {
        rbx_pushstring(rl, pair.second);
        (*rbx_setfield)(rl, -2, pair.first.c_str());
    }

    (*rbx_setfield)(rl, -2, "Cookies");
    rbx_continue(rl, 1);
}

int http_request(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (!settings::get_boolean("httpTraffic")) {
        (*rbx_error)("HTTP Disabled.");
        return 0;
    }
    
    if (rbx_gettype(rl, -1) != LUA_TTABLE) {
        (*rbx_error)("Invalid arguments for http_request", 0);
    }

    const char* body = "";
    const char* method = "GET";
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> cookies;

    rbx_assert(rl, (*rbx_getfield)(rl, -1, "Url"), "Url", LUA_TSTRING);
    const char* url = rbx_tolstring(rl, -1, nullptr);
    rbx_pop(rl, 1);

    std::cout << "[Abyss] Generating Request for " << url << "\n";

    (*rbx_getfield)(rl, -1, "Method");
    if (!rbx_isnil(rl, -1)) {
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid value for Method", 0);
        }

        method = rbx_tolstring(rl, -1, nullptr);
    }

    rbx_pop(rl, 1);
    (*rbx_getfield)(rl, -1, "Headers");
    if (!rbx_isnil(rl, -1)) {
        if (rbx_gettype(rl, -1) != LUA_TTABLE) {
            (*rbx_error)("Invalid value for Headers", 0);
        }

        rbx_pushnil(rl);
        while ((*rbx_next)(rl, -2)) {
            (*rbx_pushvalue)(rl, -2);
            const char* key = rbx_tolstring(rl, -1, nullptr);
            const char* value = rbx_tolstring(rl, -2, nullptr);
            headers[key] = value;
            rbx_pop(rl, 2);
        }
    }

    rbx_pop(rl, 1);
    if (settings::get_boolean("compatibilityMode")) {
        (*rbx_getfield)(rl, -1, "Cookies");
        if (!rbx_isnil(rl, -1)) {
            if (rbx_gettype(rl, -1) != LUA_TTABLE) {
                (*rbx_error)("Invalid value for Cookies", 0);
            }

            rbx_pushnil(rl);
            while ((*rbx_next)(rl, -2)) {
                (*rbx_pushvalue)(rl, -2);
                const char* key = rbx_tolstring(rl, -1, nullptr);
                const char* value = rbx_tolstring(rl, -2, nullptr);
                cookies[key] = value;
                rbx_pop(rl, 2);
            }
        }

        rbx_pop(rl, 1);
    }

    (*rbx_getfield)(rl, -1, "Body");
    if (!rbx_isnil(rl, -1)) {
        if (rbx_gettype(rl, -1) != LUA_TSTRING) {
            (*rbx_error)("Invalid value for Body", 0);
        }

        body = rbx_tolstring(rl, -1, nullptr);
        std::cout << "Request Body:\n" << body << "\n";
    }

    rbx_pop(rl, 1);
    std::thread(http_request_handler, rl, headers, cookies, body, method, url).detach();
    return rbx_yield(rl, 0);
}

int iscclosure(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, -1) != LUA_TFUNCTION) {
        rbx_pushboolean(rl, false);
        return 1;
    }

    roblox_structs::Closure* cl = **(roblox_structs::Closure***)(rl + offsets::state_base);
    rbx_pushboolean(rl, cl->isC);
    return 1;
}

int islclosure(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        rbx_pushboolean(rl, false);
        return 1;
    }

    roblox_structs::Closure* cl = **(roblox_structs::Closure***)(rl + offsets::state_base);
    rbx_pushboolean(rl, !cl->isC);
    return 1;
}

int hookfunction_old(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    if (rbx_gettype(rl, -1) != LUA_TFUNCTION || rbx_gettype(rl, -2) != LUA_TFUNCTION) {
        (*rbx_error)("Hookfunction expects two arguments of type function", 0);
    }

    uint64_t base = *(uint64_t*)(rl + offsets::state_base);
    auto old_function = *(roblox_structs::Closure**)base;
    auto new_function = *(roblox_structs::Closure**)(base + 0x10);

    if (new_function->nupvalues > old_function->nupvalues) {
        (*rbx_error)("Too many upvalues for hookfunction", 0);
    }

    if (old_function->isC) {
        if (!new_function->isC) {
            (*rbx_error)("C function expected", 0);
        }

        uint64_t old_address = old_function->c.f + (uint64_t)&old_function->c.f;
        uint64_t new_address = new_function->c.f + (uint64_t)&new_function->c.f;
        old_function->c.f = new_address - (uint64_t)&old_function->c.f;
        new_function->c.f = old_address - (uint64_t)&new_function->c.f;

        uint64_t old_cont = old_function->c.cont - (uint64_t)&old_function->c.cont;
        uint64_t new_cont = new_function->c.cont - (uint64_t)&new_function->c.cont;
        old_function->c.cont = new_cont + (uint64_t)&old_function->c.cont;
        new_function->c.cont = old_cont + (uint64_t)&new_function->c.cont;


        uint8_t old_nupvalues = old_function->nupvalues;
        uint8_t old_stacksize = old_function->stacksize;
        uint8_t old_preload = old_function->preload;

        old_function->nupvalues = new_function->nupvalues;
        old_function->stacksize = new_function->stacksize;
        old_function->preload = new_function->preload;

        new_function->nupvalues = old_nupvalues;
        new_function->stacksize = old_stacksize;
        new_function->preload = old_preload;

        roblox_structs::TValue old_upvalues[old_nupvalues];
        for (int i = 0; i < old_nupvalues; i++) {
            old_upvalues[i] = old_function->c.upvals[i];
            memcpy((void*)&old_function->c.upvals[i], (void*)&new_function->c.upvals[i], 0x10);
        }

         memcpy((void*)new_function->c.upvals, (void*)old_upvalues, old_nupvalues * 0x10);
    } else {
        if (new_function->isC) {
            (*rbx_error)("lua function expected", 0);
        }

        uint64_t old_proto = (uint64_t)&old_function->l.p + old_function->l.p;
        auto original_proto = deserialize_proto(old_proto);
        uint8_t old_nupvalues = old_function->nupvalues;
        uint8_t old_stacksize = old_function->stacksize;
        uint8_t old_preload = old_function->preload;

        uint64_t new_proto = (uint64_t)&new_function->l.p + new_function->l.p;
        auto proto = deserialize_proto(new_proto);
        serialize_proto(proto, old_proto);

        old_function->nupvalues = new_function->nupvalues;
        old_function->stacksize = new_function->stacksize;
        old_function->preload = new_function->preload;

        serialize_proto(original_proto, new_proto);
        new_function->nupvalues = old_nupvalues;
        new_function->stacksize = old_stacksize;
        new_function->preload = old_preload;

        roblox_structs::TValue old_upvalues[old_nupvalues];
        for (int i = 0; i < new_function->nupvalues; i++) {
            old_upvalues[i] = old_function->l.uprefs[i];
            memcpy((void*)&old_function->l.uprefs[i], (void*)&new_function->l.uprefs[i], 0x10);
        }

        memcpy((void*)new_function->l.uprefs, (void*)old_upvalues, old_nupvalues * 0x10);
    }

    **(uint64_t**)(rl + offsets::state_top) = (uint64_t)(new_function);
    *(int*)(*(uint64_t*)(rl + offsets::state_top) + 0xC) = LUA_TFUNCTION;
    *(uint64_t*)(rl + offsets::state_top) += 0x10;
    return 1;
}

namespace garbage_collector {
    struct collect_garbage {
        uint64_t state;
        uint64_t global_state;
        bool collect_tables;
        int count = 1;
    };

    /*    
        int deadmask = otherwhite(gc->global_state);
        if (!((gcobject->marked ^ WHITEBITS) & deadmask)) {
            return 0;
        }
    */

    int64_t getgc_cb(collect_garbage* gc, uint64_t page, roblox_structs::GCObject* gcobject) {
        if (!gc->collect_tables && gcobject->tt == LUA_TTABLE) {
            return 0;
        }

        if (gcobject->tt > 0xa) return 0;
        auto s = (roblox_structs::TValue*)rbx_getstacktop(gc->state);
        s->value.gcobject = (uint64_t)gcobject;
        s->tt = gcobject->tt;

        rbx_incrementtop(gc->state);
        (*rbx_rawseti)(gc->state, -2, gc->count);
        gc->count++;
        return 0;
    }

    int getgc(uint64_t rl) {
        bool tables = false;
        if (rbx_gettop(rl) == 1) {
            auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            tables = (bool)tval->value.b;
        }

        uint64_t global = filter_ptr_encryption(rl + offsets::global::global_offset, GLOBAL_ENC);
        collect_garbage gc{ rl, global, tables };

        (*rbx_createtable)(rl, 0, 0);
        (*rbx_visitgco)(rl, &gc, (void*)getgc_cb);
        return 1;
    }
}

int getidentity(uint64_t rl) {
    (*rbx_pushnumber)(rl, static_cast<double>(*rbx_getidentity(roblox_thread)));
    return 1;
}

int getidentity2(uint64_t rl) {
    (*rbx_pushnumber)(rl, static_cast<double>(*rbx_getidentity(rl)));
    return 1;
}

int setidentity(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
        (*rbx_error)("Invalid argument for setidentity", 0);
        return 0;
    }

    auto value = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    __int128 identity = static_cast<__int128>(value->value.n);
    *rbx_getidentity(rl) = identity;
    return 0;
}

int setglobalidentity(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
        (*rbx_error)("Invalid argument for setidentity", 0);
        return 0;
    }

    auto value = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    __int128 identity = static_cast<__int128>(value->value.n);
    *rbx_getidentity(roblox_thread) = identity;
    return 0;
}

int setidentityreq(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
        (*rbx_error)("Invalid argument for setidentity", 0);
    }

    auto value = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    __int128 identity = static_cast<__int128>(value->value.n);
    *rbx_getidentity(rl) = identity;
    return 0;
}

int setnamecallmethod(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, -1) != LUA_TSTRING) {
        (*rbx_error)("Invalid arguments for setnamecallmethod", 0);
    }

    uint64_t ts = **(uint64_t**)(rl + offsets::state_base);
    if (*(uint64_t*)(rl + 0x70) == 0) {
        (*rbx_error)("Unable to access namecall", 0);
    }

    *(uint64_t*)(rl + 0x70) = ts;
    return 0;
}

int getnamecall(uint64_t rl) {
    uint64_t ts_namecall = *(uint64_t*)(rl + 0x70);
    if (ts_namecall != 0) {
        uint64_t* top = (uint64_t*)(rl + offsets::state_top);
        *(uint64_t*)(*top) = ts_namecall;
        *(int*)(*top + 0xC) = LUA_TSTRING;
        *top += 0x10;
        return 1;
    }

    (*rbx_error)("Namecall is not currently available.", 0);
    return 0;
}

int setup_autoexec(uint64_t rl) {
    if (!std::filesystem::is_directory(filelibrary::autoexec_path)) {
        std::cout << "[Abyss] Auto execute directory does not exist, creating...\n";
        std::filesystem::create_directory(filelibrary::autoexec_path);
        return 0;
    }

    int exec_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(filelibrary::autoexec_path)) {
        if (entry.is_directory()) continue;
        std::string chunk;

        std::string filedata;
        std::ifstream file(entry.path());
        while (getline(file, chunk)) {
            filedata.append(chunk + '\n');
        }

        filedata.pop_back();
        file.close();

        execute_script(rl, filedata);
        exec_count++;
    }

    return exec_count;
}

int is_httpget(uint64_t rl) {
    uint64_t ts_namecall = *(uint64_t*)(rl + 0x70);

    if (!ts_namecall)
        (*rbx_error)("Namecall is not currently available.", 0);

    uint64_t* top = (uint64_t*)(rl + offsets::state_top);
    *(uint64_t*)(*top) = ts_namecall;
    *(int*)(*top + 0xC) = LUA_TSTRING;
    *top += 0x10;

    const char* method = rbx_tolstring(rl, -1, nullptr);
    rbx_pop(rl, 1);

    rbx_pushboolean(rl, !strcmp(method, "HttpGet") || !strcmp(method, "HttpGetAsync"));
    return 1;
}

int checkcaller(uint64_t rl) {
    rbx_pushboolean(rl, *(uint64_t*)(rl + offsets::state_env) == exploit_env);
    return 1;
}

namespace base64 {
    int base64encode(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, 1) != LUA_TSTRING) {
            (*rbx_error)("Incorrect argument #1 to base64encode, expected string.");
            return 0;
        }

        size_t len;
        const char* data = rbx_tolstring(rl, 1, &len);
        std::string encoded = crypto::base64_encode(std::string(data, len));
        rbx_pushstring(rl, encoded);
        return 1;
    }

    int base64decode(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, 1) != LUA_TSTRING) {
            (*rbx_error)("Incorrect argument #1 to base64decode, expected string.");
            return 0;
        }

        size_t len;
        const char* data = rbx_tolstring(rl, 1, &len);
        std::string decoded = crypto::base64_decode(std::string(data, len));
        rbx_pushstring(rl, decoded);
        return 1;
    }
}

namespace debug {
    uint64_t get_closure(uint64_t rl, int idx) {
        roblox_structs::lua_Debug debug_info;
        if (rbx_gettype(rl, idx) == LUA_TNUMBER) {
            auto val = (roblox_structs::TValue*)rbx_index2addr(rl, idx);
            double lvl = val->value.n;

            (*rbx_getinfo)(rl, static_cast<uint>(lvl), "f", &debug_info);
            auto top = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            uint64_t cl_addr = top->value.gcobject;

            rbx_pop(rl, 1);
            return cl_addr;
        } else if (rbx_gettype(rl, idx) == LUA_TFUNCTION) {
            auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, idx);
            return tval->value.gcobject;
        } else { return 0; }
    }

    int getstack(uint64_t rl) {
        if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
            (*rbx_error)("Invalid argument for getstack.");
            return 0;
        }

        int level = rbx_tointeger(rl, 1);
        uint64_t callinfo = (rl + offsets::state_callinfo);
        if (level >= (*(uint64_t*)callinfo - *(uint64_t*)(rl + offsets::state_callbase)) / 0x28 || level < 0) {
            (*rbx_error)("Invalid level for getstack.");
            return 0;
        }

        uint64_t ci = *(uint64_t*)(rl + offsets::state_callinfo) - level * 0x28;
        auto top = *(roblox_structs::TValue**)(ci + offsets::callinfo::top_offset);
        auto base = *(roblox_structs::TValue**)(ci + offsets::callinfo::base_offset);
        int stack_size = top - base;

        if (rbx_gettop(rl) == 1) {
            (*rbx_createtable)(rl, stack_size, 0);
            for (int i = 0; i < stack_size; i++) {
                rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)(base + i));
                rbx_incrementtop(rl);
                (*rbx_rawseti)(rl, -2, i + 1);
            }

            return 1;
        }
        
        if (rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid option for argument #2 getstack");
            return 0;
        }

        int index = rbx_tointeger(rl, 2);
        if (index > stack_size || index < 0) {
            (*rbx_error)("Invalid index found for getstack");
            return 0;
        }

        rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)(base + index - 1));
        rbx_incrementtop(rl);
        return 1;
    }

    int setstack(uint64_t rl) {
        (*rbx_checkany)(rl, 3);
        if (rbx_gettype(rl, 1) != LUA_TNUMBER || rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid argument for setstack.");
            return 0;
        }

        int level = rbx_tointeger(rl, 1);
        uint64_t callinfo = (rl + offsets::state_callinfo);
        if (level >= (*(uint64_t*)callinfo - *(uint64_t*)(rl + offsets::state_callbase)) / 0x28 || level < 0) {
            (*rbx_error)("Invalid level for getstack.");
            return 0;
        }

        uint64_t ci = *(uint64_t*)(rl + offsets::state_callinfo) - level * 0x28;
        auto top = *(roblox_structs::TValue**)(ci + offsets::callinfo::top_offset);
        auto base = *(roblox_structs::TValue**)(ci + offsets::callinfo::base_offset);
        int stack_size = top - base;

        int index = rbx_tointeger(rl, 2);
        if (index > stack_size || index < 0) {
            (*rbx_error)("Invalid index found for getstack");
            return 0;
        }

        rbx_setsvalue((uint64_t)(base + index - 1), rbx_index2addr(rl, 3));
        return 0;
    }

    int getproto(uint64_t rl) {
        if (rbx_gettop(rl) < 2 || rbx_gettype(rl, 1) != LUA_TFUNCTION || rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid arguments for getproto function.");
            return 0;
        }

        bool tbl = false;
        if (rbx_gettop(rl) == 3) {
            if (rbx_gettype(rl, 3) != LUA_TBOOLEAN) {
                (*rbx_error)("Invalid argument for active protos.");
                return 0;
            }

            tbl = rbx_toboolean(rl, 3);
        }

        uint64_t cl_addr = get_closure(rl, 1);
        auto idx_value = (roblox_structs::TValue*)rbx_index2addr(rl, 2);
        
        if (!cl_addr) {
            (*rbx_error)("Expected function/level, got something else.");
            return 0;
        }

        auto closure = (roblox_structs::Closure*)cl_addr;
        int idx = static_cast<int>(idx_value->value.n);

        uint64_t proto = filter_ptr_encryption((uint64_t)&closure->l.p, PROTO_ENC);
        int sizep = *(int*)(proto + offsets::proto::sizep);
        if (idx > sizep) {
            (*rbx_error)("Invalid index for getproto");
        }

        uint64_t* protos = reinterpret_cast<uint64_t*>(filter_ptr_encryption((proto + offsets::proto::protos), PROTOS_ENC));
        uint64_t subproto = protos[idx - 1];

        uint8_t nupvalues = *(uint8_t*)(subproto + offsets::proto::nupvalues);
        auto subcl = (*rbx_newlclosure)(rl, nupvalues, closure->env, subproto);

        if (tbl) {
            (*rbx_createtable)(rl, 1, 0);
            auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
            top->value.gcobject = (uint64_t)subcl;
            top->tt = LUA_TFUNCTION;
            rbx_incrementtop(rl);

            (*rbx_rawseti)(rl, -2, 1);
            return 1;
        }

        auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
        top->value.gcobject = (uint64_t)subcl;
        top->tt = LUA_TFUNCTION;
        rbx_incrementtop(rl);
        return 1;
    }

    int getprotos(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        uint64_t cl_addr = get_closure(rl, 1);

        if (!cl_addr) {
            (*rbx_error)("Invalid argument for getprotos function.");
            return 0;
        }

        auto cl = (roblox_structs::Closure*)cl_addr;
        uint64_t proto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
        int sizep = *(int*)(proto + offsets::proto::sizep);
        (*rbx_createtable)(rl, 0, sizep);

        if (sizep) {
            uint64_t* protos = reinterpret_cast<uint64_t*>(filter_ptr_encryption((proto + offsets::proto::protos), PROTOS_ENC));
            for (int i = 0; i < sizep; i++) {
                uint64_t subproto = protos[i];
                auto subcl = (*rbx_newlclosure)(rl, cl->nupvalues, cl->env, subproto);
                for (int i = 0; i < subcl->nupvalues; i++) {
                    rbx_setsvalue((uint64_t)&subcl->l.uprefs[i], (uint64_t)&cl->l.uprefs[i]);
                }

                auto sval = (roblox_structs::TValue*)rbx_getstacktop(rl);
                sval->value.gcobject = (uint64_t)subcl;
                sval->tt = LUA_TFUNCTION;
                rbx_incrementtop(rl);

                (*rbx_rawseti)(rl, -2, i + 1);
            }
        }

        return 1;
    }

    int getconstants(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        uint64_t cl_addr = get_closure(rl, 1);

        if (!cl_addr) {
            (*rbx_error)("Function/Level expected for argument #1 getconstants", 0);
            return 0;
        }

        auto cl = (roblox_structs::Closure*)cl_addr;
        if (cl->isC) {
            (*rbx_error)("Bad argument #1 for getconstant. Expected Lua function.");
            return 0;
        }

        uint64_t proto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
        uint64_t constant = (proto + offsets::proto::constants);
        uint64_t constants = filter_ptr_encryption(constant, CONSTANT_ENC);
        int size_constants = *(int*)(proto + offsets::proto::sizek);
        uint64_t table = (*rbx_createtable)(rl, 0, size_constants);

        for (int i = 0; i < size_constants; i++) {
            auto current_constant = *((roblox_structs::TValue*)constants + i);
            rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)&current_constant);
            rbx_incrementtop(rl);
            (*rbx_rawseti)(rl, -2, i + 1);
        }

        return 1;
    }

    int getconstant(uint64_t rl) {
        uint64_t cl_addr;
        (*rbx_checkany)(rl, 2);
        roblox_structs::lua_Debug debug_info;
        if (rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid argument #2 for getconstant");
            return 0;
        }
        
        if (rbx_gettype(rl, 1) == LUA_TNUMBER) {
            auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
            double lvl = val->value.n;

            (*rbx_getinfo)(rl, static_cast<uint>(lvl), "f", &debug_info);
            auto top = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            cl_addr = top->value.gcobject;
            rbx_pop(rl, 1);
        } else if (rbx_gettype(rl, 1) == LUA_TFUNCTION) {
            auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
            cl_addr = tval->value.gcobject;
        } else {
            (*rbx_error)("Function/Level expected for argument #1 getconstant", 0);
            return 0;
        }

        auto cl = (roblox_structs::Closure*)cl_addr;
        if (cl->isC) {
            (*rbx_error)("Bad argument #1 for getconstant. Expected Lua function.");
            return 0;
        }

        uint64_t proto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
        uint64_t constant = (proto + offsets::proto::constants);
        uint64_t constants = filter_ptr_encryption(constant, CONSTANT_ENC);
        int size_constants = *(int*)(proto + offsets::proto::sizek);

        int idx = rbx_tointeger(rl, 2, 0) - 1;
        if (idx < 0 || idx > size_constants) {
            (*rbx_error)("Invalid index for getconstant");
            return 0;
        }

        auto current_constant = *((roblox_structs::TValue*)constants + idx);
        rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)&current_constant);
        rbx_incrementtop(rl);
        return 1;
    }

    int setconstant(uint64_t rl) {
        uint64_t cl_addr;
        (*rbx_checkany)(rl, 3);
        roblox_structs::lua_Debug debug_info;
        if (rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid argument #2 for setconstant");
            return 0;
        }

        if (rbx_gettype(rl, 1) == LUA_TNUMBER) {
            auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
            double lvl = val->value.n;

            (*rbx_getinfo)(rl, static_cast<uint>(lvl), "f", &debug_info);
            auto top = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            cl_addr = top->value.gcobject;
            rbx_pop(rl, 1);
        } else if (rbx_gettype(rl, 1) == LUA_TFUNCTION) {
            auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
            cl_addr = tval->value.gcobject;
        } else {
            (*rbx_error)("Function/Level expected for argument #1 setconstant", 0);
            return 0;
        }

        auto cl = (roblox_structs::Closure*)cl_addr;
        if (cl->isC) {
            (*rbx_error)("Bad argument #1 for getconstant. Expected Lua function.");
            return 0;
        }

        uint64_t proto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
        uint64_t constant = (proto + offsets::proto::constants);
        uint64_t constants = filter_ptr_encryption(constant, CONSTANT_ENC);
        int size_constants = *(int*)(proto + offsets::proto::sizek);

        int idx = rbx_tointeger(rl, 2, 0) - 1;
        if (idx < 0 || idx > size_constants) {
            (*rbx_error)("Invalid index for setconstant");
            return 0;
        }

        rbx_setsvalue(constants + idx * 0x10, rbx_index2addr(rl, 3));
        return 0;
    }

    int getname(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
            (*rbx_error)("invalid arguments", 0);
        }

        auto cval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
        auto cl = (roblox_structs::Closure*)cval->value.gcobject;
        uint64_t proto = cl->l.p - (uint64_t)&cl->l.p;
        uint64_t debugname = (proto + 0x48) - *(uint64_t*)(proto + 0x48);
        const char* name = "";
        if (debugname) {
            name = (const char*)(debugname + 0x18);
        }

        rbx_pushstring(rl, name);
        return 1;
    }

    int getinfo(uint64_t rl) {
        int args = rbx_gettop(rl);
        std::string what = "flnsu";
        if (args == 2) {
            if (rbx_gettype(rl, 2) != LUA_TSTRING) {
                (*rbx_error)("String expected for argument #2 debug.getinfo", 0);
                return 0;
            }

            what = "f" + std::string(rbx_tolstring(rl, 2, nullptr));
        } else if (args != 1) {
            (*rbx_checkany)(rl, 2);
        }

        uint64_t cl_addr = 0x0;
        roblox_structs::lua_Debug debug_info;
        if (rbx_gettype(rl, 1) == LUA_TNUMBER) {
            auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
            double lvl = val->value.n;

            (*rbx_getinfo)(rl, static_cast<uint>(lvl), what.c_str(), &debug_info);
            auto top = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            cl_addr = top->value.gcobject;
            rbx_pop(rl, 1);
        } else if (rbx_gettype(rl, 1) == LUA_TFUNCTION) {
            (*rbx_getinfo)(rl, -(args), what.c_str(), &debug_info);
            auto top = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            cl_addr = top->value.gcobject;
            rbx_pop(rl, 1);
        } else {
            (*rbx_error)("Function/Level expected for argument #1 debug.getinfo", 0);
            return 0;
        }

        (*rbx_createtable)(rl, 10, 0);

        rbx_pushstring(rl, debug_info.name ? debug_info.name : "");
        (*rbx_setfield)(rl, -2, "name");

        rbx_pushstring(rl, debug_info.what ? debug_info.what : "");
        (*rbx_setfield)(rl, -2, "what");

        rbx_pushstring(rl, debug_info.source ? debug_info.source : "");
        (*rbx_setfield)(rl, -2, "source");

        rbx_pushstring(rl, debug_info.short_src ? debug_info.short_src : "");
        (*rbx_setfield)(rl, -2, "short_src");

        (*rbx_pushnumber)(rl, debug_info.linedefined);
        (*rbx_setfield)(rl, -2, "linedefined");

        (*rbx_pushnumber)(rl, debug_info.currentline);
        (*rbx_setfield)(rl, -2, "currentline");

        (*rbx_pushnumber)(rl, debug_info.nparams);
        (*rbx_setfield)(rl, -2, "numparams");

        (*rbx_pushnumber)(rl, debug_info.nupvals);
        (*rbx_setfield)(rl, -2, "nups");
        
        uint64_t top = rbx_getstacktop(rl);
        *(uint64_t*)top = cl_addr;
        *(int*)(top + 0xC) = LUA_TFUNCTION;
        rbx_incrementtop(rl);
        (*rbx_setfield)(rl, -2, "func");

        (*rbx_pushnumber)(rl, debug_info.isvararg);
        (*rbx_setfield)(rl, -2, "is_vararg");
        return 1;
    }

    int getregistry(uint64_t rl) {
        (*rbx_pushvalue)(rl, LUA_REGISTRYINDEX);
        return 1;
    }

    int getupvalues(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
            (*rbx_error)("Invalid argument for debug.getupvalues", 0);
        }

        auto cl = **(roblox_structs::Closure***)(rl + offsets::state_base);
        if (cl->isC) {
            (*rbx_createtable)(rl, 0, 0);
            return 1;
        }

        (*rbx_createtable)(rl, cl->nupvalues, 0);
        for (int i = 1; i <= cl->nupvalues; i++) {
            auto tval = &cl->l.uprefs[i - 1];
            if (tval->tt == LUA_TUPVAL) {
                auto uval = *(roblox_structs::TValue**)(tval->value.gcobject + 8);
                rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)uval);
                rbx_incrementtop(rl);
                (*rbx_rawseti)(rl, -2, i);
                continue;
            }
            
            rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)tval);
            rbx_incrementtop(rl);
            (*rbx_rawseti)(rl, -2, i);
        }

        return 1;
    }

    int getupvalue(uint64_t rl) {
        (*rbx_checkany)(rl, 2);
        if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
            (*rbx_error)("Invalid argument for debug.getupvalue", 0);
        }

        auto cl = **(roblox_structs::Closure***)(rl + offsets::state_base);
        if (cl->isC) {
            (*rbx_error)("Lua function expected", 0);
        }

        roblox_structs::TValue* val = (roblox_structs::TValue*)rbx_index2addr(rl, 2);
        int num = static_cast<int>(val->value.n) - 1;

        if (num > cl->nupvalues || num < 0) {
            (*rbx_error)("Invalid upvalue index", 0);
        }

        auto tval = &cl->l.uprefs[num];
        if (tval->tt == LUA_TUPVAL) {
            auto uval = *(roblox_structs::TValue**)(tval->value.gcobject + 8);
            rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)uval);
            rbx_incrementtop(rl);
            return 1;
        }

        rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)tval);
        rbx_incrementtop(rl);
        return 1;
    }

    int setupvalue(uint64_t rl) {
        (*rbx_checkany)(rl, 3);
        if (rbx_gettype(rl, 1) != LUA_TFUNCTION || rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid arguments for debug.setupvalue", 0);
        }

        auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
        auto cl = (roblox_structs::Closure*)tval->value.gcobject;
        if (cl->isC) {
            (*rbx_error)("Lua function expected", 0);
        }

        auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 2);
        int idx = static_cast<int>(val->value.n);

        if (idx > cl->nupvalues || idx < 0) {
            (*rbx_error)("Invalid upvalue index", 0);
        }

        rbx_setsvalue((uint64_t)&cl->l.uprefs[idx - 1], rbx_index2addr(rl, 3));
        return 0;
    }
}

int newcclosure_cont(uint64_t rl, int status) {
    if (!status) return rbx_gettop(rl); //Return Success
    (*rbx_error)(rbx_tolstring(rl, -1, nullptr)); //Return Error
    return 0; //Not Reached
}

int newcclosure_handler_pcall(uint64_t rl) {
    (*rbx_pushvalue)(rl, lua_upvalueindex(1)); //Push Upvalue To Stack For Execution
    (*rbx_insert)(rl, 1); //Move Upvalue to L->base; (Arguments start L->base + 1)
    int result = (*rbx_pcall)(rl); //Execute Pcall
    if (result == -1) return -1; //Propogate Yield

    auto status_tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1); //Get Success Value
    bool status = status_tval->value.b; //Read Success Value
    *(uint64_t*)(rl + offsets::state_base) += 0x10; //Pop Success Value
    if (!status) { //Propogate Error
        (*rbx_error)(rbx_tolstring(rl, -1, nullptr));
        return 0;
    }

    return result - 1; //Return Stack
}

int firetouchinterest(uint64_t rl) {
    (*rbx_checkany)(rl, 3);
    if (rbx_gettype(rl, 3) != LUA_TNUMBER) {
        (*rbx_error)("Incorrect argument #3 to 'firetouchinterest', expected number.");
        return 0;
    }

    (*rbx_getfield)(rl, LUA_GLOBALSINDEX, "workspace");
    uint64_t workspace = *(uint64_t*)rbx_touserdata(rl, -1);
    uint64_t world = *(uint64_t*)(workspace + offsets::roblox::world_offset);
    std::cout << "World: 0x" << world << "\n";
    rbx_pop(rl, 1);

    uint64_t part = *(uint64_t*)rbx_touserdata(rl, 1);
    uint64_t transmitter = *(uint64_t*)rbx_touserdata(rl, 2);
    int state = rbx_tointeger(rl, 3);

    uint64_t* t_primitive = (uint64_t*)(transmitter + offsets::roblox::part_primitive);
    uint64_t* f_primitive = (uint64_t*)(part + offsets::roblox::part_primitive);
    std::cout << "Primitive: 0x" << *t_primitive << "\n";
    (*rbx_touchpart)(world, *f_primitive, *t_primitive, state, 0);
    return 0;
}

int newcclosure_handler_safe(uint64_t rl) {
    uint64_t ci = *(uint64_t*)(rl + offsets::state_callinfo);
    auto tval = *(roblox_structs::TValue**)(ci + offsets::callinfo::func_offset);
    auto lclosure = safe_newcclosure_map[tval->value.gcobject];
    if (!lclosure) return 0;

    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)lclosure;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);

    (*rbx_insert)(rl, 1); //Move Upvalue to L->base; (Arguments start L->base + 1)
    int result = (*rbx_pcall)(rl); //Execute Pcall
    if (result == -1) return -1; //Propogate Yield
    
    auto status_tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1); //Get Success Value
    bool status = status_tval->value.b; //Read Success Value
    *(uint64_t*)(rl + offsets::state_base) += 0x10; //Pop Success Value
    if (!status) { //Propogate Error
        (*rbx_error)(rbx_tolstring(rl, -1, nullptr));
        return 0;
    }

    return result - 1; //Return Stack
}

int newcclosure(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        (*rbx_error)("Invalid arguments for newcclosure.", 0);
        return 0;
    }

    (*rbx_pushvalue)(rl, 1);
    (*rbx_pushcclosure)(rl, newcclosure_handler_pcall, "Abyss", 1, (uint64_t)newcclosure_cont);
    return 1;
}

int safe_newcclosure(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        std::cout << "Closure: 0x" << rbx_gettype(rl, 1) << "\n";
        (*rbx_error)("Invalid arguments for newcclosure.", 0);
        return 0;
    }

    auto tval1 = (roblox_structs::TValue*)rbx_index2addr(rl, 1); rbx_ref(rl, 1);
    (*rbx_pushcclosure)(rl, newcclosure_handler_safe, "Abyss", 0, (uint64_t)newcclosure_cont);

    auto tval2 = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
    auto cl = (roblox_structs::Closure*)tval2->value.gcobject;
    safe_newcclosure_map[tval2->value.gcobject] = (roblox_structs::Closure*)tval1->value.gcobject;
    return 1;
}

int test(uint64_t rl) {
    uint64_t test = *(uint64_t*)(*(uint64_t*)(*(uint64_t*)(rl + 0x78) + 0x18) + 8);
    if (test != 0) {
        uint64_t thing = 0;
        while (test != 0) {
            thing = test;
            std::cout << "0x" << test << "\n";
            test = *(uint64_t*)(test + 0x58);
        }

        std::cout << "0x" << thing << "\n";
        std::cout << "0x" << *(uint64_t*)(thing + 0x18) << "\n";
    }

    return 0;
}

int vindex = 0;
int setindex(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    vindex = rbx_tointeger(rl, 1, 0);
    std::cout << "Applying Index: 0x" << vindex << "\n";

    //int res = (*rbx_rawgeti)(rl, LUA_REGISTRYINDEX, vindex);
    std::cout << "Done.\n";
    //std::cout << "Value: 0x" << res << "\n";
    return 1;
}

int lol(uint64_t scan, int range, int depth) {
    uint64_t* data = (uint64_t*)scan;
    if (depth < 6) {
        for (int i = 0; i < range; i++) {
            uint64_t test = data[i];
            if (*(uint8_t*)((uint64_t)&test + 5) != 0x60 || *(uint8_t*)((uint64_t)&test + 6) != 0x0) {
                if (*(uint8_t*)((uint64_t)&test + 4) != 0x1 || *(uint8_t*)((uint64_t)&test + 5) != 0x0) {
                    continue;
                }

                continue;
            }

            if (!ptr_is_valid(test, VM_PROT_READ)) continue;
            std::cout << "0x" << depth << " 0x" << test << " 0x" << i * 8 << "\n";
            int result = lol(test, 0x60, depth + 1);
            if (result) return result;
        }
    }

    int* bytes = (int*)scan;
    for (int i = 0; i < 40; i++) {
        if (bytes[i] == vindex
        || bytes[i] == vindex - 1
        || bytes[i] == vindex + 1) {
            std::cout << "FOUND AT 0x" << scan + i * 4 << "\n";
            return bytes[i];
        }
    }

    return 0;
}

int blankfunction(uint64_t rl) {
    return 0;
}

struct signal_t;

struct signal_connection_t {
    char padding[20];
    int func_idx; //0x14
};

struct signal_data_t {
    uint64_t padding1;
    signal_t* root; //0x8
    uint64_t padding2[12];
    signal_connection_t* connection_data; //0x70
};

struct signal_t {
    uint64_t padding1[2];
    signal_t* next; //0x10
    uint64_t padding2;
    uint64_t state;
    uint64_t padding3;
    signal_data_t* signal_data; //0x30
};

int getallindexes(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument #1 for firesignal.");
        return 0;
    }

    (*rbx_createtable)(rl, 0, 0);
    auto signal = *(signal_t**)rbx_touserdata(rl, 1);
    signal = signal->next;
    int index = 1;

    while (signal) {
        (*rbx_pushnumber)(rl, signal->signal_data->connection_data->func_idx);
        (*rbx_rawseti)(rl, -2, index);
        signal = signal->next;
        index++;
    }

    return 1;
}

int signaltest(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument #1 for signaltest.");
    }

    uint64_t signal = *(uint64_t*)rbx_touserdata(rl, -1);
    std::cout << "Signal: 0x" << signal << "\n";

    int result = lol(signal, 0x60, 0);
    //auto testing = (signal_t*)signal;
    //auto testing2 = (roblox_structs::Udata*)((uint64_t)(testing->next) - 0x10);
    //std::cout << "Next: 0x" << unsigned(testing2->tt) << "\n";
    //std::cout << "Completed 1: 0x" << testing->signal_data->connection_data->func_idx << "\n";
    //std::cout << "Completed 2: 0x" << testing->signal_data->next->signal_data->connection_data->func_idx << "\n";
    std::cout << "Completed: 0x" << result << "\n";

/*
    auto current = (signal_t*)signal;
    while (current) {
        auto data = current->signal_data;
        std::cout << "Index: 0x" << data->connection_data->func_idx << "\n";
        current = current->next;
    }
*/
    /*
       for (int i = 0; i < 16; i++) {
        std::cout << "0x" << signal[i] << " 0x" << i * 8 << "\n";
    }

    std::cout << "\nTest1\n";
    uint64_t* test1 = (uint64_t*)signal[3];
    for (int i = 0; i < 12; i++) {
        std::cout << "0x" << test1[i] << " 0x" << i * 8 << "\n";
    }

    std::cout << "\nTest2\n";
    uint64_t* test2 = (uint64_t*)signal[4];
    for (int i = 0; i < 12; i++) {
        std::cout << "0x" << test2[i] << " 0x" << i * 8 << "\n";
    }
 
    */

    //uint64_t data = *(uint64_t*)(signal + 0x20);
    //std::cout << "Signal Test 1: 0x" << *(int*)(data) << "\n";
    //std::cout << "Signal Test 2: 0x" << *(int*)(data + 0x10) << "\n";
    //std::cout << "Signal Test 2: 0x" << *(int*)(data + 0x14) << "\n";
    //std::cout << "Signal Test 2: 0x" << *(uint64_t*)(signal + 0x20) << "\n";
    //std::cout << "Signal Address: 0x" << signal << "\n";

    std::string s;
    std::getline(std::cin, s);
    return 0;
}

namespace hwlib {
    bool enabled = true;
    int keypress(uint64_t rl) {
        (*rbx_checkany)(rl, 1);
        if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
            (*rbx_error)("Invalid argument #1 for keypress");
            return 0;
        }

        CGEventRef downEvt = CGEventCreateKeyboardEvent( NULL, 0, true );
        CGEventRef upEvt = CGEventCreateKeyboardEvent( NULL, 0, false );
        UniChar oneChar = static_cast<UniChar>(rbx_tointeger(rl, 1, 0));
        
        CGEventKeyboardSetUnicodeString(downEvt, 1, &oneChar);
        CGEventKeyboardSetUnicodeString(upEvt, 1, &oneChar);

        CGEventPost(kCGAnnotatedSessionEventTap, downEvt);
        CGEventPost(kCGAnnotatedSessionEventTap, upEvt);
        return 0;
    }

    int mousemove(uint64_t rl) {
        (*rbx_checkany)(rl, 2);
        if (rbx_gettype(rl, 1) != LUA_TNUMBER || rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid arguments to mousemove");
            return 0;
        }

        float x = rbx_tonumber(rl, 1);
        float y = rbx_tonumber(rl, 2);

        CGPoint point = CGPointMake(x, y);
        CGDisplayMoveCursorToPoint(CGMainDisplayID(), point);
        return 0;
    }

    int mousemoverel(uint64_t rl) {
        (*rbx_checkany)(rl, 2);
        if (rbx_gettype(rl, 1) != LUA_TNUMBER || rbx_gettype(rl, 2) != LUA_TNUMBER) {
            (*rbx_error)("Invalid arguments to mousemoverel");
            return 0;
        }

        float x = rbx_tonumber(rl, 1);
        float y = rbx_tonumber(rl, 2);

        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);

        CGPoint point = CGPointMake(cursor.x + x, cursor.y + y);
        CGDisplayMoveCursorToPoint(CGMainDisplayID(), point);
        return 0;
    }   
}

int fireclickdetector(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA || rbx_gettype(rl, 2) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid arguments to fireclickdetector.");
        return 0;
    }

    float dist = 0.0;
    if (rbx_gettop(rl) == 3 && rbx_gettype(rl, 3) == LUA_TNUMBER) {
        dist = rbx_tonumber(rl, 3);
    }

    uint64_t clickdetector = *(uint64_t*)rbx_touserdata(rl, 1);
    uint64_t player = *(uint64_t*)rbx_touserdata(rl, 2);   
    (*rbx_fireclick)(clickdetector, player, dist);
    return 0;
}

int getsignalname(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument for getsignalname", 0);
    }

    auto udata = (roblox_structs::Udata*)(**(uint64_t**)(rl + offsets::state_base));
    uint64_t data = (uint64_t)&udata->data;

    rbx_pushstring(rl, **(std::string**)(data + 0x8));
    return 1;
}

int scanudata(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument for scanudata", 0);
    }

    auto udata = (roblox_structs::Udata*)(**(uint64_t**)(rl + offsets::state_base));
    uint64_t data = (uint64_t)&udata->data;
    scan_pointer(data);
    return 0;
}

int getrawmetatable(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (!(*rbx_getmetatable)(rl, 1)) {
        rbx_pushnil(rl);
    }

    return 1;
}

int setrawmetatable(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    return (*rbx_setmetatable)(rl, 1);
}

int eval_userdata(uint64_t rl) {
    uint64_t ptr = **(uint64_t**)(rl + offsets::state_base);
    std::cout << "[Abyss] Element Pointer: 0x" << ptr << "\n";
    return 0;
}

int setreadonly(uint64_t rl) {
    (*rbx_checkany)(rl, 2);

    if (rbx_gettype(rl, 1) != LUA_TTABLE || rbx_gettype(rl, 2) != LUA_TBOOLEAN) {
        (*rbx_error)("Invalid arguments for setreadonly.", 0);
        return 0;
    }

    int readonly = *(int*)(*(uint64_t*)(rl + offsets::state_base) + 0x10);
    uint64_t table = **(uint64_t**)(rl + offsets::state_base);
    *(int8_t*)(table + offsets::table::isreadonly_offset) = (int8_t)readonly;
    return 0;
}

int teleport_test(uint64_t rl) {
    std::cout << *(int8_t*)(rl + 0xa9) << "\n";
    return 0;
}

int isreadonly(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TTABLE) {
        (*rbx_error)("Invalid arguments for setreadonly.", 0);
        return 0;
    }

    auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    uint64_t table = val->value.gcobject;
    int8_t readonly = *(int8_t*)(table + offsets::table::isreadonly_offset);
    rbx_pushboolean(rl, readonly);
    return 1;
}

int setserverteleports(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TBOOLEAN) {
        (*rbx_error)("invalid argument for serverteleports", 0);
    }

    auto val = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    prevent_server_teleports = !(val->value.b);
    (*rbx_print)(1, prevent_server_teleports ? "Abyss Disabled Teleports" : "Abyss Enabled Teleports");
    return 0;
}

int getgenv(uint64_t rl) {
    (*rbx_pushvalue)(rl, LUA_GLOBALSINDEX);
    return 1;
}

int checkidentity(uint64_t rl) {
    if (rbx_gettop(rl) != 0) {
        (*rbx_error)("checkidentity function requires 0 argument\n", 0);
        return 0;
    }

    printf("[Abyss] Current Identity: 0x%X\n", (int)*rbx_getidentity(rl));
    return 0;
}   

void sleep_test(uint64_t rl, int val) {
    sleep(val);
    (*rbx_resume)(rl, 0, 0);
}

int lua_sleep(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
        (*rbx_error)("Invalid argument for lua_sleep (#1)", 0);
        return 0;
    }

    auto number_value = reinterpret_cast<roblox_structs::TValue*>(rbx_index2addr(rl, 1));
    std::thread(sleep_test, rl, static_cast<int>(number_value->value.n)).detach();
    rbx_yield(rl, 0);
    return -1;
}

int clonefunction(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        (*rbx_error)("Invalid argument for clonefunction bruh");
        return 0;
    }

    auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    auto closure = (roblox_structs::Closure*)tval->value.gcobject;

    if (closure->isC) {
        uint64_t oldfunc = filter_ptr_encryption((uint64_t)&closure->c.f, FUNCTION_ENC);
        uint64_t oldcont = filter_ptr_encryption((uint64_t)&closure->c.cont, CONTINUE_ENC);

        for (int i = 0; i < closure->nupvalues; i++) {
            rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)&closure->c.upvals[i]);
            rbx_incrementtop(rl);
        }

        (*rbx_pushcclosure)(rl, (int(*)(uint64_t))oldfunc, "Abyss", closure->nupvalues, oldcont);
        return 1;
    }

    auto clone = (*rbx_newlclosure)(rl, closure->nupvalues, closure->env, filter_ptr_encryption((uint64_t)&closure->l.p, PROTO_ENC));
    for (int i = 0; i < closure->nupvalues; i++) {
        memcpy((void*)&clone->l.uprefs[i], (void*)&closure->l.uprefs[i], 0x10);
    }

    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)clone;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);
    return 1;
}

int cloneref(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument for cloneref.");
        return 0;
    }

    auto inst = (roblox_structs::Udata*)(*(uint64_t*)rbx_index2addr(rl, 1));
    (*rbx_newudata)(rl, inst->len, inst->tag);
    auto new_inst = (roblox_structs::Udata*)(*(uint64_t*)rbx_index2addr(rl, -1));
    new_inst->metatable = inst->metatable;
    *(uint64_t*)&new_inst->data = *(uint64_t*)&inst->data;
    return 1;
}

int loadstring(uint64_t rl) {
    (*rbx_checkany)(rl, 1);

    size_t script_size;
    const char* script_str = rbx_tolstring(rl, 1, &script_size);
    std::string script(script_str, script_size);

    std::string bytecode = Luau::compile(script);
    if ((*rbx_deserialize)(rl, script_author.c_str(), bytecode.c_str(), bytecode.size(), 0) != 0) {
        rbx_pushnil(rl);
        (*rbx_insert)(rl, -2);
        return 2; // (*rbx_error)(rbx_tolstring(rl, -1, nullptr));
    }

    return 1;
}

int create_illegal(uint64_t state) { // C API Closure
    rbx_pushstring(state, nullptr); // Illegal Byte
    auto tval = (roblox_structs::TValue*)rbx_index2addr(state, -1); // Access Raw TString Object
    //tval->value.gcobject + 
    //tval->tt = LUA_TNUMBER; // Manipulate Type Definition (for lual_typename check)
    return 1; // Return value to lua VM
}

void httpget_handler(uint64_t rl, std::string url) {
    std::string server_response = receive_string(url);
    rbx_pushstring(rl, server_response);
    rbx_continue(rl, 1);
}

int httpget(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (!settings::get_boolean("httpTraffic")) {
        (*rbx_error)("HTTP Disabled.");
        return 0;
    }

    if (rbx_gettype(rl, 1) != LUA_TSTRING) {
        (*rbx_error)("Invalid argument entered for httpget.");
        return 0;
    }

    std::string url(rbx_tolstring(rl, 1, nullptr));
    std::thread(httpget_handler, rl, url).detach();
    return rbx_yield(rl, 0);
}

int gethwid(uint64_t rl) {
    rbx_pushstring(rl, parsed_fingerprint().c_str());
    return 1;
}

void transfer_proto(roblox_structs::Closure* cl1, roblox_structs::Closure* cl2) {
    uint64_t original_proto = filter_ptr_encryption((uint64_t)&cl2->l.p, PROTO_ENC);
    cl1->l.p = original_proto + (uint64_t)&cl1->l.p;
}

int swapfunction(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    auto tval1 = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    auto tval2 = (roblox_structs::TValue*)rbx_index2addr(rl, 2);
    
    auto cl1 = (roblox_structs::Closure*)tval1->value.gcobject;
    auto cl2 = (roblox_structs::Closure*)tval2->value.gcobject;

    if (!cl2->isC) {
        (*rbx_error)("Invalid argument for swapfunction, should be C function.");
        return 0;
    }

    uint64_t cfunc = filter_ptr_encryption((uint64_t)&cl2->c.f, FUNCTION_ENC);
    uint64_t ccont = filter_ptr_encryption((uint64_t)&cl2->c.cont, CONTINUE_ENC);

    if (!cl1->isC) {
        auto cl = (*rbx_newlclosure)(rl, cl1->nupvalues, cl1->env, filter_ptr_encryption((uint64_t)&cl1->l.p, PROTO_ENC));
        for (int i = 0; i < cl->nupvalues; i++) {
            memcpy((void*)&cl->l.uprefs[i], (void*)&cl1->l.uprefs[i], 0x10);
        }

        auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
        top->value.gcobject = (uint64_t)cl;
        top->tt = LUA_TFUNCTION;
        rbx_incrementtop(rl);

        (*rbx_pushcclosure)(rl, newcclosure_handler_pcall, "Abyss", 1, (uint64_t)newcclosure_cont);
    } else {
        for (int i = 0; i < cl1->nupvalues; i++) {
            rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)&cl1->c.upvals[i]);
            rbx_incrementtop(rl);
        }

        uint64_t oldfunc = filter_ptr_encryption((uint64_t)&cl1->c.f, FUNCTION_ENC);
        uint64_t oldcont = filter_ptr_encryption((uint64_t)&cl1->c.cont, CONTINUE_ENC);
        (*rbx_pushcclosure)(rl, (int(*)(uint64_t))oldfunc, "Abyss", cl1->nupvalues, oldcont);
    }

    cl1->c.f = cfunc;
    cl1->c.f = filter_ptr_encryption((uint64_t)&cl1->c.f, FUNCTION_ENC, true);

    cl1->c.cont = ccont;
    cl1->c.cont = filter_ptr_encryption((uint64_t)&cl1->c.cont, CONTINUE_ENC, true);
    
    cl1->nupvalues = 0;
    cl1->isC = true;

    if (safe_newcclosure_map[(uint64_t)cl1]) {
        auto replacement_val = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
        safe_newcclosure_map[replacement_val->value.gcobject] = safe_newcclosure_map[(uint64_t)cl1];
    }
    
    safe_newcclosure_map[(uint64_t)cl1] = safe_newcclosure_map[(uint64_t)cl2];

/*
    return 1;

    uint64_t original_proto = PROTO_DEC((uint64_t)&cl1->l.p);
    auto original = (*rbx_newlclosure)(rl, cl1->nupvalues, cl1->env, original_proto);

    transfer_proto(cl1, cl2);
    for (int i = 0; i < cl1->nupvalues; i++) {
        memcpy((void*)&original->l.uprefs[i], (void*)&cl1->l.uprefs[i], 0x10);
    }

    cl1->env = cl2->env;
    cl1->nupvalues = cl2->nupvalues;
    for (int i = 0; i < cl2->nupvalues; i++) {
        memcpy((void*)&cl1->l.uprefs[i], (void*)&cl2->l.uprefs[i], 0x10);
    }

    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)original;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);
*/

    return 1;
}

int hookfunction_c(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    auto tval1 = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    auto tval2 = (roblox_structs::TValue*)rbx_index2addr(rl, 2);
    
    auto cl1 = (roblox_structs::Closure*)tval1->value.gcobject;
    auto cl2 = (roblox_structs::Closure*)tval2->value.gcobject;

    if (cl1->isC || cl2->isC) {
        (*rbx_error)("Invalid argument for hookfunction, use swapfunction to hook C closures.");
        return 0;
    }

    if (cl2->nupvalues > cl1->nupvalues) {
        (*rbx_error)("Too many upvalues to hook closure.");
        return 0;
    }

    uint64_t old_proto = filter_ptr_encryption((uint64_t)&cl1->l.p, PROTO_ENC);
    uint64_t new_proto = filter_ptr_encryption((uint64_t)&cl2->l.p, PROTO_ENC);
    auto replacement = (*rbx_newlclosure)(rl, cl1->nupvalues, cl1->env, old_proto);
    for (int i = 0; i < cl1->nupvalues; i++) {
        memcpy(&replacement->l.uprefs[i], &cl1->l.uprefs[i], 0x10);
    }

    cl1->nupvalues = cl2->nupvalues;
    cl1->env = cl2->env;

    cl1->l.p = new_proto;
    cl1->l.p = filter_ptr_encryption((uint64_t)&cl1->l.p, PROTO_ENC, true);

    for (int i = 0; i < cl2->nupvalues; i++) {
        memcpy(&cl1->l.uprefs[i], &cl2->l.uprefs[i], 0x10);
    }

    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)replacement;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);
    return 1;
}

int namecall_hook(uint64_t rl) {
    uint64_t ts_namecall = *(uint64_t*)(rl + 0x70);
    if (!ts_namecall) {
        return hook::rbx_namecall(rl);
    }

    /*
        (*rbx_getfield)(rl, LUA_GLOBALSINDEX, "httpget_async");
        auto test = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
        (*rbx_insert)(rl, 1);

        uint64_t func = *(uint64_t*)(rl + offsets::state_base);
        (*rbx_call)(rl, func, -1);
    */

    const char* method = (const char*)(ts_namecall + 0x18);
    if (*rbx_getidentity(rl) == global_identity) {
        std::cout << method << "\n";
    }
    
    if (!strcmp(method, "HttpGet") || !strcmp(method, "HttpGetAsync")) {
        *(uint64_t*)(rl + offsets::state_base) += 0x10;
        return httpget(rl);
    }

    if (!strcmp(method, "GetObjects")) {
        *(uint64_t*)(rl + offsets::state_base) += 0x10;
        (*rbx_getfield)(rl, LUA_GLOBALSINDEX, "get_objects");
        (*rbx_insert)(rl, 1);

        uint64_t func = *(uint64_t*)(rl + offsets::state_base);
        (*rbx_call)(rl, func, -1);
        return rbx_gettop(rl);
    }

    return hook::rbx_namecall(rl);
}

int console_print(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    std::cout << rbx_tolstring(rl, -1, nullptr) << "\n";
    return 0;
}

int getfunctionaddress(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    auto tv = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    auto cl = (roblox_structs::Closure*)(tv->value.gcobject);
    uint64_t function = filter_ptr_encryption((uint64_t)&cl->c.f, FUNCTION_ENC);

    if (cl->isC) {
        std::cout << "[Abyss] Function 0x" << function << "\n";
        std::cout << "[Abyss] Function 0x" << aslr_bypass(function) << "\n";
    }

    return 0;
}

int getscriptfromsrc(uint64_t rl) {
    const char* script_name = rbx_tolstring(rl, 1, nullptr);
    (*rbx_pushvalue)(rl, LUA_REGISTRYINDEX);
    rbx_pushnil(rl);

    while ((*rbx_next)(rl, -2)) {
        if (rbx_gettype(rl, -1) == LUA_TFUNCTION) {
            auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            auto cl = (roblox_structs::Closure*)tval->value.gcobject;
            if (!cl->isC) {
                rbx_pop(rl, 1);
                continue;
            }

            uint64_t proto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
            for (uint64_t search : game_script_dir) {
                if (search != proto) continue;
                return 1;
            }
        }

        rbx_pop(rl, 1);
    }

    (*rbx_error)("The script does not exist.");
    return 0;
}

int getscriptfromname(uint64_t rl) {
    const char* script_name = rbx_tolstring(rl, 1, nullptr);
    auto closure = (roblox_structs::Closure*)(game_script_map[script_name]);
    if (!closure) {
        (*rbx_error)("The script does not exist, has been moved or has not been loaded.");
        return 0;
    }

    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)closure;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);
    return 1;
}

int getrenv(uint64_t rl) {
    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = scripts_env;
    top->tt = LUA_TTABLE;
    rbx_incrementtop(rl);
    return 1;
}

int firesignal(uint64_t rl) {
    std::cout << "[Abyss] Attempt to fire RBXScriptSignal!\n";
    return 0;
}

struct connection_object {
    signal_t* signal;
    uint64_t state;
    uint64_t metatable;
    uint64_t root;
};

std::unordered_map<signal_t*, connection_object> connection_table;

int connection_blank(uint64_t rl) {
    return 0;
}

int disable_connection(uint64_t rl) {
    auto connection = (connection_object*)rbx_touserdata(rl, 1);
    if (connection->signal->state != 0)
        connection->state = connection->signal->state;

    connection->signal->state = 0;
    return 0;
}

int enable_connection(uint64_t rl) {
    auto connection = (connection_object*)rbx_touserdata(rl, 1);
    connection->signal->state = connection->state;
    return 0;
}

int disconnect_connection(uint64_t rl) {
    auto connection = (connection_object*)rbx_touserdata(rl, 1);
    auto root = (signal_t*)connection->root;
    if ((uint64_t)root == (uint64_t)connection) {
        (*rbx_error)("Cannot disconnect a root connection.");
        return 0;
    }

    while (root->next && root->next != connection->signal) {
        root = root->next;
    }

    if (!root->next) {
        (*rbx_error)("That connection has already been disconnected.");
        return 0;
    }

    root->next = root->next->next;
    connection->signal->state = 0;
    return 0;
}

int connection_index(uint64_t rl) {
    std::string key = std::string(rbx_tolstring(rl, 2, nullptr));
    auto connection = (connection_object*)rbx_touserdata(rl, 1);

    if (key == "Enabled" || key == "enabled") {
        rbx_pushboolean(rl, !(connection->signal->state == 0));
        return 1;
    }

    if (key == "Function" || key == "function" || key == "Fire" || key == "fire" || key == "Defer" || key == "defer") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;
            (*rbx_rawgeti)(rl, LUA_REGISTRYINDEX, index);

            if (rbx_gettype(rl, -1) != LUA_TFUNCTION)
                (*rbx_pushcclosure)(rl, connection_blank, "Abyss", 0, 0);
            
            return 1;
        }

        (*rbx_pushcclosure)(rl, connection_blank, "Abyss", 0, 0);
        return 1;
    }

    if (key == "LuaConnection") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;

            (*rbx_rawgeti)(rl, LUA_REGISTRYINDEX, index);
            auto func_tval = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            auto cl = (roblox_structs::Closure*)func_tval->value.gcobject;
            bool lua = !cl->isC;
            
            rbx_pop(rl, 1);
            rbx_pushboolean(rl, lua);
            return 1;
        }

        rbx_pushboolean(rl, false);
        return 1;
    }

    if (key == "ForeignState") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;

            (*rbx_rawgeti)(rl, LUA_REGISTRYINDEX, index);
            auto func_tval = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
            auto cl = (roblox_structs::Closure*)func_tval->value.gcobject;
            bool c = cl->isC;
            
            rbx_pop(rl, 1);
            rbx_pushboolean(rl, c);
            return 1;
        }

        rbx_pushboolean(rl, false);
        return 1;
    }

    if (key == "Disconnect" || key == "disconnect") {
        (*rbx_pushcclosure)(rl, disconnect_connection, "Abyss", 0, 0);
        return 1;
    }

    if (key == "Disable" || key == "disable") {
        (*rbx_pushcclosure)(rl, disable_connection, "Abyss", 0, 0);
        return 1;
    }

    if (key == "Enable" || key == "enable") {
        (*rbx_pushcclosure)(rl, enable_connection, "Abyss", 0, 0);
        return 1;
    }

    if (key == "Thread") {
        auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
        top->value.gcobject = rl;
        top->tt = LUA_TTHREAD;
        rbx_incrementtop(rl);
        return 1;
    }

    (*rbx_error)("Not a valid index of Connection.");
    return 0;
}

int getconnections(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid argument #1 for getconnections.");
        return 0;
    }

    auto signal = *(signal_t**)rbx_touserdata(rl, 1);
    signal = signal->next;
    
    (*rbx_createtable)(rl, 0, 0);
    auto signal_root = signal;
    int index = 1;

    while (signal) {
        int func_idx = signal->signal_data->connection_data->func_idx;

        if (!connection_table.count(signal)) {
            connection_object new_connection;
            new_connection.signal = signal;
            new_connection.root = (uint64_t)signal_root;
            new_connection.state = signal->state;
            connection_table[signal] = new_connection;
        }

        auto connection = (connection_object*)(*rbx_newudata)(rl, sizeof(connection_object), 0);
        *connection = connection_table[signal];

        (*rbx_createtable)(rl, 0, 0);
        (*rbx_pushcclosure)(rl, connection_index, "Abyss", 0, 0);
        (*rbx_setfield)(rl, -2, "__index");

        rbx_pushstring(rl, "Connection");
        (*rbx_setfield)(rl, -2, "__type");
        (*rbx_setmetatable)(rl, -2);

        (*rbx_rawseti)(rl, -2, index++);
        signal = signal->next;
    }

    return 1;
}

int getcustomasset(uint64_t rl) {
    rbx_pushstring(rl, "");
    return 1;
}

int crashroblox(uint64_t rl) {
    exit(127);
    return 0;
}

int isexecutorclosure(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        (*rbx_error)("Invalid argument #1 for isexecutorclosure");
        return 0;
    }

    auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, 1);
    auto cl = (roblox_structs::Closure*)tval->value.gcobject;
    roblox_structs::lua_Debug debug_info;
    (*rbx_getinfo)(rl, -1, "sn", &debug_info);
    rbx_pop(rl, 1);
    
    if (cl->isC) {
        rbx_pushboolean(rl, !strcmp(debug_info.name ? debug_info.name : "", "Abyss"));
        return 1;
    }

    rbx_pushboolean(rl, !strcmp(debug_info.source ? debug_info.source : "", script_author.c_str()));
    return 1;
}

int setfpscap(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TNUMBER) {
        (*rbx_error)("Invalid argument #1 for setfpscap");
        return 0;
    }

    double fps_cap = rbx_tonumber(rl, 1);
    *(double*)(*(uint64_t*)taskscheduler_address + 0x138) = 1 / (fps_cap == 0.0 ? 60.0 : fps_cap);
    return 0;
}

int logscfds(uint64_t rl) {
    std::string data = std::string(rbx_tolstring(rl, 1, nullptr));
    // log_exec(data);
    return 0;
}

int messagebox(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    if (rbx_gettype(rl, 1) != LUA_TSTRING || rbx_gettype(rl, 2) != LUA_TSTRING) {
        (*rbx_error)("Invalid argument #1 for messagebox");
        return 0;
    }

    const char* text = rbx_tolstring(rl, 1, nullptr);
    const char* caption = rbx_tolstring(rl, 2, nullptr);

    int style = 0;
    if (rbx_gettype(rl, 3) == LUA_TNUMBER) {
        style = rbx_tointeger(rl, 3, 0);
    }

    CFOptionFlags flags;
    switch (style) {
        case 0:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("OK"), NULL, NULL, &flags);

            (*rbx_pushnumber)(rl, 1);
            return 1;
        case 1:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Ok"), CFSTR("Cancel"), NULL, &flags);
            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 1 : 2);
            return 1;
        case 2:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Abort"), CFSTR("Retry"), CFSTR("Ignore"), &flags);

            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 3 : (flags == kCFUserNotificationAlternateResponse ? 4 : 5));
            return 1;
        case 3:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Yes"), CFSTR("No"), CFSTR("Cancel"), &flags);

            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 6 : (flags == kCFUserNotificationAlternateResponse ? 7 : 2));
            return 1;
        case 4:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Yes"), CFSTR("No"), NULL, &flags);

            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 6 : 7);
            return 1;
        case 5:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Retry"), CFSTR("Cancel"), NULL, &flags);

            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 4 : 2);
            return 1;
        case 6:
            CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
                NULL, NULL, NULL,
                CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8),
                CFStringCreateWithCString(NULL, caption, kCFStringEncodingUTF8),
                CFSTR("Cancel"), CFSTR("Try Again"), CFSTR("Continue"), &flags);

            (*rbx_pushnumber)(rl, flags == kCFUserNotificationDefaultResponse ? 2 : (flags == kCFUserNotificationAlternateResponse ? 10 : 11));
            return 1;
        default:
            (*rbx_error)("Invalid style argument for messagebox");
            return 0;
    }
}

int getscriptbytecode(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    uint64_t ud = *(uint64_t*)(*rbx_touserdata)(rl, 1);
    std::cout << "[Func] Userdata: 0x" << *(uint64_t*)(ud + 0x20) << "\n";
    sleep(500);
    return 0;
}

void init_files(uint64_t rl) {
    if (!std::filesystem::is_directory(filelibrary::workspace_path)) {
        std::cout << "[Abyss] Workspace directory does not exist, creating...\n";
        std::filesystem::create_directory(filelibrary::workspace_path);
    }

    rbx_register(rl, filelibrary::readfile, "readfile");
    rbx_register(rl, filelibrary::writefile, "writefile");
    rbx_register(rl, filelibrary::loadfile, "loadfile");
    rbx_register(rl, filelibrary::appendfile, "appendfile");
    rbx_register(rl, filelibrary::listfiles, "listfiles");
    rbx_register(rl, filelibrary::isfile, "isfile");
    rbx_register(rl, filelibrary::isfolder, "isfolder");
    rbx_register(rl, filelibrary::makefolder, "makefolder");
    rbx_register(rl, filelibrary::delfolder, "delfolder");
    rbx_register(rl, filelibrary::delfile, "delfile");
    std::cout << "[Abyss] File Library Ready.\n";
}

int getuserdatascript(uint64_t rl) {
    (*rbx_getfield)(rl, -1, "script");
    return 1;
}

int userdatatotable(uint64_t rl) {
    (*rbx_createtable)(rl, 0, 0);
    (*rbx_pushvalue)(rl, -2);
    rbx_pushnil(rl);

    while ((*rbx_next)(rl, -2)) {
        (*rbx_pushvalue)(rl, -1);
        std::cout << "Stack: 0x" << rbx_gettop(rl) << "\n";
        std::cout << "Item: " << rbx_tolstring(rl, -3) << "\n";
        std::cout << "Value: 0x" << rbx_gettype(rl, -1) << " 0x" << rbx_gettype(rl, -5) << "\n";
        (*rbx_setfield)(rl, -5, rbx_tolstring(rl, -3));
        rbx_pop(rl, 1);
        std::cout << "Set!\n";
    }

    rbx_pop(rl, 1);
    std::cout << "Stack: 0x" << rbx_gettop(rl) << "\n";
    return 1;
}

void protoinfo(uint64_t rl, uint64_t subproto) {
    (*rbx_createtable)(rl, 0, 0);
    int sizecode = *(int*)(subproto + offsets::proto::sizecode);
    (*rbx_pushnumber)(rl, sizecode);
    (*rbx_setfield)(rl, -2, "sizeCode");
    uint32_t* code = (uint32_t*)filter_ptr_encryption(subproto + offsets::proto::code, CODE_ENC);
    
    (*rbx_createtable)(rl, sizecode, 0);
    for (int j = 0; j < sizecode; j++) {
        (*rbx_pushnumber)(rl, code[j]);
        (*rbx_rawseti)(rl, -2, j + 1);
    }

    (*rbx_setfield)(rl, -2, "codeTable");
    int sizek = *(int*)(subproto + offsets::proto::sizek);

    (*rbx_pushnumber)(rl, sizek);
    (*rbx_setfield)(rl, -2, "sizeConsts");

    (*rbx_createtable)(rl, sizek, 0);
    auto constants = (roblox_structs::TValue*)(filter_ptr_encryption((subproto + offsets::proto::constants), CONSTANT_ENC));
    for (int j = 0; j < sizek; j++) {
        (*rbx_createtable)(rl, 0, 1);
        rbx_setsvalue(rbx_getstacktop(rl), (uint64_t)&constants[j]);
        rbx_incrementtop(rl);

        (*rbx_setfield)(rl, -2, "value");
        (*rbx_rawseti)(rl, -2, j + 1);
    }

    (*rbx_setfield)(rl, -2, "kTable");

    int sizep = *(int*)(subproto + offsets::proto::sizep);
    (*rbx_createtable)(rl, sizep, 0);

    uint64_t* protos = (uint64_t*)filter_ptr_encryption(subproto + offsets::proto::protos, PROTOS_ENC);
    for (int i = 0; i < sizep; i++) {
        uint64_t subproto2 = protos[i];
        protoinfo(rl, subproto2);
        (*rbx_rawseti)(rl, -2, i + 1);
    }

    (*rbx_setfield)(rl, -2, "pTable");

    auto cl = (*rbx_newlclosure)(rl, 0, 0, subproto);
    auto top = (roblox_structs::TValue*)rbx_getstacktop(rl);
    top->value.gcobject = (uint64_t)cl;
    top->tt = LUA_TFUNCTION;
    rbx_incrementtop(rl);

    roblox_structs::lua_Debug debug;
    (*rbx_getinfo)(rl, -1, "flnsu", &debug);
    rbx_pop(rl, 2);

    uint8_t numparams = debug.nparams;
    bool is_vararg = debug.isvararg;

    if (debug.name && strcmp(debug.name, "")) {
        rbx_pushstring(rl, debug.name);
        (*rbx_setfield)(rl, -2, "source");
    }

    rbx_pushboolean(rl, is_vararg);
    (*rbx_setfield)(rl, -2, "isVarArg");

    (*rbx_pushnumber)(rl, numparams);
    (*rbx_setfield)(rl, -2, "numParams");
}

int getprotoinfo(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (rbx_gettype(rl, 1) != LUA_TFUNCTION) {
        (*rbx_error)("Failed to validate parsed closure.");
        return 0;
    }

    auto tval = (roblox_structs::TValue*)rbx_index2addr(rl, -1);
    auto cl = (roblox_structs::Closure*)tval->value.gcobject;
    (*rbx_createtable)(rl, 0, 0);

    uint64_t mainproto = filter_ptr_encryption((uint64_t)&cl->l.p, PROTO_ENC);
    int sizep = *(int*)(mainproto + offsets::proto::sizep);

    if (sizep) {
        uint64_t* protos = (uint64_t*)filter_ptr_encryption(mainproto + offsets::proto::protos, PROTOS_ENC);
        (*rbx_createtable)(rl, sizep, 0);

        for (int i = 0; i < sizep; i++) {
            uint64_t subproto = protos[i];
            protoinfo(rl, subproto);
            (*rbx_rawseti)(rl, -2, i + 1);
        }
    }

    protoinfo(rl, mainproto);
    return 2;
}

int fetch_setting(uint64_t rl) {
    (*rbx_checkany)(rl, 1);
    if (!settings::get_boolean("macsploit")) {
        (*rbx_print)(2, "Unknown error while loading settings, reloading...");
        settings::reload_config();
    }

    std::string key = rbx_tolstring(rl, 1);
    (*rbx_pushboolean)(rl, settings::get_boolean(key));
    return 1;
}

void init_functions(uint64_t rl) {
    rbx_register(rl, userdatatotable, "userdatatotable");
    rbx_register(rl, getuserdatascript, "getuserdatascript");
    rbx_register(rl, checkidentity, "checkidentity");
    rbx_register(rl, httpget, "httpget");
    rbx_register(rl, loadstring, "loadstring");
    rbx_register(rl, setreadonly, "setreadonly");
    rbx_register(rl, getgenv, "getgenv");
    rbx_register(rl, getrawmetatable, "getrawmetatable");
    rbx_register(rl, isreadonly, "isreadonly");
    rbx_register(rl, newcclosure, "newcclosure");
    rbx_register(rl, safe_newcclosure, "newcclosure_s");
    rbx_register(rl, checkcaller, "checkcaller");
    rbx_register(rl, getnamecall, "getnamecallmethod");
    rbx_register(rl, scanudata, "scanudata");
    rbx_register(rl, swapfunction, "swapfunction");
    rbx_register(rl, hookfunction_c, "hookfunction_c");
    rbx_register(rl, setnamecallmethod, "setnamecallmethod");
    rbx_register(rl, islclosure, "islclosure");
    rbx_register(rl, iscclosure, "iscclosure");
    rbx_register(rl, queue_on_teleport, "queue_on_teleport");
    rbx_register(rl, getclipboard, "getclipboard");
    rbx_register(rl, setclipboard, "setclipboard");
    rbx_register(rl, setfpscap, "setfpscap");

    rbx_register(rl, setidentity, "setidentity");
    rbx_register(rl, setidentity, "setthreadcontext");
    rbx_register(rl, getidentity, "getthreadcontext");
    rbx_register(rl, setidentity, "set_thread_identity");
    rbx_register(rl, setidentity, "set_thread_context");
    rbx_register(rl, getidentity2, "get_thread_context");
    rbx_register(rl, getidentity2, "get_thread_identity");
    rbx_register(rl, getidentity, "getidentity");

    rbx_register(rl, setrawmetatable, "setrawmetatable");
    rbx_register(rl, setserverteleports, "set_teleporting_behaviour");
    rbx_register(rl, getfunctionaddress, "getfunctionaddress");
    rbx_register(rl, setidentity, "setthreadcontext");
    rbx_register(rl, setidentity, "setcontext");
    rbx_register(rl, getidentity, "getcontext");
    rbx_register(rl, gethwid, "gethwid");
    rbx_register(rl, clonefunction, "clonefunction");
    rbx_register(rl, isexecutorclosure, "isexecutorclosure");
    rbx_register(rl, crashroblox, "robloxcrash");

    rbx_register(rl, getconnections, "getconnections_c");
    rbx_register(rl, fireclickdetector, "fireclickdetector_c");
    rbx_register(rl, setindex, "setindex");

    rbx_register(rl, identifyexecutor, "identifyexecutor");
    rbx_register(rl, getrenv, "getrenv");
    rbx_register(rl, getcustomasset, "getcustomasset");
    rbx_register(rl, getcustomasset, "getsynasset");
    rbx_register(rl, getrenv, "getrobloxenv");

    rbx_register(rl, loadstring, "load");
    rbx_register(rl, lua_sleep, "lua_sleep");
    rbx_register(rl, teleport_test, "teleport_test");
    rbx_register(rl, signaltest, "signaltest");
    rbx_register(rl, getsignalname, "getsignalname");
    rbx_register(rl, console_print, "console_print");
    rbx_register(rl, getallindexes, "getallindexes");
    rbx_register(rl, messagebox, "messagebox");
    // rbx_register(rl, logscfds, "logscfds");

    rbx_register(rl, debug::getproto, "getproto");
    rbx_register(rl, debug::getprotos, "getprotos");
    rbx_register(rl, debug::getconstants, "getconstants");
    rbx_register(rl, debug::getregistry, "getreg");
    rbx_register(rl, debug::setupvalue, "setupvalue");
    rbx_register(rl, debug::getupvalue, "getupvalue");
    rbx_register(rl, debug::getupvalues, "getupvalues");
    rbx_register(rl, debug::setconstant, "setconstant");
    rbx_register(rl, debug::getconstant, "getconstant");
    rbx_register(rl, garbage_collector::getgc, "getgc");

    rbx_register(rl, base64::base64encode, "base64encode");
    rbx_register(rl, base64::base64decode, "base64decode");
    
    rbx_register(rl, http_request, "http_request_c");
    rbx_register(rl, httpget, "httpget_c");
    rbx_register(rl, getprotoinfo, "getprotoinfo");
    rbx_register(rl, websocket_connect, "websocketconnect_c");
    rbx_register(rl, firetouchinterest, "firetouchinterest_c");
    rbx_register(rl, fetch_setting, "fetch_setting");
    rbx_register(rl, create_illegal, "create_illegal");

    if (hwlib::enabled) {
        rbx_register(rl, hwlib::mousemove, "mousemove"); // Dangerous
        rbx_register(rl, hwlib::mousemoverel, "mousemoverel");
        rbx_register(rl, hwlib::keypress, "keypress");
    }

    rbx_registerlib(rl, "bit", {
        { "bdiv",  bitop::bit_bdiv },
        { "badd",  bitop::bit_badd },
        { "bsub",  bitop::bit_bsub },
        { "bmul",  bitop::bit_bmul },
        { "band",  bitop::bit_band },
        { "bor",  bitop::bit_bor },
        { "bxor",  bitop::bit_bxor },
        { "bnot",  bitop::bit_bnot },
        { "bswap",  bitop::bit_bswap },
        { "ror",  bitop::bit_ror },
        { "rol",  bitop::bit_rol },
        { "tohex",  bitop::bittohex },
        { "tobit",  bitop::bit_tobit },
        { "lshift",  bitop::bit_lshift },
        { "rshift",  bitop::bit_rshift },
        { "arshift",  bitop::bit_rshift }
    });

    rbx_registerlib(rl, "crypt", {
        { "generatebytes", crypto::generatebytes },
        { "generatekey", crypto::generatekey },
        { "base64encode", base64::base64encode },
        { "base64decode", base64::base64decode },
        { "encrypt", crypto::encrypt },
        { "decrypt", crypto::decrypt },
        { "hash", crypto::hash }
    });

    rbx_registerlib(rl, "debug", {
        { "getregistry", debug::getregistry },
        { "getupvalue", debug::getupvalue },
        { "getupvalues", debug::getupvalues },
        { "setupvalue", debug::setupvalue },
        { "getinfo", debug::getinfo },
        { "getprotos", debug::getprotos },
        { "getproto", debug::getproto },
        { "getconstant", debug::getconstant },
        { "setconstant", debug::setconstant },
        { "getconstants", debug::getconstants },
        { "setstack", debug::setstack },
        { "getstack", debug::getstack },
        { "getmetatable", getrawmetatable },
        { "setmetatable", setrawmetatable }
    });

    init_files(rl);
}

#endif //CLIENT_FUNCTIONS_H