/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include "wifi.h"
#include "api.h"
#include "uart.h"

#define SERIAL_DEBUG false
#define SERIAL_BUF_SIZE 255

#define ENABLE_URL_SETTING	false

#define WIFI_LED D6
#define UART_LED D4

char URLBuffer[SERIAL_BUF_SIZE];
int UrlIndex = 0;

APIRx ReceiveData;
APITx TransmitData;

void setup() {
	Serial.begin(115200);
	Serial.println("Starting setup...");

	URLBuffer[0] = '\0'; // Initalize buffer with empty string
	init_uart(Serial);

	pinMode(WIFI_LED, OUTPUT);
	pinMode(UART_LED, OUTPUT);
}
char* getDataStart(char* buf, int length){
	for(int i = 0; i < length-4; i++){
		if(*(buf+i) == 'S' && *(buf+i+1) == 'T' && *(buf+i+2) == 'R' && *(buf+i+3) == 'T'){
			return buf+i+4;
		}
	}
	return 0;
}
char* getDataEnd(char* buf, int length){
	for(int i = 0; i < length-4; i++){
		if(*(buf+i) == 'S' && *(buf+i+1) == 'T' && *(buf+i+2) == 'O' && *(buf+i+3) == 'P'){
			return buf+i-1;
		}
	}
	return 0;
}
// Realigns the specified addresses in the url buffer to the 4-byte boundary to the right of it
int align_url_buffer(char* startAddr, char* endAddr){
	int shiftAmount = 4 - (((int)startAddr)%4);
	if(shiftAmount == 4){ // Already aligned
		return 0;
	}
	// Starting from the end (to avoid overwriting), shift values to the right
	for(char* addr = endAddr; addr >= startAddr; addr--){
		*(addr+shiftAmount) = *addr;
	}
	return shiftAmount;
}
void loop() {
	// Wifi init
	// Wait 5 seconds after setup before wifi init
	if(!wifi_is_init() && millis() > 5000){
		wifi_init();
	}
	// wait for WiFi connection
	if (wifi_connected()) {
		digitalWrite(WIFI_LED, 1);
		if(millis() % 1000 == 0){
			char* json_data = TransmitData.toJSON();
			String response = wifi_post_json(json_data);
			ReceiveData.readFromJSON(response);

			if(SERIAL_DEBUG) ReceiveData.print();
			
			ReceiveData.toBuffer(OutputBuffer, &BufferLength);
			uart_tx(true);

			
			
		}
	}
	else{
		if(millis() % 1000 == 0){ // Re-attempt connections every 5 seconds
			if(wifi_attempt_connection()){
				if(SERIAL_DEBUG) Serial.println("WIFI CONNECTED");
				digitalWrite(WIFI_LED, 1);
			}
			else{
				if(SERIAL_DEBUG) Serial.println("WIFI NOT CONNECTED");
				digitalWrite(WIFI_LED, 0);
			}
		}
		else{
			digitalWrite(WIFI_LED, 0);
		}
	}

	// URL configuration
	while (Serial.available() > 0) {
		if(!ENABLE_URL_SETTING){
			char incomingByte = Serial.read();
			
			// Serial.println(incomingByte);
			URLBuffer[UrlIndex] = incomingByte;
			UrlIndex++;
			if(UrlIndex >= SERIAL_BUF_SIZE-1){
				// Reset buffer
				URLBuffer[0] = '\0';
				UrlIndex = 0;
				// Serial.println("URL max length reached");
			}
			char* dataStart = getDataStart(URLBuffer, UrlIndex-1); // Checks if STRT is in the buffer at all
			// char* dataEnd = getDataEnd(URLBuffer,)
			URLBuffer[UrlIndex] = '\0';
			// Serial.println(URLBuffer);
			if(dataStart != 0 && UrlIndex > 4 && URLBuffer[UrlIndex-4] == 'S' && URLBuffer[UrlIndex-3] == 'T' && URLBuffer[UrlIndex-2] == 'O' && URLBuffer[UrlIndex-1] == 'P'){
				Serial.println("Packet found!");
				int shiftAmount = align_url_buffer(dataStart, URLBuffer + UrlIndex - 5);
				TransmitData.fromBuf(dataStart+shiftAmount, URLBuffer + UrlIndex - 5 + shiftAmount); // dataStart will be the first char after 'STRT', and URLBuffer + UrlIndex-5 will be the last char before "STOP"
				UrlIndex = 0; // Reset buffer
			}
		}
		else{

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
				char* dataStart = getDataStart(URLBuffer, UrlIndex-1); // Checks if STRT is in the buffer at all
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
						// Serial.println(URLBuffer);
						//http://172.19.120.124/mcu
					}
					else{
						URLBuffer[UrlIndex] = '\0';
						// Serial.println(URLBuffer);
					}
				}
				else{ // If an mcu command
					URLBuffer[UrlIndex] = '\0';
					// Serial.println(URLBuffer);
					if(UrlIndex > 4 && URLBuffer[UrlIndex-4] == 'S' && URLBuffer[UrlIndex-3] == 'T' && URLBuffer[UrlIndex-2] == 'O' && URLBuffer[UrlIndex-1] == 'P'){
						int shiftAmount = align_url_buffer(dataStart, URLBuffer + UrlIndex - 5);
						TransmitData.fromBuf(dataStart+shiftAmount, URLBuffer + UrlIndex - 5 + shiftAmount); // dataStart will be the first char after 'STRT', and URLBuffer + UrlIndex-5 will be the last char before "STOP"
						UrlIndex = 0; // Reset buffer
					}
				}
			}
		}
	}
}