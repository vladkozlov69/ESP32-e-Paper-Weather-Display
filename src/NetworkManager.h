#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#include <esp_wifi.h>
#include "WiFi.h"
#include <ESPAsyncWiFiManager.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

typedef struct {
    String ssid;
    String pass;
    int gmt_offset;
    int dst_offset;
    String posix_tz;
    String owm_key;
    String accu_key;
    String accu_loc;
    String tomorrow_io_key;
    String latitude;
    String longitude;
    String iana_tz;
} NetworkSettings;

class NetworkManager
{
private:
    Stream * m_Debug;
    Preferences * m_Prefs;
    NetworkSettings * m_Settings;

    void (*m_NmStatusCallbackFunc)(int);
    AsyncWiFiManager * wm = NULL;
    WiFiClient espClient;    
public:
    AsyncWebServer * m_WebServer;
    DNSServer * m_DnsServer;
public:
    NetworkManager(Preferences * prefs, Stream * debug, NetworkSettings * settings);   
    void loadSettings();
    void begin();
    void reset();
};






#endif