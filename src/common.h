#ifndef COMMON_H_
#define COMMON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "forecast_record.h"
#include "common_functions.h"

// char      *_EXFUN(strptime,     (const char *__restrict,
// 				 const char *__restrict,
// 				 struct tm *__restrict));

char accu_openw_icon_translations[][3] = {
/* 0 */ "nd",
/* 1	Sunny	*/  "01",
/* 2	Mostly Sunny */  "02",
/* 3	Partly Sunny	*/  "02",
/* 4	Intermittent Clouds	*/  "03",
/* 5	Hazy Sunshine	*/  "02",
/* 6	Mostly Cloudy	*/  "03",
/* 7	Cloudy */  "03",
/* 8	Dreary */  "10",
/* 9 */ "nd",
/* 10 */ "nd",
/* 11	Fog */  "50",
/* 12	Showers */  "10",
/* 13	Mostly Cloudy w/ Showers */  "10",
/* 14	Partly Sunny w/ Showers */  "10",
/* 15	T-Storms */  "11",
/* 16	Mostly Cloudy w/ T-Storms */  "11",
/* 17	Partly Sunny w/ T-Storms */  "11",
/* 18	Rain */  "10",
/* 19	Flurries */  "13",
/* 20	Mostly Cloudy w/ Flurries */  "13",
/* 21	Partly Sunny w/ Flurries */  "13",
/* 22	Snow */  "13",
/* 23	Mostly Cloudy w/ Snow */  "13",
/* 24	Ice */  "13",
/* 25	Sleet */  "10",
/* 26	Freezing Rain */  "10",
/* 27 */ "nd",
/* 28 */ "nd",
/* 29	Rain and Snow */  "13",
/* 30	Hot */  "01",
/* 31	Cold */  "01",
/* 32	Windy */  "01",
/* 33	Clear */  "01",
/* 34	Mostly Clear */  "02",
/* 35	Partly Cloudy */  "02",
/* 36	Intermittent Clouds */  "03",
/* 37	Hazy Moonlight */  "01",
/* 38	Mostly Cloudy */  "02",
/* 39	Partly Cloudy w/ Showers */  "10",
/* 40	Mostly Cloudy w/ Showers */  "10",
/* 41	Partly Cloudy w/ T-Storms */  "11",
/* 42	Mostly Cloudy w/ T-Storms */  "11",
/* 43	Mostly Cloudy w/ Flurries */  "13",
/* 44	Mostly Cloudy w/ Snow */  "13"
};

char tomorrow_io_openw_translations[][3][20] = {
  {"1000", "01d", "Clear"},
  {"1001", "03d", "Cloudy"},
  {"1100", "02d", "Mostly Clear"},
  {"1101", "03d", "Partly Cloudy"},
  {"1102", "03d", "Mostly Cloudy"},
  {"2000", "50n", "Fog"},
  {"2100", "50n", "Light Fog"},
  {"3000", "02d", "Light Wind"},
  {"3001", "02d", "Wind"},
  {"3002", "02d", "Strong Wind"},
  {"4000", "10d", "Drizzle"},
  {"4001", "10d", "Rain"},
  {"4200", "10d", "Light Rain"},
  {"4201", "10d", "Heavy Rain"},
  {"5000", "13d", "Snow"},
  {"5001", "13d", "Flurries"},
  {"5100", "13d", "Light Snow"},
  {"5101", "13d", "Heavy Snow"},
  {"6000", "10d", "Freezing Drizzle"},
  {"6001", "10d", "Freezing Rain"},
  {"6200", "10d", "Light Freezing Rain"},
  {"6201", "10d", "Heavy Freezing Rain"},
  {"7000", "13d", "Ice Pellets"},
  {"7101", "13d", "Heavy Ice Pellets"},
  {"7102", "13d", "Light Ice Pellets"},
  {"8000", "11d", "Thunderstorm"}
};

void decodeTomorrowIoInterval(Forecast_record_type * forecast, JsonObject values);
void decodeTomorrowIoPrecipitation(Forecast_record_type * forecast, JsonArray intervals, int position, int count);
int tomorrowIoToUnixTime(const char * tomorrowIo_Time, const char * posixTZ);

