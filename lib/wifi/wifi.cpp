#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "wifi.h"

#define MAX_URL_LENGTH 255
#define URL_FLASH_ADDR 0

ESP8266WiFiMulti WiFiMulti;

bool IsInitialized = false;
bool IsConnected = false;

String WifiURL = "";

void wifi_init(){
	if(!IsInitialized){
		WiFi.mode(WIFI_STA);
  		WiFiMulti.addAP("UTS-DeviceNet");//, "PASSWORD");
		WiFiMulti.addAP("Telstra58A511_2GEXT","6q4cenave8");
		WiFiMulti.addAP("shrek", "aaaaaaaa");
		WiFiMulti.addAP("octopus", "shrekthethird");

		// Load saved URL from EEPROM
		EEPROM.begin(MAX_URL_LENGTH);
		for(int i = 0; i < MAX_URL_LENGTH; i++){
			char c = EEPROM.read(URL_FLASH_ADDR+i);
			Serial.print("EEPROM read: ");
			Serial.println(c);
			if(c == '\0'){
				break;
			}
			WifiURL += c;
		}
		Serial.print("URL loaded from flash: ");
		Serial.println(WifiURL);
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

void wifi_get(){
	WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin at url ");
	Serial.println(WifiURL);
	String url(WifiURL);
    //if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // HTTP
    if (http.begin(client, "http://192.168.43.10/mcu")) {  // HTTP


		Serial.print("[HTTP] GET...\n");
		// start connection and send HTTP header
		int httpCode = http.GET();

		// httpCode will be negative on error
		if (httpCode > 0) {
			// HTTP header has been send and Server response header has been handled
			Serial.printf("[HTTP] GET... code: %d\n", httpCode);

			// file found at server
			if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
			String payload = http.getString();
			Serial.println(payload);
		}
		} else {
			Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
}
String wifi_post_json(char* json_body){
	WiFiClient client;

    HTTPClient http;

    //if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // HTTP

    if (http.begin(client, WifiURL)) {  // HTTP


		Serial.print("[HTTP] POST to ");
		Serial.println(WifiURL);
		// start connection and send HTTP header
		http.addHeader("Content-Type","application/json");
		int httpCode = http.POST(json_body);

		// httpCode will be negative on error
		if (httpCode > 0) {
			// HTTP header has been send and Server response header has been handled
			Serial.printf("[HTTP] POST... code: %d\n", httpCode);

			// file found at server
			if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
			String payload = http.getString();
			return payload;
		}
		} else {
			Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}
      	http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
	return "";
}
void wifi_set_url(char* url, int length){
	// Copy from main input buffer to WifiURL, to be used by get and post functions above
	WifiURL = "";
	for(int i = 0; i < length; i++){
		WifiURL += *(url+i); // Copy from input buffer to URL
		EEPROM.write(URL_FLASH_ADDR+i, *(url+i)); // Write byte to EEPROM, but only if its different to existing value
	}
	EEPROM.write(URL_FLASH_ADDR+length, '\0'); // Save terminating value to EEPROM also
	EEPROM.commit(); // Commit changes to EEPROM
}