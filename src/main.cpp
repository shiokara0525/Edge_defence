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

int A_ball = 0;  //ライン踏んでるか踏んでないか
int B_ball = 999;  //前回踏んでるか踏んでないか

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int line_flag = 0;               //最初にどんな風にラインの判定したか記録
double edge_flag = 0; //ラインの端にいたときにゴールさせる確率を上げるための変数だよ(なんもなかったら0,右の端だったら1,左だったら2)

const int stop_range[2]= {7,30};
double goval_a;

const int Tact_Switch = 15;  //スイッチのピン番号 
const int pingPin = 32;
const double pi = 3.1415926535897932384;  //円周率

int Far = 100;

void Switch(int);
int readUS();

Ball ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer;
timer Sentor;
moter MOTER;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/

const double val_max = 100;         //モーターの出力の最大値

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  goval_a = val_max / (stop_range[1] - stop_range[0]);
  
  Switch(1);
  if(45 < abs(ac.dir)){
    Far = 100;
  }
  else{
    Far = readUS();
  }
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  int goval = val_max;
  int ball_flag = 0;  //ボールがコート上にあるかないか
  int stop_flag = 5;
  int far = 0;

  if(A == 10){  //情報入手
    ball_flag = ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();             //姿勢制御の値入手
    line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定

    if(abs(ac.dir) < 5){
      Far = readUS();
    }

    far = Far;

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

    if(abs(ball.ang) < 5){
      A_ball = 0;
      if(A_ball != B_ball){
        B_ball = A_ball;
        Sentor.reset();
      }
    }
    else{
      A_ball = 1;
      if(A_ball != B_ball){
        B_ball = A_ball;
        Sentor.reset();
      }
      Sentor.reset();
    }

    if(3000 < Sentor.read_ms()){
      go_ang = ball.ang;
      A = 30;
    }
    else{
      A = 40;
    }
  }
  if(A == 30){
    int A_ = 0;
    int A_line = 0;
    int B_line = 999;
    while(1){
      A_ = 10;
      if(A_ == 10){
        ball_flag = ball.getBallposition();
        int line_flag = line.getLINE_Vec();
        AC_val = ac.getAC_val();
        if(ball_flag == 0){
          A_ = 15;
        }
        else{
          A_ = 20;
        }
      }

      if(A_ == 15){
        while(1){
          MOTER.moter_0();
          ball.getBallposition();
          if(ball.far_x != 0 || ball.far_y != 0){
            A_ = 20;
            break;
          }
        }
      }
      
      if(A_ == 20){
        if(abs(ball.ang) < 30){
          go_ang = ball.ang;
        }
        else{
          double ang_defference = 75.0 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)
          /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

          if(ball.ang < 0){  //ここで進む角度決めてるよ!(ボールの角度が負の場合)
            go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : -45) * (0.2 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
          }
          else{  //(ボールの角度が正の場合)
            go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : 45) * (0.2 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
          }
        }
        A_ = 30;
      }

      if(A_ == 30){
        if(line_flag == 1){  //ラインがオンだったら
          A_line = 1;
          angle linedir(line.Lvec_Dir,true);

          if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
            B_line = A_line;

            line_flag = line.switchLineflag(linedir);

            if(line_flag == 3){
              A = 20;
              break;
            }

            if(line.Lrange_num == 1){  //ラインをちょっと踏んでるとき(ここでは緊急性が高くないとする)
              stop_flag = line_flag;   //緊急性高くないし、まともにライン踏んでるから緩めの処理するよ
            }
            else{  //斜めに踏んでるか、またはラインをまたいでるとき(緊急性が高いとするよ,進む角度ごと変えるよ)
              go_ang = line.decideGoang(linedir,line_flag);
            }

            MOTER.moter_0();
            delay(75);
          }
          else{  //連続でライン踏んでるとき
            if(1 < line.Lrange_num){  //ラインをまたいでいたらその真逆に動くよ
              go_ang = line.decideGoang(linedir,line_flag);
              stop_flag = 0;
            }
            else{
              stop_flag = line.switchLineflag(linedir);
              line_flag = stop_flag;
            }
          }
          if(line_flag == 0){  //ライン踏んでるけど別に進んでいいよ～って時
            B_line = 0;  //ラインで特に影響受けてないからライン踏んでないのと扱い同じのほうが都合いいよね!
          }
        }
        else{
          A_line = 0;
          if(A_line != B_line){
            B_line = A_line;
          }
          line_flag = 0;
        }
        A_ = 50;
      }

      if(A_ = 50){
        MOTER.moveMoter(go_ang,goval,AC_val,stop_flag,line);
        A_ = 10;
      }
    }
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

    if(digitalReadFast(Tact_Switch) == LOW){
      Switch(2);
    }
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/


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
  return cm;
}






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