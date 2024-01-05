#include<Cam.h>


Cam::Cam(){
    csize.setLenth(20);
    csize.reset();
}


void Cam::begin(){
    Serial1.begin(115200);
}



void Cam::print(){
    if(on == 0){
        Serial.print("No block detected");
    }
    else{
        Serial.print("  ang: ");
        Serial.print(ang);
        Serial.print("  size: ");
        Serial.print(Size);
    }
}