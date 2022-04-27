#include "uart.h"

HardwareSerial SerialPort = Serial;
byte OutputBuffer[MAX_OUTPUT_BUF_SIZE];
unsigned int BufferLength = 0;

bool init_uart(HardwareSerial serialPort){
    SerialPort = serialPort;
    return true;
}
void convertBufferToChars(){
    for(unsigned int i = 0; i < BufferLength; i++){
        *((char*)(OutputBuffer+i)) = *((char*)(OutputBuffer+i)) + '0';
    }
}
bool uart_tx(bool blocking){
    if(SerialPort.availableForWrite()){
        Serial.print("STRT");
        // convertBufferToChars(); // Converts numerical values to chars by adding '0'
        SerialPort.write(OutputBuffer, BufferLength);
        Serial.print("STOP");
        Serial.println();
        if(blocking){
            SerialPort.flush(); // Waits for the transmission of outgoing serial data to complete
        }
        return true;
    }
    else{
        return false;
    }
}