String tomorrow_io_weather_decode(const char * code, const int index)
{
  for (int i = 0; i <=25; i++)
  {
    if (strcmp(code, tomorrow_io_openw_translations[i][0]) == 0)
    {
      return tomorrow_io_openw_translations[i][index];
    }
  }

  return "N/A";
}

String tomorrow_io_weather_icon(const char * code)
{
  return tomorrow_io_weather_decode(code, 1);
}

String tomorrow_io_weather_text(const char * code)
{
  return tomorrow_io_weather_decode(code, 2);
}

//#########################################################################################
void Convert_Readings_to_Imperial() {
  WxConditions[0].Pressure = hPa_to_inHg(WxConditions[0].Pressure);
  WxForecast[1].Rainfall   = mm_to_inches(WxForecast[1].Rainfall);
  WxForecast[1].Snowfall   = mm_to_inches(WxForecast[1].Snowfall);
}

void Convert_Readings_to_Russian() { // Only the first 3-hours are used
  WxConditions[0].Pressure = hPa_to_mmHg(WxConditions[0].Pressure);
}

//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool DecodeWeather(WiFiClient& json, String Type) {
  Serial.print(F("\nCreating object... "));

  Serial.print(F("ESP.getFreeHeap() = "));
  Serial.println(ESP.getFreeHeap());
  Serial.print(F("ESP.getMaxAllocHeap() = "));
  Serial.println(ESP.getMaxAllocHeap());
  // allocate the JsonDocument
  DynamicJsonDocument doc(min((unsigned int)(35 * 1024), ESP.getMaxAllocHeap() - 5000));
  //StaticJsonDocument<35 * 1024> doc;
  Serial.print(F("DynamicJsonDocument capacity = "));
  Serial.println(doc.capacity());
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // convert it to a JsonObject
  JsonObject root = doc.as<JsonObject>();
  Serial.println(" Decoding " + Type + " data");
  
  if (Type == "onecall") {
    // All Serial.println statements are for diagnostic purposes and some are not required, remove if not needed with //
    WxConditions[0].High        = -50; // Minimum forecast low
    WxConditions[0].Low         = 50;  // Maximum Forecast High
    WxConditions[0].Timezone   = doc["timezone_offset"]; // "0"
    JsonObject current = doc["current"];
    WxConditions[0].Sunrise     = current["sunrise"];                              Serial.println("SRis: " + String(WxConditions[0].Sunrise));
    WxConditions[0].Sunset      = current["sunset"];                               Serial.println("SSet: " + String(WxConditions[0].Sunset));
    WxConditions[0].Temperature = current["temp"];                                 Serial.println("Temp: " + String(WxConditions[0].Temperature));
    WxConditions[0].Feelslike   = current["feels_like"];                           Serial.println("FLik: " + String(WxConditions[0].Feelslike));
    WxConditions[0].Pressure    = current["pressure"];                             Serial.println("Pres: " + String(WxConditions[0].Pressure));
    WxConditions[0].Humidity    = current["humidity"];                             Serial.println("Humi: " + String(WxConditions[0].Humidity));
    WxConditions[0].DewPoint    = current["dew_point"];                            Serial.println("DPoi: " + String(WxConditions[0].DewPoint));
    WxConditions[0].UVI         = current["uvi"];                                  Serial.println("UVin: " + String(WxConditions[0].UVI));
    WxConditions[0].Cloudcover  = current["clouds"];                               Serial.println("CCov: " + String(WxConditions[0].Cloudcover));
    WxConditions[0].Visibility  = current["visibility"];                           Serial.println("Visi: " + String(WxConditions[0].Visibility));
    WxConditions[0].Windspeed   = current["wind_speed"];                           Serial.println("WSpd: " + String(WxConditions[0].Windspeed));
    WxConditions[0].Winddir     = current["wind_deg"];                             Serial.println("WDir: " + String(WxConditions[0].Winddir));
    JsonObject current_weather  = current["weather"][0];
    String Description = current_weather["description"];                           // "scattered clouds"
    String Icon        = current_weather["icon"];                                  // "01n"
    WxConditions[0].Forecast0   = Description;                                     Serial.println("Fore: " + String(WxConditions[0].Forecast0));
    WxConditions[0].Icon        = Icon;                                            Serial.println("Icon: " + String(WxConditions[0].Icon));
  }
  if (Type == "onecall") { // 3.0
    Serial.print(F("\nReceiving Onecall 3.0 Forecast")); 
    //------------------------------------------------
    JsonArray hourly                  = root["hourly"];
    int readings_3hr = hourly.size() / 3 - 1;
    for (byte r = 0; r < min(readings_3hr, max_readings); r++) {
      int hr = r * 3; // 3-hour intervals
      Serial.println("\nPeriod-" + String(r) + "--------------");
      WxForecast[r].Dt                = hourly[hr]["dt"].as<int>();                          Serial.println("DTim: "+String(WxForecast[r].Dt));
      WxForecast[r].Visibility        = hourly[hr]["clouds"].as<int>(); Serial.println("Visi: "+String(WxForecast[r].Visibility));
      WxForecast[r].Temperature       = hourly[hr]["temp"].as<float>();              Serial.println("Temp: "+String(WxForecast[r].Temperature));
      WxForecast[r].Feelslike         = hourly[hr]["feels_like"].as<float>();              Serial.println("FLik: "+String(WxForecast[r].Feelslike));
      WxForecast[r].Pressure          = hourly[hr]["pressure"].as<float>();                  Serial.println("Pres: "+String(WxForecast[r].Pressure));
      WxForecast[r].Humidity          = hourly[hr]["humidity"].as<float>();                  Serial.println("Humi: "+String(WxForecast[r].Humidity));
      WxForecast[r].Icon              = hourly[hr]["weather"][0]["icon"].as<String>();        Serial.println("Icon: "+String(WxForecast[r].Icon));
      WxForecast[r].Description       = hourly[hr]["weather"][0]["description"].as<String>(); Serial.println("Desc: "+String(WxForecast[r].Description));
      WxForecast[r].Cloudcover        = hourly[hr]["clouds"].as<int>();                      Serial.println("CCov: "+String(WxForecast[r].Cloudcover)); // in % of cloud cover
      WxForecast[r].Windspeed         = hourly[hr]["wind_speed"].as<float>();                Serial.println("WSpd: "+String(WxForecast[r].Windspeed));
      WxForecast[r].Winddir           = hourly[hr]["wind_deg"].as<float>();               Serial.println("WDir: "+String(WxForecast[r].Winddir));
      WxForecast[r].Pop               = hourly[hr]["pop"].as<float>();                       Serial.println("Pop:  "+String(WxForecast[r].Pop));
      WxForecast[r].Period            = "dt_txt";                   Serial.println("Peri: "+String(WxForecast[r].Period));
      if (r < 8) { // Check next 3 x 8 Hours = 1 day
        if (WxForecast[r].Temperature > WxConditions[0].High) WxConditions[0].High = WxForecast[r].High; // Get Highest temperature for next 24Hrs
        if (WxForecast[r].Temperature  < WxConditions[0].Low)  WxConditions[0].Low  = WxForecast[r].Low;  // Get Lowest  temperature for next 24Hrs
      }
    }

    JsonArray daily                  = root["daily"];
    
    size_t daily_readings_count = min(daily.size(), (size_t)max_readings);
    pressure_readings_count = daily_readings_count;
    temperature_readings_count = daily_readings_count * 4;
    humidity_readings_count = daily_readings_count;
    rain_readings_count = daily_readings_count;
    snow_readings_count = daily_readings_count;
    for (byte r = 0; r < daily_readings_count; r++) {
      Serial.println("\nDaily Period-" + String(r) + "--------------");
      if (Units == "I") pressure_readings[r] = daily[r]["pressure"].as<float>() * 0.02953;   else pressure_readings[r] = daily[r]["pressure"].as<float>();
      if (Units == "R") pressure_readings[r] = daily[r]["pressure"].as<float>() * 0.75006;   else pressure_readings[r] = daily[r]["pressure"].as<float>();

      if (Units == "I") rain_readings[r] = daily[r]["rain"].as<float>() * 0.02953;   else rain_readings[r] = daily[r]["rain"].as<float>();
      if (Units == "I") snow_readings[r] = daily[r]["snow"].as<float>() * 0.02953;   else snow_readings[r] = daily[r]["snow"].as<float>();
      humidity_readings[r] = daily[r]["humidity"].as<float>();
      temperature_readings[r * 4 + 0] = daily[r]["temp"]["morn"].as<float>();
      temperature_readings[r * 4 + 1] = daily[r]["temp"]["day"].as<float>();
      temperature_readings[r * 4 + 2] = daily[r]["temp"]["eve"].as<float>();
      temperature_readings[r * 4 + 3] = daily[r]["temp"]["night"].as<float>();

      Serial.println("Pres: "+String(pressure_readings[r]));
      Serial.println("Humi: "+String(humidity_readings[r]));
      Serial.println("Rain: "+String(rain_readings[r]));
      Serial.println("Snow: "+String(snow_readings[r]));
      Serial.println("Tem1: "+String(temperature_readings[r * 4 + 0]));
      Serial.println("Tem2: "+String(temperature_readings[r * 4 + 1]));
      Serial.println("Tem3: "+String(temperature_readings[r * 4 + 2]));
      Serial.println("Tem4: "+String(temperature_readings[r * 4 + 3]));
    }
    //------------------------------------------
    float pressure_trend = WxForecast[0].Pressure - WxForecast[2].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";

    if (Units == "I") Convert_Readings_to_Imperial();
    if (Units == "R") Convert_Readings_to_Russian();
  }
