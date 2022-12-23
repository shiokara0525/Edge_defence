#include<Arduino.h>
#include<ball.h>
#include<line.h>
#include<ac.h>
#include<timer.h>



Ball ball;
LINE line;
AC ac;
timer timer_sawa;

const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;
int Mang[4] = {45,135,225,315};  //モーターの角度

void moter(double,double,double);
void moter_0();
void moter_back(double,double);

const int Val_max = 100;

double mSin[4] = {1,1,-1,-1};
double mCos[4] = {1,-1,-1,1};

int A = 0;

int A_line = 0;
int B_line = 999;

float Ldir_last;
int count = 0;
int go_flag = 0;
int go_LR = 0;




void setup(){
  for(int i = 0; i < 4; i++){
    pinMode(ena[i] , OUTPUT);
    pinMode(pah[i] , OUTPUT);
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
      ball.getBallposition();
      if(digitalRead(Tact_Switch) == LOW){
        A = 4; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        A = 10;
      }
    }
  }
  Serial.begin(9600);
}


void loop(){
  double AC_val;
  double line_kkp;
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
    if(abs(line.Lvec_Dir + ac.dir) < 30 || 150 < abs(line.Lvec_Dir + ac.dir)){  //まっすぐのところにいたら
      A_line = 1;
      if(A_line != B_line){
        B_line = A_line;
      }
      line_kkp = -line.LP_X;
      moter(line_kkp,ball.PD_val_y,AC_val);  //縦軸がライントレース,横軸がボールの動き
      Serial.print("ラインの上だよ ");
      if(ball.ang < 0){
        go_LR = 0;
      }
      else{
        go_LR = 1;
      }
    }

    else{  //ラインの曲がってるとこにいたら
      A_line = 2;
      if(A_line != B_line){
        B_line = A_line;
        Serial.print(" 曲がってるときのフラグ : ");
        Serial.print(go_flag);

        if(0 < ball.ang){
          go_flag = 2;
        }
        else{
          go_flag = 1;
        }
      }

      if(go_flag = 3 || go_flag == 4){
        moter(line_kkp,ball.PD_val_y,AC_val);  //縦軸がライントレース,横軸がボールの動き
        if(go_flag == 3){
          if(0 < ball.ang){
            go_flag = 1;
          }
        }
        else if(go_flag == 4){
          if(ball.ang < 0){
            go_flag = 2;
          }
        }
      }

      while(go_flag == 1 || go_flag == 2){
        Line_flag = line.getLINE_Vec();
        ball.getBallposition();
        
        if(go_flag == 1){
          if(0 < ball.ang){
            moter_0();
            Serial.print(" 右にボールあるよ ");
          }
          else{
            go_flag = 3;
          }
        }
        else if(go_flag == 2){
          if(ball.ang < 0){
            moter_0();
            Serial.print(" 左にボールあるよ ");
          }
          else{
            go_flag = 4;
          }
        }

        if(Line_flag == 0){
          A = 30;
          break;
        }
        ball.print();
        Serial.println("");
      }
    }
    Ldir_last = line.Lvec_Dir;  //最新のラインの角度を取得
    A = 40;
  }

  if(A == 30){
    A_line = 0;
    if(A_line != B_line){
      B_line = A_line;
    }

    if(Ldir_last < 0){
      Serial.print("出たよ(右に) ");
      moter(0,100,AC_val);
    }
    else{
      Serial.print("出たよ(左に) ");
      moter(0,-100,AC_val);
    }
    A = 40;
  }

  if(A == 40){  //最終的に処理するとこ(モーターとかも)
    A = 10;
    ball.print();
    Serial.print(" 今いるステート : ");
    Serial.print(A_line);
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
  double val = Val_max;

  val -= ac_val;

  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * val_x + mCos[i] * val_y;
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




void moter_0(){
  Serial.print("止まってるよ");
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
  }
}




void moter_back(double ang,double val){
  double goval_x = cos(radians(ang));
  double goval_y = sin(radians(ang));
  double Mval[4] = {0,0,0,0};
  double g = 0;
  int val_max = Val_max;

  val_max -= val;

  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * goval_x + mCos[i] * goval_y; //モーターの回転速度を計算(行列式で管理)

    if(g < Mval[i]){
      g = Mval[i];
    }
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] / g * val_max + val;
    if(Mval[i] > 0){            //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]); //モーターの回転速度を設定
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);     //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }
  }
}