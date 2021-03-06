#include "api.h"
#include <stdio.h>
#include <ArduinoJson.h>

StaticJsonDocument<500> JsonParser;

const char PAYLOAD_STRUCTURE[] = "{"
    // Readings
    "\"speed\": %.02f,"
    "\"voltage\": %.02f,"
    "\"acc_x\": %.02f,"
    "\"acc_y\": %.02f,"
    "\"acc_z\": %.02f,"
    "\"obstacle_dist\": %.02f,"
    "\"x_rot\": %.02f,"
    "\"y_rot\": %.02f,"
    "\"z_rot\": %.02f,"

    // Control data
    "\"emergency_stop\": %d,"
    "\"steer_direction\": %d,"
    "\"power_pwm\": %.02f,"
    "\"steer_pwm\": %.02f,"
    "\"drive_pwm\": %.02f,"

    // Misc/non-physical data
    "\"timestamp\": %d,"
    "\"is_default\": %d"
    "}"; // Indicates if this class member has been modified or just contains default values

char Payload[500];

APITx::APITx(){
}
char* APITx::toJSON(){
    sprintf(Payload, PAYLOAD_STRUCTURE, speed, voltage, acc_x, acc_y, acc_z,
                                        obstacle_dist, x_rot, y_rot, z_rot,
                                        emergency_stop, steer_direction,
                                        power_pwm, steer_pwm, drive_pwm,
                                        timestamp, is_default );
    return Payload;
}
bool APITx::fromBuf(char* startBuf, char* endBuf){
    for(char* a = startBuf; a <= endBuf; a++){
        Serial.println(*a);
        // *a = *a - '0';
    }
    Serial.println("ATTEMPTING FROM BUF");
    if( (endBuf - startBuf) + 1 < 4*11){ // We require 11 floats. floats are 4 bytes in arduino
        Serial.println(endBuf - startBuf);
        return false;
    }

    timestamp = millis();
    is_default = false;

    char* currAddr = startBuf;
    speed = *((float*)currAddr);
    currAddr += sizeof(float);

    voltage = *((float*)currAddr);
    currAddr += sizeof(float);

    acc_x = *((float*)currAddr);
    currAddr += sizeof(float);

    acc_y = *((float*)currAddr);
    currAddr += sizeof(float);

    acc_z = *((float*)currAddr);
    currAddr += sizeof(float);

    obstacle_dist = *((float*)currAddr);
    currAddr += sizeof(float);

    emergency_stop = *((float*)currAddr);
    currAddr += sizeof(float);

    steer_direction = (SteerDir) (int) *((float*)currAddr);
    currAddr += sizeof(float);

    power_pwm = *((float*)currAddr);
    currAddr += sizeof(float);

    steer_pwm = *((float*)currAddr);
    currAddr += sizeof(float);

    drive_pwm = *((float*)currAddr);
    currAddr += sizeof(float);
    
    return true;
}
bool APIRx::readFromJSON(String json){ 
    Serial.println(json);
    DeserializationError err = deserializeJson(JsonParser, json);
    if(err){
        Serial.print("JSON parsing failed: \"");
        Serial.println(err.f_str());
        return false;
    }
    speed = JsonParser["speed"].as<float>();
    autonomous_steer = JsonParser["autonomous_steer"].as<int>();
    steer_direction = JsonParser["steer_direction"].as<float>();
    stop_distance = JsonParser["stop_distance"].as<float>();
    stop_accel = JsonParser["stop_accel"].as<float>();
    is_default = JsonParser["is_default"].as<int>();
    return true;
}
/*
    bool autonomous_steer = false;
    int steer_direction = STRAIGHT;
    int timestamp = 0;

    float stop_distance = 0;
    float stop_accel = 0;*/
bool APIRx::toBuffer(byte* buffer, unsigned int* length){
    byte* currAddr = buffer; // Start at first address

    *((float*)currAddr) = speed;
    currAddr += sizeof(float);

    // Cast the bool to an integer so that we are always writing to 32-bit aligned addresses
    // Otherwise MCU will crash
    *((int*)currAddr) = autonomous_steer;
    currAddr += sizeof(int);

    *((float*)currAddr) = steer_direction;
    currAddr += sizeof(float);

    *((float*)currAddr) = stop_distance;
    currAddr += sizeof(float);

    *((float*)currAddr) = stop_accel;
    currAddr += sizeof(float);

    *length = sizeof(float)*3 + sizeof(bool) + sizeof(int);
    return true;
}
void APIRx::print(){
    Serial.print("Speed: ");
    Serial.println(speed);
    Serial.print("Autonomouse steer: ");
    Serial.println(autonomous_steer);
    Serial.print("Steer direction: ");
    Serial.println(steer_direction);
    Serial.print("Stop distance: ");
    Serial.println(stop_distance);
    Serial.print("Stop accel: ");
    Serial.println(stop_accel);
}