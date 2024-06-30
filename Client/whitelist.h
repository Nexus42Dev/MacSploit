#include <iostream>
#include <unistd.h>

#include <ApplicationServices/ApplicationServices.h>

#include "http_request.h"
#include "exploit.h"

std::string whitelist_url = OBFUSCATE("https://abyss-exploit.vercel.app/api/whitelist?utm_source=client&hwid=");

void update_whitelist(CURL* curl, std::string hwid, std::string option) {
    std::string url = whitelist_url + hwid + "&opt=" + option;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_perform(curl);
}

void whitelist_check(bool use_discord) {
    sleep(1);
    std::string hwid = fetch_fingerprint();
    std::string url = whitelist_url + hwid;
    std::string response;
    CURL* curl;

    curl = curl_easy_init();
    if (!curl) {
        std::cout << "[Whitelist] Failed to initialise cURL.\n";
        exit(-1);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writedata);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);

    if (response == "") {
        std::cout << "[Whitelist] Empty Server Response Error.\n";
        std::cout << "[Whitelist] Ticket: 0x" << hwid << "\n";
        exit(-1); return;
    }

    if (response.find("Whitelist check complete.") == std::string::npos) {
        std::cout << "[Whitelist] You do not have permission to use MacSploit.\n";
        std::cout << "[Whitelist] Ticket: 0x" << hwid << "\n";
        CFUserNotificationDisplayAlert(5, kCFUserNotificationCautionAlertLevel,
            NULL, NULL, NULL,
            CFSTR("Whitelist Check Error"),
            CFSTR("This device has not been whitelisted via milo's secure client authentication system. Please contact support in a ticket to get whitelisted!"),
            NULL, NULL, NULL, NULL);

        exit(-1);
    }

    boosting_status = response.find("Server Booster.") != std::string::npos;
    if (boosting_status && use_discord && current_port == 5553) {
        setup_discord_rpc();
        std::cout << "[Abyss] Discord RPC Initialized.\n";
    }

    early_access = response.find("Early Access.") != std::string::npos;
    blox_products = response.find("Bloxproducts.") != std::string::npos;
    bool configured = response.find("bloxproducts_CONFIG.") != std::string::npos;
    whitelisted = true;

    if (blox_products && !configured) {
        CFOptionFlags flags;
        CFUserNotificationDisplayAlert(-1, kCFUserNotificationNoteAlertLevel,
            NULL, NULL, NULL,
            CFSTR("Bloxproducts Buyer"),
            CFSTR("Thank you for buying macsploit. We are very thankful, however I wonder how you found out about macsploit. Please answer this survey question: did you find out about macsploit from bloxproducts or from somewhere else?"),
            CFSTR("Bloxproducts"), CFSTR("Elsewhere"), NULL, &flags);

        if (flags != kCFUserNotificationDefaultResponse) {
            update_whitelist(curl, hwid, "bloxproducts_CONFIG");
        }
    }

    curl_easy_cleanup(curl);
    std::cout << "[Whitelist] Successfully Authenticated.\n";
}