/*
  if (Type == "forecast") {
    //Serial.println(json);
    Serial.print(F("\nReceiving Forecast period - ")); //------------------------------------------------
    JsonArray list                    = root["list"];
    for (byte r = 0; r < max_readings; r++) {
      Serial.println("\nPeriod-" + String(r) + "--------------");
      // WxForecast[r].Dt                = list[r]["dt"].as<int>();                          Serial.println("DTim: "+String(WxForecast[r].Dt));
      // WxForecast[r].Temperature       = list[r]["main"]["temp"].as<float>();              Serial.println("Temp: "+String(WxForecast[r].Temperature));
      // WxForecast[r].Low               = list[r]["main"]["temp_min"].as<float>();          Serial.println("TLow: "+String(WxForecast[r].Low));
      // WxForecast[r].High              = list[r]["main"]["temp_max"].as<float>();          Serial.println("THig: "+String(WxForecast[r].High));
      // WxForecast[r].Pressure          = list[r]["main"]["pressure"].as<float>();          Serial.println("Pres: "+String(WxForecast[r].Pressure));
      // WxForecast[r].Humidity          = list[r]["main"]["humidity"].as<float>();          Serial.println("Humi: "+String(WxForecast[r].Humidity));
      // WxForecast[r].Forecast0         = list[r]["weather"][0]["main"].as<char*>();        Serial.println("For0: "+String(WxForecast[r].Forecast0));
      // WxForecast[r].Forecast1         = list[r]["weather"][1]["main"].as<char*>();        Serial.println("For1: "+String(WxForecast[r].Forecast1));
      // WxForecast[r].Forecast2         = list[r]["weather"][2]["main"].as<char*>();        Serial.println("For2: "+String(WxForecast[r].Forecast2));
      // WxForecast[r].Icon              = list[r]["weather"][0]["icon"].as<char*>();        Serial.println("Icon: "+String(WxForecast[r].Icon));
      // WxForecast[r].Description       = list[r]["weather"][0]["description"].as<char*>(); Serial.println("Desc: "+String(WxForecast[r].Description));
      // WxForecast[r].Cloudcover        = list[r]["clouds"]["all"].as<int>();               Serial.println("CCov: "+String(WxForecast[r].Cloudcover)); // in % of cloud cover
      // WxForecast[r].Windspeed         = list[r]["wind"]["speed"].as<float>();             Serial.println("WSpd: "+String(WxForecast[r].Windspeed));
      // WxForecast[r].Winddir           = list[r]["wind"]["deg"].as<float>();               Serial.println("WDir: "+String(WxForecast[r].Winddir));
      // WxForecast[r].Rainfall          = list[r]["rain"]["3h"].as<float>();                Serial.println("Rain: "+String(WxForecast[r].Rainfall));
      // WxForecast[r].Snowfall          = list[r]["snow"]["3h"].as<float>();                Serial.println("Snow: "+String(WxForecast[r].Snowfall));
      // WxForecast[r].Pop               = list[r]["pop"].as<float>();                       Serial.println("Pop:  "+String(WxForecast[r].Pop));
      // WxForecast[r].Period            = list[r]["dt_txt"].as<char*>();                    Serial.println("Peri: "+String(WxForecast[r].Period));
      if (r < 8) { // Check next 3 x 8 Hours = 1 day
        if (WxForecast[r].High > WxConditions[0].High) WxConditions[0].High = WxForecast[r].High; // Get Highest temperature for next 24Hrs
        if (WxForecast[r].Low  < WxConditions[0].Low)  WxConditions[0].Low  = WxForecast[r].Low;  // Get Lowest  temperature for next 24Hrs
      }
    }
    for (byte r = 0; r < min(8, max_readings); r++) {
      if (WxForecast[r].Temperature > WxConditions[0].High) WxConditions[0].High = WxForecast[r].Temperature;
      if (WxForecast[r].Temperature < WxConditions[0].Low) WxConditions[0].Low = WxForecast[r].Temperature;
    }
    //------------------------------------------
    float pressure_trend = WxForecast[0].Pressure - WxForecast[2].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";

    if (Units == "I") Convert_Readings_to_Imperial();
    if (Units == "R") Convert_Readings_to_Russian();
  }
  */
  /*
  if (Type == "currentconditions") // accuweather
  {
    Serial.println(F("Decoding: currentconditions (accuweather)"));
    JsonArray rootArray = doc.as<JsonArray>();
    JsonObject root = rootArray.getElement(0);
    // All Serial.println statements are for diagnostic purposes and some are not required, remove if not needed with //
    WxConditions[0].Temperature      = root["Temperature"]["Metric"]["Value"];                 Serial.println("Temp: " + String(WxConditions[0].Temperature));
    WxConditions[0].Feelslike        = root["RealFeelTemperature"]["Metric"]["Value"];         Serial.println("FLik: " + String(WxConditions[0].Feelslike)); 
    WxConditions[0].Pressure         = root["Pressure"]["Metric"]["Value"];                    Serial.println("Pres: " + String(WxConditions[0].Pressure));
    WxConditions[0].PressureTendency = root["PressureTendency"]["LocalizedText"].as<char*>();  Serial.println("PTen: " + String(WxConditions[0].PressureTendency));
    WxConditions[0].Humidity         = root["RelativeHumidity"];                               Serial.println("Humi: " + String(WxConditions[0].Humidity));
    WxConditions[0].DewPoint         = root["DewPoint"]["Metric"]["Value"];                    Serial.println("DPoi: " + String(WxConditions[0].DewPoint));
    WxConditions[0].UVI              = root["UVIndex"];                                        Serial.println("UVin: " + String(WxConditions[0].UVI));
    WxConditions[0].UVIText          = root["UVIndexText"].as<char*>();                        Serial.println("UVtx: " + String(WxConditions[0].UVIText));
    WxConditions[0].Cloudcover       = root["CloudCover"];                                     Serial.println("CCov: " + String(WxConditions[0].Cloudcover));
    WxConditions[0].Visibility       = root["Visibility"]["Metric"]["Value"];                  Serial.println("Visi: " + String(WxConditions[0].Visibility));
    float WindSpeed = root["Wind"]["Speed"]["Metric"]["Value"];
    WxConditions[0].Windspeed        = WindSpeed/3.6;                                          Serial.println("WSpd: " + String(WxConditions[0].Windspeed));
    WxConditions[0].Winddir          = root["Wind"]["Direction"]["Degrees"];                   Serial.println("WDir: " + String(WxConditions[0].Winddir));
    String Description               = root["WeatherText"];                                    // "scattered clouds"
    int IconIndex                    = root["WeatherIcon"].as<int>();         
    boolean IsDayTime                = root["IsDayTime"].as<boolean>();
    WxConditions[0].Forecast0        = Description;                                            Serial.println("Fore: " + String(WxConditions[0].Forecast0));
    if (IconIndex > 0 && IconIndex <= 44)
    {
      String Icon = String(accu_openw_icon_translations[IconIndex]) + (IsDayTime ? "d" : "n");
      WxConditions[0].Icon           = Icon;                                                   Serial.println("Icon: " + String(WxConditions[0].Icon));
    }

    WxConditions[0].Visibility = WxConditions[0].Visibility * 1000;
    WxConditions[0].Pressure = hPa_to_mmHg(WxConditions[0].Pressure);
  }
*/


  return true;
}

