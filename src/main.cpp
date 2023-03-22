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

const int stop_range[2]= {7,30};
double goval_a;

const int Tact_Switch = 15;  //スイッチのピン番号 
const int pingPin = 32;
const double pi = 3.1415926535897932384;  //円周率

void Switch(int);
int readUS();

Ball ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer;
moter MOTER;
MA US;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


const double val_max = 120;         //モーターの出力の最大値

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
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  int goval = val_max;
  int stop_flag = 5;

  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();             //姿勢制御の値入手
    line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    A = 20;
  }

  if(A == 15){
    int far = readUS();
    Serial.print(" 距離 : ");
    Serial.print(far);
    if(40 < far){
      go_ang = 179.9;
      while(line.getLINE_Vec() == 0){
        AC_val = ac.getAC_val();
        //MOTER.moveMoter(go_ang,goval,AC_val,0,line);
      }
    }
    A = 20;
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
        goval = 0;
        stop_flag = 999;
      }
      else if(go_border[i] - stop_range[1] < ball.ang && ball.ang < go_border[i] + stop_range[1]){
        goval = (stop_range[1] - abs(ball.ang)) * goval_a;
      }
    }
    if(170 < abs(go_ang.degrees)){
      stop_flag = 999;
    }

    A = 50;
  }

  if(A == 50){
    Serial.print(" 進む角度 : ");
    Serial.print(go_ang.degrees);
    Serial.print(" 進む速さ : ");
    Serial.print(goval);
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
      delay(100);
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
      delay(100);
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




int readUS(){
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
  Serial.print(duration);

  //パルスの長さを半分に分
  duration=duration/2;
  //cmに変換
  cm = int(duration/29); 
  if(cm == 0){
    cm = 100;
  }


  delayMicroseconds(100);
  cm = US.demandAve(cm);
  for(int i = 0; i < 8; i++){
    if(cm < 20 + (i * 10)){
      cm = 20 + (i * 10);
      break;
    }
  }
  return cm;
}