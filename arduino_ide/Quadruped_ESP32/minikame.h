#ifndef minikame_h
#define minikame_h

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Octosnake.h"

class MiniKame{
public:
    void init();
    void run(float steps, int period);
    void walk(float steps, int period);
    void omniWalk(float steps, int T, bool side, float turn_factor);
    //void backward(float steps, int period);
    void turnL(float steps, int period);
    void turnR(float steps, int period);
    void moonwalkL(float steps, int period);
    void dance(float steps, int period);
    void upDown(float steps, int period);
    void pushUp(float steps, int period);
    void hello();
    void jump();
    void home();
    void zero();
    void frontBack(float steps, int period);

    void testLeg(int leg_id);

    void setServo(int id, float target);
    void reverseServo(int id);
    float getServo(int id);
    void moveServos(int time, float target[8]);

private:
    Adafruit_PWMServoDriver pwm; // Đối tượng điều khiển PCA9685
    Oscillator oscillator[8]; // QUAN TRỌNG: Khai báo lại Oscillator
    
    int trim[8];
    bool reverse[8];
    unsigned long _init_time;
    unsigned long _final_time;
    unsigned long _partial_time;
    float _increment[8];
    float _servo_position[8];

    void execute(float steps, int period[8], int amplitude[8], int offset[8], int phase[8]);
};

#endif
