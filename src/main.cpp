#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
/*--------------------------------------------------------------定数----------------------------------------------------------------------*/


const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;
const int toggle_Switch = 14;
const double pi = 3.1415926535897932384;  //円周率

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //スイッチを押したらメインプログラムに移動できる変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

int line_flag = 0;  //どんな風にラインの判定したか記録

Ball ball;  //ボールのクラスのオブジェクトを生成
AC ac;  //姿勢制御のクラスのオブジェクトを生成
LINE line;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


void moter(double,int,double,int);  //モーター制御関数
void moter_0();
double val_max = 125;  //モーターの最大値
int Mang[] = {45,135,225,315};  //モーターの角度
double mSin[] = {1,1,-1,-1};  //行列式のsinの値
double mCos[] = {1,-1,-1,1};  //行列式のcosの値

double val_moter[4][5];
int count_moter = 0;




/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  
  
  
  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
  }  //モーターのピンと行列式に使う定数の設定
  
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
        ball.setup();
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
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  double goang = 0;  //進みたい角度
  
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  int ball_flag = 0;  //ボールがコート上にあるかないか
  int stop_flag = 0;  //ラインをちょっと踏んでるときにどんな動きをするかの旗 
  int goval = val_max;  //動くスピード決定

  if(A == 10){  //情報入手
    ball_flag = ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();             //姿勢制御の値入手
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    if(ball_flag == 0){  //ボール見てなかったら
      A = 15;  //止まるとこ
    }
    else{  //ボール見てたら
      A = 20;  //進む角度決めるとこ
    }
  }

  if(A == 15){
    while(1){
      moter_0();
      ball.getBallposition();
      if(ball.far_x != 0 || ball.far_y != 0){
        break;
      }
    }
    A = 30;
  }

  if(A == 20){  //進む角度決めるとこ
    double ang_defference = 80.0 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)

    if(ball.ang < 0){  //ここで進む角度決めてるよ!
      goang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : -45) * (1.0 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }
    else{
      goang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : 45) * (1.0 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }

    if(20 < abs(ball.ang) && abs(ball.ang) < 50){  //ボールが斜め前にあったら進む速さちょっと落とすよ
      goval -= 25;
    }

    if(270 < abs(goang)){  //回り込みの差分が大きすぎて逆に前に進むことを防ぐよ
      if(goang < 0){
        goang = -270;
      }
      else{
        goang = 270;
      }
    }
    
    while(180 < abs(goang)){  //角度が180°を超えたらちょっとわかりづらいからわかりやすくするよ
      if(goang < 0){
        goang += 360;
      }
      else{
        goang -= 360;
      }
    }
    A = 30;
  }
  
  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(どういう風に動くか決めるよ!)
        B_line = A_line;
        if(line.Lrange_num == 1){
          if(abs(line.Lvec_Dir) < 45){  //前でライン踏んでたら
            if(abs(goang) < 90){  //前方向に進もうとしてたら
              line_flag = 1;
              stop_flag = 1;
            }
          }
          else if(45 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 135){  //真横にライン踏んでたら
            if(goang < 0 && line.Lvec_Dir < 0){  //左方向でライン踏んでて左に進もうとしてたら
              line_flag = 4;  //これはライン離れるまで同じ動きするための変数(ラインを通り越して左で踏んでたはずが右で踏んじゃった~みたいなことになったら困るから)
              stop_flag = 4;
            }
            else if(goang > 0 && line.Lvec_Dir > 0){  //右方向でライン踏んでて右に進もうとしてたら
              line_flag = 2;
              stop_flag = 2;
            }
          }
          else if(abs(line.Lvec_Dir) > 135){  //後ろでライン踏んでたら
            if(90 < abs(goang)){  //後ろ向きに進もうとしてたら
              line_flag = 3;
              stop_flag = 3;
            }
          }
        }
        else{  //ラインをまたいでいたらその真逆に進むよ
          goang = line.Lvec_Dir - 180;
          stop_flag = 0;
        }
      }
      else{  //連続でライン踏んでたら(踏んだまま斜めのとこ来て動き続けてたら怖いから斜めのとこ対策)
        if(1 < line.Lrange_num){  //ラインをまたいでいたらその真逆に動くよ
          if(abs(line.Lvec_Dir) < 15){
            goang = 179;
            if(line_flag == 3){
              goang = 0;
            }
          }
          else if(15 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 45){
            if(line.Lvec_Dir < 0){
              goang = 150;
            }
            else{
              goang = -150;
            }

            if(line_flag == 3){
              goang = 0;
            }
          }
          else if(45 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 75){
            if(line.Lvec_Dir < 0){
              goang = -120;

              if(line_flag == 4){
                goang = 90;
              }
            }
            else{
              goang = 120;

              if(line_flag == 2){
                goang = -90;
              }
            }
          }
          else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){
            if(line.Lvec_Dir < 0){
              goang = 90;

              if(line_flag == 2){
                goang = -90;
              }
            }
            else{
              goang = -90;

              if(line_flag == 4){
                goang = 90;
              }
            }
          }
          else if(105 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 135){
            if(line.Lvec_Dir < 0){
              goang = 60;
              if(line_flag == 2){
                goang = -90;
              }
            }
            else{
              goang = -60;
              if(line_flag == 4){
                goang = 90;
              }
            }            
          }
          else if(135 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 165){
            if(line.Lvec_Dir < 0){
              goang = 30;
            }
            else{
              goang = -30;
            }

            if(line_flag == 1){
              goang = 179;
            }  
          }
          else if(abs(line.Lvec_Dir) < 165){
            goang = 0;
            if(line_flag == 1){
              goang = 179;
            }  
          }
          stop_flag = 0;
        }
        else{
          stop_flag = line_flag;
        }
      }
      if(line_flag == 0){  //ライン踏んでるけど別に進んでいいよ～って時
        B_line = 0;  //ラインで特に影響受けてないからライン踏んでないのと扱い同じのほうが都合いいよね!
      }
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
      }
      line_flag = 0;
    }

    while(180 < abs(goang)){
      if(goang < 0){
        goang += 360;
      }
      else{
        goang -= 360;
      }
    }
    A = 40;

  }

  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    moter(goang,goval,AC_val,stop_flag);  //モーターの処理(ここで渡してるのは進みたい角度,姿勢制御の値,ライン踏んでその時どうするか~ってやつだよ!)

    A = 10;
    Serial.print(" 進む角度 : ");
    Serial.print(goang);


    if(digitalRead(Tact_Switch) == LOW){
      A = 50; //スイッチが押されたら
    }

  }
  if(A == 50){
    if(digitalRead(Tact_Switch) == HIGH){
      delay(100);
      A = 60;
      digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
      moter_0();
    }
  }
  if(A == 60){
    if(digitalRead(Tact_Switch) == LOW){
      ac.setup_2();  //姿勢制御の値リセットしたよ
      A = 70;
    }
  }
  if(A == 70){
    digitalWrite(line.LINE_light,HIGH);  //ライン付けたよ
    if(digitalRead(Tact_Switch) == HIGH){
      A = 80;  //準備オッケーだよ 
    }
  }
  if(A == 80){
    if(digitalRead(Tact_Switch) == LOW){
      A = 90;  //スイッチはなされたらいよいよスタートだよ
    }
  }
  if(A == 90){
    if(digitalRead(Tact_Switch) == HIGH){
      A = 10;  //スタート!
    }
  }
}