bool DecodeWeatherTomorrowIo(String json, String Type, const char * posixTZ) {
  Serial.print(F("\nCreating object... "));

  Serial.print(F("ESP.getFreeHeap() = "));
  Serial.println(ESP.getFreeHeap());
  Serial.print(F("ESP.getMaxAllocHeap() = "));
  Serial.println(ESP.getMaxAllocHeap());
  // allocate the JsonDocument
  DynamicJsonDocument doc(min((unsigned int)(35 * 1024), ESP.getMaxAllocHeap() - 5000));
  //StaticJsonDocument<35 * 1024> doc;
  Serial.print(F("DynamicJsonDocument capacity = "));
  Serial.println(doc.capacity());
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // convert it to a JsonObject
  JsonObject root = doc.as<JsonObject>();
  Serial.println(" Decoding " + Type + " data");

  if (Type == "current,1h") // tomorrowIo
  {
    Serial.println(F("Decoding: tomorrowIo current,1h"));

    JsonArray timelines = root["data"]["timelines"];
    
    String step0 = timelines[0]["timestep"].as<String>();

    JsonObject currentStep = step0 == "1h" ? timelines[1] : timelines[0]; // FIXME iterate over timelines and check whether it's current or forecast
    JsonObject forecastStep = step0 == "1h" ? timelines[0] : timelines[1];
    JsonArray currentIntervals = currentStep["intervals"];
    JsonArray forecastIntervals = forecastStep["intervals"];

    WxConditions[0].High        = -50; // Minimum forecast low
    WxConditions[0].Low         = 50;  // Maximum Forecast High
    decodeTomorrowIoInterval(&(WxConditions[0]), currentIntervals[0]);

    size_t daily_readings_count = max_readings;
    pressure_readings_count = daily_readings_count;
    temperature_readings_count = daily_readings_count;
    humidity_readings_count = daily_readings_count;
    rain_readings_count = daily_readings_count;
    snow_readings_count = daily_readings_count;

    for (byte r = 0; r < max_readings; r++) 
    {
      Serial.println("--------------");
      decodeTomorrowIoInterval(&(WxForecast[r]), forecastIntervals[r * 3 + 2]);
      decodeTomorrowIoPrecipitation(&(WxForecast[r]), forecastIntervals, r * 3, 3);

      if (r < 8) { // Check next 3 x 8 Hours = 1 day
        if (WxForecast[r].High > WxConditions[0].High) WxConditions[0].High = WxForecast[r].High; // Get Highest temperature for next 24Hrs
        if (WxForecast[r].Low  < WxConditions[0].Low)  WxConditions[0].Low  = WxForecast[r].Low;  // Get Lowest  temperature for next 24Hrs
      }

      if (Units == "I") pressure_readings[r] = WxForecast[r].Pressure * 0.02953;   else pressure_readings[r] = WxForecast[r].Pressure;
      if (Units == "R") pressure_readings[r] = WxForecast[r].Pressure * 0.75006;   else pressure_readings[r] = WxForecast[r].Pressure;

      rain_readings[r]        = WxForecast[r].Rainfall;
      snow_readings[r]        = WxForecast[r].Snowfall; 
      
      humidity_readings[r]    = WxForecast[r].Humidity;
      temperature_readings[r] = WxForecast[r].Temperature;
    }

    //------------------------------------------
    float pressure_trend = WxForecast[0].Pressure - WxForecast[2].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";

    if (Units == "I") Convert_Readings_to_Imperial();
    if (Units == "R") Convert_Readings_to_Russian();
  }


  if (Type == "1d") // tomorrowIo
  {
    Serial.println(F("Decoding: tomorrowIo 1d"));

    JsonArray timelines = root["data"]["timelines"];
    JsonObject currentStep = timelines[0];
    JsonArray currentIntervals = currentStep["intervals"];
    JsonObject current = currentIntervals[0];
  
    WxConditions[0].Sunrise     = tomorrowIoToUnixTime(current["values"]["sunriseTime"].as<String>().c_str(), posixTZ); Serial.println("SRis: "+String(WxConditions[0].Sunrise));
    WxConditions[0].Sunset      = tomorrowIoToUnixTime(current["values"]["sunsetTime"].as<String>().c_str(), posixTZ);  Serial.println("SSet: "+String(WxConditions[0].Sunset));
  }

  return true;
}

