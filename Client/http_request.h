//
// Created by Nexus Pancakes on 9/11/2022.
//

#ifndef CLIENT_HTTP_REQUEST_H
#define CLIENT_HTTP_REQUEST_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>
#include <curl/curl.h>
#include <unordered_map>

std::unordered_map<int, std::string> status_messages = {
    {200, "OK"},
    {302, "Server Redirect"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

typedef std::unordered_map<std::string, std::string> string_map;

struct http_response {
    bool response_success;
    bool succeeded;
    long status_code;
    std::string response_body;
    string_map headers;
    string_map cookies;
};

size_t curl_writedata(char* contents, size_t len, size_t nmeb, std::string* response) {
    size_t new_len = len * nmeb;
    response->append(contents, new_len);
    return new_len;
}

http_response execute_request(std::string url, std::string method, std::string body, string_map headers, string_map cookies) {
    http_response response;
    CURL* curl;

    curl = curl_easy_init();
    if (!curl) {
        response.succeeded = false;
        return response;
    }

    std::string response_headers;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writedata);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_writedata);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.response_body);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else if (method == "HEAD") {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    }

    struct curl_slist* curl_headers;
    std::string fingerprint = parsed_fingerprint();
    if (headers.find("User-Agent") == headers.end()) {
        curl_headers = curl_slist_append(NULL, "User-Agent: Roblox/WinInet");
    } else {
        curl_headers = curl_slist_append(NULL, std::string("User-Agent: " + headers["User-Agent"]).c_str());
    }

    for (auto pair : headers) {
        curl_headers = curl_slist_append(curl_headers, std::string(pair.first + ": " + pair.second).c_str());
    }

    curl_headers = curl_slist_append(curl_headers, std::string("Macsploit-Fingerprint: " + fingerprint).c_str());

    if (settings::get_boolean("compatibilityMode")) {
        curl_headers = curl_slist_append(curl_headers, std::string("Hydrogen-Fingerprint: " + fingerprint).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

    if (cookies.size() > 0) {
        std::string curl_cookies;
        for (auto pair : cookies) {
            curl_cookies = curl_cookies + std::string(pair.first + "=" + pair.second + "; ");
        }

        curl_easy_setopt(curl, CURLOPT_COOKIE, curl_cookies.c_str());
    }
    
    curl_easy_perform(curl);
    std::string response_cookies;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

    size_t pos = 0;
    while ((pos = response_headers.find("\n")) != std::string::npos) {
        std::string line = response_headers.substr(0, pos);
        std::string key = line.substr(0, line.find(": "));
        line.erase(0, line.find(": ") + 2);
        response.headers[key] = line;
        response_headers.erase(0, pos + 1);
    }

    for (auto pair : response.headers) {
        if (pair.first == "set-cookie") {
            size_t pos = 0;
            std::string data = pair.second;
            while ((pos = data.find("=")) != std::string::npos) {
                std::string key = data.substr(0, pos);
                data.erase(0, pos + 1);
                std::string value = data.substr(0, data.find("; "));
                data.erase(0, data.find("; ") + 2);
                response.cookies[key] = value;
            }
        }
    }

    curl_easy_cleanup(curl);
    response.response_success = response.status_code >= 200 && response.status_code < 300;
    response.succeeded = true;
    return response;
}

std::string receive_string(std::string url) {
    CURL* curl;

    curl = curl_easy_init();
    if (curl) {
        std::string response;
        struct curl_slist* curl_headers = curl_slist_append(NULL, "User-Agent: Roblox/WinInet");
        curl_headers = curl_slist_append(curl_headers, std::string("Macsploit-Fingerprint: " + fetch_fingerprint()).c_str());

        if (settings::get_boolean("compatibilityMode")) {
            curl_headers = curl_slist_append(curl_headers, std::string("Hydrogen-Fingerprint: " + fetch_fingerprint()).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writedata);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return response;
    } else {
        return "Failed to download.";
    }
}

#endif //CLIENT_HTTP_REQUEST_H
