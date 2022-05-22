#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "wifi.h"

#define MAX_URL_LENGTH 255
#define URL_FLASH_ADDR 0

#define SERIAL_DEBUG false

ESP8266WiFiMulti WiFiMulti;

bool IsInitialized = false;
bool IsConnected = false;

String WifiURL = "http://172.19.121.241/mcu\0"; // URL of telemetry server

void wifi_init(){
	if(!IsInitialized){
		WiFi.mode(WIFI_STA);
  		WiFiMulti.addAP("UTS-DeviceNet");//, "PASSWORD");
		WiFiMulti.addAP("Telstra58A511_2GEXT","6q4cenave8");
		WiFiMulti.addAP("shrek", "aaaaaaaa");
		WiFiMulti.addAP("octopus", "shrekthethird");

		IsInitialized = true;
	}
}
bool wifi_is_init(){
	return IsInitialized;
}
bool wifi_connected(){
  	return IsConnected;
}
bool wifi_attempt_connection(){
	bool result = WiFiMulti.run() == WL_CONNECTED;
	IsConnected = result;
	return result;
}
String wifi_post_json(char* json_body){
	WiFiClient client;
    HTTPClient http;
    if (http.begin(client, WifiURL)) {  // HTTP
		if(SERIAL_DEBUG) Serial.print("[HTTP] POST to ");
		if(SERIAL_DEBUG) Serial.println(WifiURL);
		// start connection and send HTTP header
		http.addHeader("Content-Type","application/json");
		int httpCode = http.POST(json_body);

		// httpCode will be negative on error
		if (httpCode > 0) {
			// HTTP header has been send and Server response header has been handled
			if(SERIAL_DEBUG) Serial.printf("[HTTP] POST... code: %d\n", httpCode);

			// file found at server
			if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
			String payload = http.getString();
			return payload;
		}
		} else {
			if(SERIAL_DEBUG) Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}
      	http.end();
    } else {
      if(SERIAL_DEBUG) Serial.printf("[HTTP} Unable to connect\n");
    }
	return "";
}