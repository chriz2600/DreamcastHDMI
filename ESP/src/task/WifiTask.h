#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include "../global.h"
#include <Task.h>

extern TaskManager taskManager;
extern bool inInitialSetupMode;
extern IPAddress ipAddress;
extern char confIPAddr[24];
extern char confIPGateway[24];
extern char confIPMask[24];
extern char confIPDNS[24];
extern char host[64];
extern char ssid[64];
extern char password[64];
extern char httpAuthUser[64];
extern char httpAuthPass[64];
extern char WiFiAPPSK[12];
extern char AP_NameChar[64];

bool generate_password(char *buffer);
void setupHTTPServer();
void refreshWiFiMenu();

void generateWiFiPassword() {
    generate_password(WiFiAPPSK);
}

void _setupAPMode(const char *ssid, const char *psk) {
    WiFi.mode(WIFI_AP);
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = String("DCDigital") + String("-") + macID;

    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (uint i=0; i<AP_NameString.length(); i++) {
        if (i < 63) {
            AP_NameChar[i] = AP_NameString.charAt(i);
        }
    }

    generateWiFiPassword();
    if (ssid && psk) {
        strcpy(AP_NameChar, ssid);
        strcpy(WiFiAPPSK, psk);
    }

    WiFi.softAP(AP_NameChar, WiFiAPPSK);
    DEBUG2(">> SSID:   %s\n", AP_NameChar);
    DEBUG2(">> AP-PSK: %s\n", WiFiAPPSK);
}

enum Init_States { AP, STA };

class WifiTask : public Task {

    public:
        WifiTask(int num_tries) :
            Task(500),
            tries(num_tries)
        { };

        bool isReady() {
            return initDone;
        }

        bool isAPOnly() {
            return apOnly;
        }


    private:
        enum Init_States initState;
        bool initDone = false;
        bool staConnected = false;
        bool apOnly = true;
        uint8_t tries;

        virtual bool OnStart() {
            WiFi.persistent(false);
            if (strlen(ssid) == 0) {
                DEBUG2(">> No ssid, starting AP mode...\n");
                initState = AP;
            } else {
                DEBUG2(">> Trying to connect in client mode first...\n");
                initState = STA;
            }
            staConnected = false;
            initDone = false;
            tries = 0;
            trigger();
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (initDone) {
                taskManager.StopTask(this);
                return;
            }
            trigger();
        }

        virtual void OnStop() {
            refreshWiFiMenu();
            setupHTTPServer();
            DEBUG2(">> httpAuthUser: %s\n", httpAuthUser);
            DEBUG2(">> httpAuthPass: %s\n", httpAuthPass);
            DEBUG2(">> Ready.\n");
            DEBUG("done.\n");
        }

        void trigger() {
            switch (initState) {
                case AP:
                    setupAPMode();
                    break;
                case STA:
                    setupWiFiStation();
                    break;
            }
        }

        void setupAPMode() {
            _setupAPMode(NULL, NULL);
            inInitialSetupMode = (strlen(ssid) == 0);
            initDone = true;
        }

        void setupWiFiStation() {
            if (!staConnected) {
                if (tries == 0) {
                    bool doStaticIpConfig = false;
                    IPAddress ipAddr;
                    doStaticIpConfig = ipAddr.fromString(confIPAddr);
                    IPAddress ipGateway;
                    doStaticIpConfig = doStaticIpConfig && ipGateway.fromString(confIPGateway);
                    IPAddress ipMask;
                    doStaticIpConfig = doStaticIpConfig && ipMask.fromString(confIPMask);
                    IPAddress ipDNS;
                    doStaticIpConfig = doStaticIpConfig && ipDNS.fromString(confIPDNS);

                    //WIFI INIT
                    WiFi.mode(WIFI_STA/*WIFI_AP_STA*/);
                    WiFi.persistent(false);
                    WiFi.setAutoConnect(false);
                    WiFi.softAPdisconnect(true);
                    WiFi.setSleepMode(WIFI_NONE_SLEEP, 0);

                    DEBUG2(">> Do static ip configuration: %i\n", doStaticIpConfig);
                    if (doStaticIpConfig) {
                        WiFi.config(ipAddr, ipGateway, ipMask, ipDNS);
                    }
                    DEBUG2(">> WiFi.getAutoConnect: %i\n", WiFi.getAutoConnect());
                    DEBUG2(">> Connecting to %s\n", ssid);
                }

                if (WiFi.status() != WL_CONNECTED) {
                    if (tries % 20 == 0) {
                        DEBUG2("\n>> WiFi.begin.\n");
                        WiFi.disconnect();
                        WiFi.begin(ssid, password);
                    } else {
                        DEBUG2(".%02x", WiFi.status());
                    }

                    if (tries >= 60) {
                        WiFi.disconnect();
                        initState = AP;
                        return;
                    }

                    tries++;
                    return;
                }

                staConnected = true;
                DEBUG2("\n>> success: %i\n", staConnected);
                return;
            }

            WiFi.hostname(host);
            WiFi.setAutoReconnect(true);

            ipAddress = WiFi.localIP();
            IPAddress gateway = WiFi.gatewayIP();
            IPAddress subnet = WiFi.subnetMask();

            DEBUG2(">> Connected!\n   IP address:      %d.%d.%d.%d\n", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
            DEBUG2("   Gateway address: %d.%d.%d.%d\n", gateway[0], gateway[1], gateway[2], gateway[3]);
            DEBUG2("   Subnet mask:     %d.%d.%d.%d\n", subnet[0], subnet[1], subnet[2], subnet[3]);
            DEBUG2("   Hostname:        %s.local\n", host);
            apOnly = false;
            initDone = true;
        }
    
};

#endif