#include<timer.h>
#pragma once


class A_B{
    public:
    A_B(int M){
        A = 0;
        B = 999;
        max = M;
    }
    int setA(int);
    int A;
    int B;
    unsigned long max = 0;
    timer T;
};