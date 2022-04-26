/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include "wifi.h"
#include "api.h"
#include "uart.h"

#define SERIAL_BUF_SIZE 255
char URLBuffer[SERIAL_BUF_SIZE];
int UrlIndex = 0;

APIRx ReceiveData;
APITx TransmitData;

void setup() {
	Serial.begin(9600);
	Serial.println("Starting setup...");

	URLBuffer[0] = '\0'; // Initalize buffer with empty string
	init_uart(Serial);
}
char* getDataStart(char* buf, int length){
	for(int i = 0; i < length-4; i++){
		if(*(buf+i) == 'S' && *(buf+i+1) == 'T' && *(buf+i+2) == 'R' && *(buf+i+3) == 'T'){
			return buf+i+4;
		}
	}
	return 0;
}
void loop() {
	// Wifi init
	// Wait 5 seconds after setup before wifi init
	if(!wifi_is_init() && millis() > 5000){
		wifi_init();
	}
	// wait for WiFi connection
	if (wifi_connected()) {
		if(millis() %10000 == 0){
			char* json_data = TransmitData.toJSON();
			String response = wifi_post_json(json_data);
			ReceiveData.readFromJSON(response);
			
			ReceiveData.print();

			ReceiveData.toBuffer(OutputBuffer, &BufferLength);
			uart_tx(true);

			
			
		}
	}
	else{
		if(millis() % 10000 == 0){ // Re-attempt connections every 10 seconds
			if(wifi_attempt_connection()){
				Serial.println("WIFI CONNECTED");
			}
			else{
				Serial.println("WIFI NOT CONNECTED");
			}
		}
	}
	
	// UART communication
	if(millis() % 1000 == 0){
		// ReceiveData.toBuffer(OutputBuffer, &BufferLength);
		// uart_tx(true);
	}

	// URL configuration
	if (Serial.available() > 0) {
		// read the incoming byte:
		char incomingByte = Serial.read();
		URLBuffer[UrlIndex] = incomingByte;
		UrlIndex++;
		if(UrlIndex >= SERIAL_BUF_SIZE-1){
			// Reset buffer
			URLBuffer[0] = '\0';
			UrlIndex = 0;
			Serial.println("URL max length reached");
		}
		else{
			char* dataStart = getDataStart(URLBuffer, UrlIndex-1);
			if(dataStart == 0){ // If not an mcu command
				if(incomingByte == 27){ //Escape
					URLBuffer[0] = '\0';
					UrlIndex = 0;
				}
				else if(incomingByte == 10){ // Line feed
					URLBuffer[UrlIndex-1] = '\0';
					wifi_set_url(URLBuffer, UrlIndex-2);
					UrlIndex = 0;
					Serial.print("Setting target URL: ");
					Serial.println(URLBuffer);
				}
				else if(incomingByte == 34){ // Enter
					URLBuffer[UrlIndex-1] = '\0';
					UrlIndex = 0;
				}
				else if(incomingByte == 8){ // Backspace
					UrlIndex--;
					URLBuffer[UrlIndex] = '\0';
					UrlIndex--;
					Serial.println(URLBuffer);
				}
				else{
					URLBuffer[UrlIndex] = '\0';
					Serial.println(URLBuffer);
				}
			}
			else{ // If an mcu command
				URLBuffer[UrlIndex] = '\0';
				Serial.println(URLBuffer);
				if(UrlIndex > 4 && URLBuffer[UrlIndex-4] == 'S' && URLBuffer[UrlIndex-3] == 'T' && URLBuffer[UrlIndex-2] == 'O' && URLBuffer[UrlIndex-1] == 'P'){
					TransmitData.fromBuf(dataStart, URLBuffer + UrlIndex - 5);
					UrlIndex = 0; // Reset buffer
				}
			}
		
			// Check for MCU communication
			
		}
	}
}