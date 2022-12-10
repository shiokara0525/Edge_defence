#include<Arduino.h>
#include<ball.h>
#include<line.h>
#include<ac.h>



Ball ball;
LINE line;
AC ac;

const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;
int Mang[4] = {45,135,225,315};  //モーターの角度

void moter(double,double,double);

const int val_max = 200;

double mSin[4];
double mCos[4];

int A = 0;




void setup(){
  for(int i = 0; i < 4; i++){
    pinMode(ena[i] , OUTPUT);
    pinMode(pah[i] , OUTPUT);
    mSin[i] = sin(radians(Mang[i]));
    mCos[i] = cos(radians(Mang[i]));
  }

  ball.setup();

  while(A != 10){
    if (A == 0){
      A = 1; //スイッチが押されるのを待つ
    }
    else if(A == 1){
      if(digitalRead(Tact_Switch) == LOW){
        A = 2; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 2){
      if(digitalRead(Tact_Switch) == HIGH){  //手が離されたらその時点で正面方向決定
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        line.setup();  //ラインとかのセットアップ
        delay(100);
        A = 3;  //メインプログラムいけるよ
      }
    }
    else if(A == 3){
      if(digitalRead(Tact_Switch) == LOW){
        A = 4; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        A = 10; //スイッチから手が離されるのを待つ
      }
    }
  }
  Serial.begin(9600);
}


void loop(){
  double AC_val = ac.getAC_val();
  double line_kkp;
  double ball_kkp;

  line.getLINE_Vec();
  ball.getBallposition();

  line_kkp = line.LP_X * line.kp * -1;
  ball_kkp = ball.far_y * ball.kp * -1;

  moter(line_kkp,ball_kkp,AC_val);

  Serial.print(" 横方向の出力 : ");
  Serial.print(ball_kkp);
  Serial.print(" 縦方向の出力 : ");
  Serial.print(line_kkp);
  Serial.println("");
}




void moter(double val_x,double val_y,double ac_val){  //モーター制御する関数
  double Mval[4] = {0,0,0,0};
  double val = val_max;

  val -= ac_val;

  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * val_x + mCos[i] * val_y;
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i]+ ac_val;
    if(ac.flag == 1){
      digitalWrite(pah[i],LOW);
      analogWrite(ena[i],0);
    }
    else if(Mval[i] > 0){            //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]); //モーターの回転速度を設定
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);     //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }
  }  
}