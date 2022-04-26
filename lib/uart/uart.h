#include <Arduino.h>

#define MAX_OUTPUT_BUF_SIZE 256

extern byte OutputBuffer[MAX_OUTPUT_BUF_SIZE];
extern unsigned int BufferLength; // The length of the newest data in the buffer. 0 indicates no new data

bool init_uart(HardwareSerial serialPort);
bool uart_tx(bool blocking);