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

const int stop_range[2]= {7,30};
double goval_a;

int A = 0;

int line_onoff_A = 0;
int line_onoff_B = 999;

int K = 0;

int A_line = 0;
int B_line = 999;

/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


const double val_max = 100;         //モーターの出力の最大値

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  goval_a = val_max / (stop_range[1] - stop_range[0]);
  
  Switch(1);
  A = 10;
}




void loop(){
  double AC_val = 100;
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
    line_onoff_A = 1;
    if(line_onoff_A != line_onoff_B){
      line_onoff_B = line_onoff_A;
    }

    if(abs(line.Lvec_Dir + ac.dir) < 30 || 150 < abs(line.Lvec_Dir + ac.dir)){  //まっすぐのところにいたら
      A_line = 1;
      if(A_line != B_line){
        B_line = A_line;
        line.Lvec_X_target = 0.3;
      }
      line_kkp = -line.LP_X;
      moter(line_kkp,-ball.PD_val_y,AC_val);  //縦軸がライントレース,横軸がボールの動き
      Serial.print("ラインの上だよ ");
      if(ball.ang < 0){
        go_LR = 0;
      }
      else{
        go_LR = 1;
      }
    }

    else{  //ラインの曲がってるとこにいたら
      A_line = 3;
      if(A_line != B_line){
        B_line = A_line;
        line.Lvec_X_target = -0.4;
      }
      moter(30,-ball.PD_val_y * 0.5,AC_val);  //縦軸がライントレース,横軸がボールの動き
    }
    Ldir_last = line.Lvec_Dir;  //最新のラインの角度を取得
    A = 40;
  }

  if(A == 20){
    int go_flag = 0;
    double go_border[2];
    angle balldir(ball.ang,true);

    if(line.Lvec_Dir < 0){
      go_border[0] = line.Lvec_Dir;
      go_border[1] = line.Lvec_Dir + 180;
    }
    else{
      go_border[0] = line.Lvec_Dir - 180;
      go_border[1] = line.Lvec_Dir;
    }

    balldir.to_range(go_border[0],false);

    if(go_border[0] < balldir.degrees && balldir.degrees < go_border[1]){
      go_flag = 0;
    }
    else{
      go_flag = 1;
    }

    go_ang = go_border[go_flag] + 90;
    go_ang.to_range(180,true);

    for(int i = 0; i < 2; i++){
      if((go_border[i] - stop_range[0] < ball.ang && ball.ang < go_border[i] + stop_range[0])){
        stop_flag = 999;
      }
      else if(go_border[i] - stop_range[1] < ball.ang && ball.ang < go_border[i] + stop_range[1]){
        goval = (abs(ball.ang) - stop_range[0]) * goval_a;
      }
    }
    if(170 < abs(go_ang.degrees)){
      stop_flag = 999;
    }

    A = 50;
  }
  if(A == 50){
    // Serial.print(" 進む角度 : ");
    // Serial.print(go_ang.degrees);
    Serial.print(" 進む角度 : ");
    Serial.print(go_ang.degrees);
    Serial.print(" 進む速さ : ");
    Serial.print(goval);
    Serial.println();
    MOTER.moveMoter(go_ang,goval,AC_val,stop_flag,line);
    A = 10;

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