void decodeTomorrowIoInterval(Forecast_record_type * forecast, JsonObject interval, const char * posixTZ)
{
    forecast->Period           = interval["time"].as<String>();                     Serial.println("Peri: " + String(forecast->Period));
    forecast->Temperature      = interval["values"]["temperature"];                 Serial.println("Temp: " + String(forecast->Temperature));
    forecast->Feelslike        = interval["values"]["temperatureApparent"];         Serial.println("FLik: " + String(forecast->Feelslike)); 
    forecast->Pressure         = interval["values"]["pressureSurfaceLevel"];        Serial.println("Pres: " + String(forecast->Pressure));
    forecast->Humidity         = interval["values"]["humidity"];                    Serial.println("Humi: " + String(forecast->Humidity));
    forecast->DewPoint         = interval["values"]["dewPoint"];                    Serial.println("DPoi: " + String(forecast->DewPoint));
    forecast->UVI              = interval["values"]["uvIndex"];                     Serial.println("UVin: " + String(forecast->UVI));
    forecast->Cloudcover       = interval["values"]["cloudCover"];                  Serial.println("CCov: " + String(forecast->Cloudcover));
    forecast->Visibility       = interval["values"]["visibility"];                  Serial.println("Visi: " + String(forecast->Visibility));
    
    forecast->Visibility = forecast->Visibility * 1000;
    
    forecast->Windspeed        = interval["values"]["windSpeed"];                   Serial.println("WSpd: " + String(forecast->Windspeed));
    forecast->Winddir          = interval["values"]["windDirection"];               Serial.println("WDir: " + String(forecast->Winddir));
    int WeatherCode            = interval["values"]["weatherCode"];                 Serial.println("WCod: " + String(WeatherCode));
    if (WeatherCode > 0)
    {
      forecast->Forecast0 = tomorrow_io_weather_text(String(WeatherCode).c_str());  Serial.println("Fore: " + String(forecast->Forecast0));
      forecast->Icon      = tomorrow_io_weather_icon(String(WeatherCode).c_str());  Serial.println("Icon: " + String(forecast->Icon));
    }
    forecast->High = forecast->Low = forecast->Temperature;
    forecast->Dt = tomorrowIoToUnixTime(forecast->Period.c_str(), posixTZ);                 Serial.println("Dt  : " + String(forecast->Dt));
}


