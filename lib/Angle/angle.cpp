#include<angle.h>

angle::angle(double ang,bool flag){
    if(flag == true){
        degrees = ang;
        radians = radians(ang);
    }
    else{
        radians = ang;
        degrees = degrees(ang);
    }
}



angle::angle(double ang,bool ang_unit,double border,bool border_flag){
    if(ang_unit == true){
        degrees = ang;
        radians = radians(ang);
    }
    else{
        radians = ang;
        degrees = degrees(ang);
    }

    float max;
    float low;
    if(border_flag == true){
        max = border;
        low = border - 360;
    }
    else{
        low = border;
        max = border + 360;
    }

    while(degrees < low){
        degrees += 360;
    }

    while(max < degrees){
        degrees -= 360;
    }
}



double angle::to_range(double border,bool flag){
    float max;
    float low;
    if(flag == true){
        max = border;
        low = border - 360;
    }
    else{
        low = border;
        max = border + 360;
    }

    while(degrees < low){
        degrees += 360;
    }

    while(max < degrees){
        degrees -= 360;
    }

    return degrees;
}