
#include<A_B.h>


int A_B::setA(int A_){
    A = A_;
    if(A != B){
        B = A;
        T.reset();
    }

    if(max <= T.read_ms()){
        return A_;
    }
    else{
        return 0;
    }
}