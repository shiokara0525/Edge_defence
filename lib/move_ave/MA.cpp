#include<MA.h>


void MA::setLenth(int lenth){
    this->lenth = lenth;
}



float MA::demandAve(float result){
    record[count % lenth] = result;
    float ave = 0;
    for(int i = 0; i < lenth; i++){
        ave += record[i];
    }

    ave /= (lenth < count ? lenth : count);
    // Serial.print(ave);
    // Serial.print(" ");
    count++;
    return ave;
}


float MA::sum(float result){
    record[count % lenth] = result;
    float sum = 0;
    for(int i = 0; i < lenth; i++){
        sum += record[i];
    }
    count++;
    return sum;
} 



float MA::returnAve(){
    float ave = 0;
    for(int i = 0; i < lenth; i++){
        ave += record[i];
    }

    ave /= (lenth < count ? lenth : count);
    return ave;
}




void MA::reset(){
    for(int i = 0; i < lenth; i++){
        record[i] = 0;
    }
}