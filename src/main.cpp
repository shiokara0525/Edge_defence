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

const int val_max = 100;

double mSin[4];
double mCos[4];

int A = 0;

int A_line = 0;
int B_line = 999;

float Ldir_last;




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
  double AC_val;
  double line_kkp;
  double ball_kkp;
  int Line_flag;

  if(A == 10){  //センサーの値取得したりステートの分岐したりするとこ

    AC_val = ac.getAC_val();
    Line_flag = line.getLINE_Vec();
    ball.getBallposition();
    if(Line_flag == 1){  //ライン踏んでたら
      A = 20;  //ラインの上動くとこ
    }
    else{  //ライン踏んでなかったら
      A = 30;  //がんばってラインに戻るとこ
    }

  }
  if(A == 20){  //ラインの上動くよ

    if(abs(line.Lvec_Dir) < 30){  //まっすぐのところにいたら
      A_line = 0;
      if(A_line != B_line){
        B_line = A_line;
      }
      line_kkp = -line.LP_X;
      moter(line_kkp,ball_kkp,AC_val);  //縦軸がライントレース,横軸がボールの動き
    }
    else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){  //縦のところにいたら
      A_line = 1;
      if(A_line != B_line){
        B_line = A_line;
      }
      line_kkp = -line.LP_Y;
      moter(ball.PD_val_y,line_kkp,AC_val);
    }
    else{  //ラインの曲がってるとこにいたら
      A_line = 2;
      if(A_line != B_line){
        B_line = A_line;
      }
      line_kkp = -line.LP_X;
      moter(ball.PD_val_y * 0.3,line_kkp,AC_val);  //横軸がライントレース,縦軸がボールの動き(だいぶ出力弱め)
    }
    Ldir_last = line.Lvec_Dir;  //最新のラインの角度を取得
    A = 40;

  }
  if(A == 30){
    if(B_line == 0){
      if(abs(Ldir_last) < 90){
        moter(-80,0,AC_val);
      }
      else{
        moter(80,0,AC_val);
      }
    }
    else{
      if(0 < Ldir_last){  //最後に踏んだラインが右方向だったら
        moter(0,-80,AC_val);  //左方向に進む
      }
      else{
        moter(0,80,AC_val);  //右方向に進む
      }
    }
    A = 40;
  }  
  if(A == 40){  //最終的に処理するとこ(モーターとかも)
    A = 10;
    Serial.println("");

    if(digitalRead(Tact_Switch) == LOW){
      A = 50; //スイッチから手が離されるのを待つ
    }
    
  }
  if(A == 50){
    if(digitalRead(Tact_Switch) == HIGH){
      delay(100);
      A = 60;
      for(int i = 0; i < 4; i++){
        digitalWrite(pah[i],LOW);
        analogWrite(ena[i],0);
      }
      digitalWrite(line.LINE_light,LOW);
    }
  }
  if(A == 60){
    if(digitalRead(Tact_Switch) == LOW){
      A = 70;
    }
  }
  if(A == 70){
    digitalWrite(line.LINE_light,HIGH);
    if(digitalRead(Tact_Switch) == HIGH){
      A = 80;
      ac.setup_2();
    }
  }
  if(A == 80){
    if(digitalRead(Tact_Switch) == LOW){
      A = 90;
    }
  }
  if(A == 90){
    if(digitalRead(Tact_Switch) == HIGH){
      A = 10;
    }
  }

}




void moter(double val_x,double val_y,double ac_val){  //モーター制御する関数
  double Mval[4] = {0,0,0,0};
  double val = val_max;

  val -= ac_val;

  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * val_x + mCos[i] * val_y;
    if(abs(val_y) < 10){
      Mval[i] = 0;
    }
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] + ac_val;
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