#pragma once

#include <Arduino.h>
#include<MA.h>

class BALL{
    public:
    MA ball_x;
    MA ball_y;
    MA far__;
    float ang;
    float far;
    float far_old = 0;
    float x_pos;
    float y_pos;
    int flag = 1;
    float dx;
    float far_;
    int ball_get;
    BALL();
    int getBallposition();
    void print();
    void begin();
};