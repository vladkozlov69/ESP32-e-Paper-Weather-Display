#include <NetworkManager.h>

NetworkManager::NetworkManager(Preferences * prefs, Stream * debug, NetworkSettings * settings)
{
    m_Prefs = prefs;
    m_Debug = debug;
    m_Settings = settings;
}

void NetworkManager::begin()
{
    m_WebServer = new AsyncWebServer(80);
    m_DnsServer = new DNSServer();

    wm = new AsyncWiFiManager(m_WebServer, m_DnsServer);

    if (m_Debug)
    {
        m_Debug->println(F("Starting ConfigPortal..."));
    }

    AsyncWiFiManagerParameter wmp_gmtOffset(      "gmtOffset",       "GMT Offset, H",   String(m_Settings->gmt_offset).c_str(),  2);
    AsyncWiFiManagerParameter wmp_dstOffset(      "dstOffset",       "DST Offset, H",   String(m_Settings->dst_offset).c_str(),  2);
    AsyncWiFiManagerParameter wmp_posixTZ(        "posixTZ",         "posixTZ",         m_Settings->posix_tz.c_str(),  40);
    AsyncWiFiManagerParameter wmp_owm_key(        "owm_key",         "OWM API Key",     m_Settings->owm_key.c_str(),  40);
    AsyncWiFiManagerParameter wmp_accu_key(       "accu_key",        "AccuWeather Key", m_Settings->accu_key.c_str(),  40);
    AsyncWiFiManagerParameter wmp_accu_loc(       "accu_loc",        "AccuWeather Loc", m_Settings->accu_loc.c_str(),  40);
    AsyncWiFiManagerParameter wmp_tomorrow_io_key("climacell_key",   "Tomorrow.io Key", m_Settings->tomorrow_io_key.c_str(),  40);
    AsyncWiFiManagerParameter wmp_lat(            "latitude",        "Latitude",        m_Settings->latitude.c_str(),  40);
    AsyncWiFiManagerParameter wmp_long(           "longitude",       "Longitude",       m_Settings->longitude.c_str(),  40);
    AsyncWiFiManagerParameter wmp_iana_tz(        "iana_tz",         "IANA Timezone",   m_Settings->iana_tz.c_str(),  40);


    wm->addParameter(&wmp_gmtOffset);
    wm->addParameter(&wmp_dstOffset);
    wm->addParameter(&wmp_posixTZ);
    wm->addParameter(&wmp_owm_key);
    wm->addParameter(&wmp_accu_key);
    wm->addParameter(&wmp_accu_loc);
    wm->addParameter(&wmp_tomorrow_io_key);
    wm->addParameter(&wmp_lat);
    wm->addParameter(&wmp_long);
    wm->addParameter(&wmp_iana_tz);

    wm->setTryConnectDuringConfigPortal(true);

    if(wm->startConfigPortal("ConfigPortalAP"))
    {
        if (m_Debug)
        {
            m_Debug->println(F("connected...yeey :)"));
            m_Debug->print(F("SSID:"));
            m_Debug->println(WiFi.SSID());
            m_Debug->print(F("PASS:"));
            m_Debug->println(WiFi.psk());
        }

        m_Prefs->begin("network");
        m_Prefs->putString("n_ssid",          WiFi.SSID().c_str());
        m_Prefs->putString("n_pass",          WiFi.psk().c_str());
        m_Prefs->putInt("gmt_offset",         String(wmp_gmtOffset.getValue()).toInt());
        m_Prefs->putInt("dst_offset",         String(wmp_dstOffset.getValue()).toInt());
        m_Prefs->putString("posix_tz",        wmp_posixTZ.getValue());
        m_Prefs->putString("owm_key",         wmp_owm_key.getValue());
        m_Prefs->putString("accu_key",        wmp_accu_key.getValue());
        m_Prefs->putString("accu_loc",        wmp_accu_loc.getValue());
        m_Prefs->putString("tomorrow_io_key", wmp_tomorrow_io_key.getValue());
        m_Prefs->putString("latitude",        wmp_lat.getValue());
        m_Prefs->putString("longitude",       wmp_long.getValue());
        m_Prefs->putString("iana_tz",         wmp_iana_tz.getValue());
        m_Prefs->end();

        loadSettings();
    } 
    else 
    {
        if (m_Debug)
        {
            m_Debug->println(F("Not connected"));
        }
    }
}

void NetworkManager::loadSettings() 
{
    m_Prefs->begin("network");
    m_Settings->ssid            = m_Prefs->getString("n_ssid",          m_Settings->ssid);
    m_Settings->pass            = m_Prefs->getString("n_pass",          m_Settings->pass);
    m_Settings->gmt_offset      = m_Prefs->getInt("gmt_offset",         m_Settings->gmt_offset);
    m_Settings->dst_offset      = m_Prefs->getInt("dst_offset",         m_Settings->dst_offset);
    m_Settings->posix_tz        = m_Prefs->getString("posix_tz",        m_Settings->posix_tz);
    m_Settings->owm_key         = m_Prefs->getString("owm_key",         m_Settings->owm_key);
    m_Settings->accu_key        = m_Prefs->getString("accu_key",        m_Settings->accu_key);
    m_Settings->accu_loc        = m_Prefs->getString("accu_loc",        m_Settings->accu_loc);
    m_Settings->tomorrow_io_key = m_Prefs->getString("tomorrow_io_key", m_Settings->tomorrow_io_key);
    m_Settings->latitude        = m_Prefs->getString("latitude",        m_Settings->latitude);
    m_Settings->longitude       = m_Prefs->getString("longitude",       m_Settings->longitude);
    m_Settings->iana_tz         = m_Prefs->getString("iana_tz",         m_Settings->iana_tz);
    m_Prefs->end();
    if (m_Debug)
    {
        m_Debug->print("Loaded SSID:");
        m_Debug->println(m_Settings->ssid);
        m_Debug->print("Loaded PASS:");
        m_Debug->println(m_Settings->pass);
    }
}

void NetworkManager::reset()
{
    m_Prefs->begin("network");
    m_Prefs->clear();
    m_Prefs->end();
    WiFi.disconnect(true, true);
}






