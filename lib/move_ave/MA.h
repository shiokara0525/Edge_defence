#include<Arduino.h>
#pragma once

class MA{
    
    private:
        int lenth;
        float record[1000];
        int count = 0;
    public:
        void setLenth(int);
        float demandAve(float);
        void reset();
        float sum(float);
};