/*---------------------------------------------------------------モーター制御関数-----------------------------------------------------------*/


void moter(double ang,int val,double ac_val,int go_flag){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double Mval_n[4] = {0,0,0,0};
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(radians(ang));  //進みたいベクトルのx成分
  double mval_y = sin(radians(ang));  //進みたいベクトルのy成分
  count_moter++;

  float back_val = 2;
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    if(go_flag == 0){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    }
    
    else if(go_flag == 1){  //前のストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * -back_val;
    }
    else if(go_flag == 2){  //右のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * -back_val;
    }
    else if(go_flag == 3){  //後ろのストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * back_val;
    }
    else if(go_flag == 4){  //左のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * back_val;
    }
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){
    Mval[i] /= g;
    Mval_n[i] = Mval[i];
    val_moter[i][(count_moter % 5)] = Mval[i];
    double valsum_moter = 0;
    for(int j = 0; j < 5; j++){
      valsum_moter += val_moter[i][j];
    }
    Mval[i] = valsum_moter / 5;
    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  
  for(int i = 0; i < 4; i++){

    if(line_flag == 0){
      Mval[i] = Mval[i] / h * max_val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    }
    else{
      Mval[i] = Mval_n[i] / h * max_val + ac_val;
    }

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
  
  if(ac.flag == 1){  //姿勢制御のせいでモータードライバがストップしちゃいそうだったら
    delay(100);   //ちょっと待つ
    ac.flag = 0;  //姿勢制御のフラグを下ろす
  }
}




void moter_0(){
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
  }
}