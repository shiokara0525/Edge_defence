#pragma once

#include <Arduino.h>
#include<MA.h>

class BALL{
  public:
    BALL(){
    ball_x.setLenth(3);
    ball_y.setLenth(3);
    }
    MA ball_x;
    MA ball_y;
    float ang;
    float far;
    float x_pos;
    float y_pos;
    int flag = 1;
    int getBallposition(){
      float x = ball_x.returnAve();
      float y = ball_y.returnAve();
      x *= 0.05;
      y *= 0.05;
      if(0 < x){
        x_pos = 130 - x;
      }
      else{
        x_pos = -130 - x;
      }
      if(0 < y){
        y_pos = 130 - y;
      }
      else{
        y_pos = -130 - y;
      }
      if(150 < abs(x)){
        x = (x < 0 ? -150 : 150);
      }
      if(150 < abs(y)){
        y = (y < 0 ? -150 : 150);
      }
      ang = degrees(atan2(y,x));
      far = sqrt(x_pos*x_pos + y_pos*y_pos) - 60;
      x_pos = x;
      y_pos = y;
      return flag;
    }
    void print(){
      Serial.print(" ang : ");
      Serial.print(ang);
      Serial.print(" far : ");
      Serial.print(far);
      Serial.print(" x : ");
      Serial.print(x_pos);
      Serial.print(" y : ");
      Serial.print(y_pos);
    }
};