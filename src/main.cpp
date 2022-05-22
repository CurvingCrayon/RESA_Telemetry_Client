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

#define WIFI_LED D6 // LED	used for wifi indication
#define UART_LED D4 // LED	used for UART indication

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
// Checks for start of a UART packet within a given buffer. Returns 0 if unfound.
char* getDataStart(char* buf, int length){
	for(int i = 0; i < length-4; i++){
		if(*(buf+i) == 'S' && *(buf+i+1) == 'T' && *(buf+i+2) == 'R' && *(buf+i+3) == 'T'){
			return buf+i+4;
		}
	}
	return 0;
}
// Checks for end of a UART	packet within a given buffer. Returns 0 if unfound.
char* getDataEnd(char* buf, int length){
	for(int i = 0; i < length-4; i++){
		if(*(buf+i) == 'S' && *(buf+i+1) == 'T' && *(buf+i+2) == 'O' && *(buf+i+3) == 'P'){
			return buf+i-1;
		}
	}
	return 0;
}
// Realigns the specified addresses in the url buffer to the 4-byte boundary to the right of it
// This is necessary when dereferencing memory, as required by microcontrollers.
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
		digitalWrite(WIFI_LED, 1); // LED indication
		if(millis() % 1000 == 0){ // Every second
			// Wifi transmit
			char* json_data = TransmitData.toJSON();      // Convert data class to a json object as a string
			String response = wifi_post_json(json_data);  // Send POST request over wifi, returning a JSON response from the server
			ReceiveData.readFromJSON(response);           // Converte json object into a class member

			if(SERIAL_DEBUG) ReceiveData.print();         // Print received data contents over UART (for debugging only)
			
			ReceiveData.toBuffer(OutputBuffer, &BufferLength); // Convert class member into a UART packet
			uart_tx(true);									   // Transmit packet over UART
		}
	}
	else{
		if(millis() % 1000 == 0){ // Re-attempt connections every 1 second
			// WiFi LED	indication
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

	// URL configuration / packet receiving
	// URL configuration allows one to set the URL of the telemetry server using a serial terminal
	while (Serial.available() > 0) { // If UART	byte received
		char incomingByte = Serial.read();
		URLBuffer[UrlIndex] = incomingByte; // Add byte to internal buffer
		UrlIndex++;
		if(UrlIndex >= SERIAL_BUF_SIZE-1){ // If buffer size exceeded
			// Reset buffer
			URLBuffer[0] = '\0';
			UrlIndex = 0;
		}
		char* dataStart = getDataStart(URLBuffer, UrlIndex-1); // Checks if STRT is in the buffer at all
		URLBuffer[UrlIndex] = '\0'; // Terminate string in case printed over UART
		// If the start and end of a packet is detected
		if(dataStart != 0 && UrlIndex > 4 && URLBuffer[UrlIndex-4] == 'S' && URLBuffer[UrlIndex-3] == 'T' && URLBuffer[UrlIndex-2] == 'O' && URLBuffer[UrlIndex-1] == 'P'){
			int shiftAmount = align_url_buffer(dataStart, URLBuffer + UrlIndex - 5);
			TransmitData.fromBuf(dataStart+shiftAmount, URLBuffer + UrlIndex - 5 + shiftAmount); // dataStart will be the first char after 'STRT', and URLBuffer + UrlIndex-5 will be the last char before "STOP"
			UrlIndex = 0; // Reset buffer
		}
	}
}