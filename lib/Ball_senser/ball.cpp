#include<ball.h>

BALL::BALL(){
    ball_x.setLenth(3);
    ball_y.setLenth(3);
    far__.setLenth(1000);
}


void BALL::begin(){
    Serial8.begin(57600);
}


int BALL::getBallposition(){
    float x = ball_x.returnAve();
    float y = ball_y.returnAve();
    if(x == 0 || y == 0){
        flag = 0;
        return flag;
    }
    else{
        flag = 1;
    }
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
    dx = (far - far_old);
    far_ = far__.sum(dx);
    x_pos = x;
    y_pos = y;
    far_old = far;
    return flag;
}

void BALL::print(){
    Serial.print(" ang : ");
    Serial.print(ang);
    Serial.print(" far : ");
    Serial.print(far);
    Serial.print(" x : ");
    Serial.print(x_pos);
    Serial.print(" y : ");
    Serial.print(y_pos);
    Serial.print(" dx : ");
    Serial.print(dx);
    Serial.print(" catch : ");
    Serial.print(ball_get);
    Serial.print(" flag : ");
    Serial.print(flag);
}