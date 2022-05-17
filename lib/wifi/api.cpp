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

    // Debug messages
    "\"uno_debug_msg\": \"%s\","

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
                                        uno_debug_msg,
                                        timestamp, is_default );
    return Payload;
}
bool APITx::fromBuf(char* startBuf, char* endBuf){
    // Serial.println((int)(endBuf - startBuf));
    if( (endBuf - startBuf) + 1 < 4*15){ // We require 11 floats. floats are 4 bytes in arduino
        Serial.println("Packet too small. Rejected.");
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

    x_rot = *((float*)currAddr);
    currAddr += sizeof(float);

    y_rot = *((float*)currAddr);
    currAddr += sizeof(float);

    z_rot = *((float*)currAddr);
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

    uno_msg_size = *((float*)currAddr);
    currAddr += sizeof(float);


    for(int i = 0; i < uno_msg_size; i++){
        if(currAddr > endBuf){
            return true;
        }
        uno_debug_msg[i] = *((char*)currAddr);
        currAddr += sizeof(char);
        
    }
    uno_debug_msg[uno_msg_size] = '\0';
    currAddr += sizeof(char);

    return true;
}
bool APIRx::readFromJSON(String json){ 
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
    emergency_stop = JsonParser["emergency_stop"].as<int>();
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
    *((float*)currAddr) = (float)autonomous_steer;
    currAddr += sizeof(float);

    *((float*)currAddr) = steer_direction;
    currAddr += sizeof(float);

    *((float*)currAddr) = stop_distance;
    currAddr += sizeof(float);

    *((float*)currAddr) = stop_accel;
    currAddr += sizeof(float);

    *((float*)currAddr) = (float)emergency_stop;
    currAddr += sizeof(float);

    *length = (int)(currAddr - buffer);
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