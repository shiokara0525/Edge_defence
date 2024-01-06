
#include<A_B.h>


int A_B::setA(int A_){
    A = A_;
    if(A != B){
        B = A;
        T.reset();
    }

    if(A == 2){
        Serial.print(B);
        Serial.print(" ");
        Serial.print(T.read_ms());
    }

    if(max <= T.read_ms()){
        return A_;
    }
    else{
        return 0;
    }
}