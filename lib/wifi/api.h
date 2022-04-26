#include <Arduino.h>

typedef enum {
    STRAIGHT = 0,
    LEFT = 1,
    RIGHT = 2
} SteerDir;


class APITx{
    public:
    APITx();
    char* toJSON();
    bool fromBuf(char* startBuf, char* endBuf);
    // Reading data
    float speed = 0;
    float voltage = 0;
    float acc_x = 0;
    float acc_y = 0;
    float acc_z = 0;
    float obstacle_dist = 0;
    float x_rot = 0;
    float y_rot = 0;
    float z_rot = 0;

    // Control data
    bool emergency_stop = false;
    SteerDir steer_direction = STRAIGHT;
    float power_pwm = 0;
    float steer_pwm = 0;
    float drive_pwm = 0;

    // Misc/non-physical data
    int timestamp = 0;
    bool is_default = true; // Indicates if this class object has been modified or just contains default values
};

class APIRx{
    public:
    bool readFromJSON(String json);
    bool toBuffer(byte* buffer, unsigned int* length);
    void print();
    float speed = 0;
    
    bool autonomous_steer = false;
    float steer_direction = STRAIGHT;
    int timestamp = 0;

    float stop_distance = 0;
    float stop_accel = 0;

    bool is_default = true; // Indicates if this class object has been modified or just contains default values
};