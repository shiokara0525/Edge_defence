#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<moter.h>

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //どのチャプターに移動するかを決める変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int line_flag = 0;               //最初にどんな風にラインの判定したか記録
double edge_flag = 0; //ラインの端にいたときにゴールさせる確率を上げるための変数だよ(なんもなかったら0,右の端だったら1,左だったら2)

const int stop_range = 8;

const int Tact_Switch = 15;  //スイッチのピン番号 
const double pi = 3.1415926535897932384;  //円周率

void Switch(int);

Ball ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer;
moter MOTER;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


const double val_max = 120;         //モーターの出力の最大値

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  
  Switch(1);
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  int goval = val_max;
  int ball_flag = 0;  //ボールがコート上にあるかないか
  int stop_flag = 5;

  if(A == 10){  //情報入手
    ball_flag = ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();             //姿勢制御の値入手
    line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    if(ball_flag == 0){  //ボール見てなかったら
      A = 15;  //止まるとこ
    }
    else{  //ボール見てたら
      A = 20;  //進む角度決めるとこ
    }
  }

  if(A == 15){
    MOTER.moter_0();
    while(1){
      ball.getBallposition();
      if(ball.far_x != 0 || ball.far_y != 0){
        break;
      }
    }
    A = 50;
  }

  if(A == 20){
    int go_flag = 0;
    double go_border[3];
    angle balldir(ball.ang,true);

    if(line.Lvec_Dir < 0){
      go_border[0] = line.Lvec_Dir;
      go_border[1] = line.Lvec_Dir + 180;
      go_border[2] = line.Lvec_Dir + 360;
    }
    else{
      go_border[0] = line.Lvec_Dir - 180;
      go_border[1] = line.Lvec_Dir;
      go_border[2] = line.Lvec_Dir + 180;
    }

    balldir.to_range(go_border[0],false);

    if(go_border[0] < balldir.degrees && balldir.degrees < go_border[1]){
      go_flag = 0;
    }
    else{
      go_flag = 1;
    }

    double goang_2[2] = {go_border[0] + 90,go_border[1] + 90};
    go_ang = goang_2[go_flag];

    if((go_border[0] - stop_range < ball.ang && ball.ang < go_border[0] + stop_range)){
      stop_flag = 999;
    }
    else if((go_border[1] - stop_range < ball.ang && ball.ang < go_border[1] + stop_range)){
      stop_flag = 999;
    }
    
    A = 50;
  }
  if(A == 50){
    Serial.println();
    MOTER.moveMoter(go_ang,goval,AC_val,stop_flag,line);
    A = 10;

    if(digitalReadFast(Tact_Switch) == LOW){
      Switch(2);
    }
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/

void Switch(int flag){
  int A = 0;
  while(1){
    if(A == 0){
      if(flag == 2){
        if(digitalRead(Tact_Switch) == HIGH){
          delay(100);
          digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
          MOTER.moter_0();
          A = 1;
        }
      }
      else{
        A = 1;
      }
    }

    if(A == 1){
      if(digitalRead(Tact_Switch) == LOW){
        A = 2;
      }
    }

    if(A == 2){
      if(flag == 1){
        ball.setup();
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        line.setup();  //ラインとかのセットアップ
      }
      else{
        ac.setup_2();  //姿勢制御の値リセットしたよ
        digitalWrite(line.LINE_light,HIGH);  //ライン付けたよ
      }
      
      if(digitalRead(Tact_Switch) == HIGH){
        A = 3;  //準備オッケーだよ 
      }
    }

    if(A == 3){
      if(digitalRead(Tact_Switch) == LOW){
        A = 4;  //スイッチはなされたらいよいよスタートだよ
      }
    }
    
    if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        break;
      }
    }
  }
  return;
}