void decodeTomorrowIoPrecipitation(Forecast_record_type * forecast, JsonArray intervals, int position, int count)
{
    for (int i = 0; i < count; i++)
    {
      JsonObject interval = intervals[position + i];
      
        // precipitationType:
        // 0: N/A
        // 1: Rain
        // 2: Snow
        // 3: Freezing Rain
        // 4: Ice Pellets
      
      // Serial.println(interval["values"].as<String>());
      int precipitationType = interval["values"]["precipitationType"].as<float>();
      float precipitationIntensity = interval["values"]["precipitationIntensity"].as<float>();
      if (precipitationType == 2 || precipitationType == 4)
      {
        forecast->Snowfall += precipitationIntensity;                                 Serial.println("Snow: " + String(precipitationIntensity));
      }
      else
      {
        forecast->Rainfall += precipitationIntensity;                                 Serial.println("Rain: " + String(precipitationIntensity));
      }
    }
    Serial.println("Snow : " + String(forecast->Snowfall));
    Serial.println("Rain : " + String(forecast->Rainfall));
}


int tomorrowIoToUnixTime(const char * tomorrowIoTime, const char * posixTZ)
{
    struct tm tm;
    time_t ts = 0;
    memset(&tm, 0, sizeof(tm));
    strptime(tomorrowIoTime, "%Y-%m-%dT%H:%M:%S+%z:00", &tm);
    tm.tm_isdst = -1;
    Serial.printf(
        "tomorrowIoToUnixTime: [%s] => hour=%d, min=%d, dst=%d\n", 
        tomorrowIoTime, tm.tm_hour, tm.tm_min, tm.tm_isdst);
    return mktime(&tm);
}


