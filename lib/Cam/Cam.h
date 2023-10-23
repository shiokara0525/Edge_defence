#pragma once

#include<timer.h>
#include<MA.h>

class Cam{
    private:
        int B = 999;
        int A = 0;
    public:
        Cam();
        float ang;
        int LR = 0;
        float Size;
        void print();
        float P = 0;
        int on = 0;
        int color = 0;
        MA csize;
};