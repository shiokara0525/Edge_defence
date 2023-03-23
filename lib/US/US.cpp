#include<US.h>



us::us(){
    us_ma.setLenth(10);
}

int us::readFar(){
    unsigned long duration;
    int cm;
    //ピンをOUTPUTに設定（パルス送信のため）
    pinMode(pingPin, OUTPUT);
    //LOWパルスを送信
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);  
    //HIGHパルスを送信
    digitalWrite(pingPin, HIGH);  
    //5uSパルスを送信してPingSensorを起動
    delayMicroseconds(5); 
    digitalWrite(pingPin, LOW); 
    
    //入力パルスを読み取るためにデジタルピンをINPUTに変更（シグナルピンを入力に切り替え）
    pinMode(pingPin, INPUT);
    //入力パルスの長さを測定
    duration = pulseIn(pingPin, HIGH,6000);

    //パルスの長さを半分に分
    duration=duration/2;
    //cmに変換
    cm = int(duration/29); 
    if(cm == 0){
        cm = 100;
    }


    delayMicroseconds(100);
    cm = us_ma.demandAve(cm);
    for(int i = 0; i < 8; i++){
        if(cm < 20 + (i * 10)){
        cm = 20 + (i * 10);
        break;
        }
    }
    return cm;
}