//#########################################################################################
String ConvertUnixTime(int unix_time) {
  // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
  
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  if (Units == "M" || Units == "R") {
    strftime(output, sizeof(output), "%H:%M %d/%m/%y", now_tm);
  }
  else {
    strftime(output, sizeof(output), "%I:%M%P %m/%d/%y", now_tm);
  }
  return output;
}
//#########################################################################################
//WiFiClient client; // wifi client object

bool obtain_wx_data_accuweather(WiFiClient& client, const String& RequestType, 
      const String& accuLocation, const String& accuKey) 
{
  client.stop(); // close connection before sending a new request
  HTTPClient http;

  String uri = String("/") + RequestType + "/v1/" + accuLocation + "?apikey=" + accuKey + "&details=true";

  http.begin(client, accuServer, 80, uri);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    if (!DecodeWeather(http.getStream(), RequestType)) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}

bool obtain_wx_data_tomorrowIo(WiFiClientSecure& client, const String& RequestType, 
    tm * p_current_time, int hours_to_fetch, 
    const String& tomorrowIoKey, const String& latitude, const String& longitude, const String& tomorrowIoTimezone, const char * posixTZ)
{
  client.stop(); // close connection before sending a new request
  HTTPClient http;

  time_t rawtime = mktime(p_current_time);
  struct tm * endTime = localtime(&rawtime);
  endTime->tm_hour = endTime->tm_hour + hours_to_fetch;
  mktime(endTime);

// "2019-03-20T14:09:50Z"
  char endTimeStr[21];
  strftime(endTimeStr, sizeof(endTimeStr), "%Y-%m-%dT%H:%M:%SZ", endTime);

  String uri = String("/v4/timelines?apikey=") + tomorrowIoKey + 
    "&location=" + latitude + "," + longitude + 
    "&fields=" + (RequestType == "1d" ? tomorrowIoFieldsAstro : tomorrowIoFieldsWeather) + 
    "&endTime=" + endTimeStr +
    "&timesteps=" + RequestType +
    "&units=metric&timezone=" + tomorrowIoTimezone;

  Serial.println(uri);

  client.setInsecure();

  //http.begin(client, String(tomorrowIo_Server) + uri);
  
  http.begin(client, tomorrowIoServer, 443, uri, true);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    String json = http.getString();
    // Serial.print(json);
    if (!DecodeWeatherTomorrowIo(json, RequestType, posixTZ)) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error:  %d - %s", httpCode, http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}


bool obtain_wx_data_owm(WiFiClient& client, const String& RequestType, 
    const String& Latitude, const String& Longitude, const String apiKey) {
  const String units = (Units == "I" ? "imperial" : "metric");
  client.stop(); // close connection before sending a new request
  HTTPClient http;
  //api.openweathermap.org/data/3.0/RequestType?lat={lat}&lon={lon}&appid={API key}
  //String uri = "/data/3.0/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units=" + units + "&lang=" + Language;
  String uri = "/data/3.0/" + RequestType + "?lat=" + Latitude + "&lon=" + Longitude + "&appid=" + apiKey + "&mode=json&units=" + units + "&lang=" + Language;
  if (RequestType == "onecall") uri += "&exclude=minutely,alerts";
  if(RequestType != "weather")
  {
    uri += "&cnt=" + String(max_readings);
  }
  //http.begin(uri,test_root_ca); //HTTPS example connection
  http.begin(client, server, 80, uri);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    if (!DecodeWeather(http.getStream(), RequestType)) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}
#endif /* ifndef COMMON_H_ */
