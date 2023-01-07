#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
/*--------------------------------------------------------------定数----------------------------------------------------------------------*/


const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;
const int toggle_Switch = 14;
const double pi = 3.1415926535897932384;  //円周率

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //スイッチを押したらメインプログラムに移動できる変数
Ball ball;  //ボールのクラスのオブジェクトを生成
AC ac;  //姿勢制御のクラスのオブジェクトを生成
LINE line;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


void moter(double,double,int);  //モーター制御関数
void moter_0();
double val_max = 110;  //モーターの最大値
int Mang[4] = {45,135,225,315};  //モーターの角度
double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
double mCos[4] = {1,-1,-1,1};  //行列式のcosの値


int A_go = 0;  //どんな動きしてるか
int B_go = 999;  //前回どんな動きしてたか

int mawarikomi_flag = 0;

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

int line_flag = 0;  //どんな風にラインの判定したか記録


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
  int go_flag = 0;  //回り込みでは回り込みする方向、ラインではラインを踏んでる方向
  
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  double line_dir;


  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();  //姿勢制御の値入手
    Line_flag = line.getLINE_Vec();  //ライン踏んでるか踏んでないかを判定 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    line_dir = line.Lvec_Dir;
    line_dir += ac.dir;
    A = 20;
  }

  if(A == 20){
    double ang_defference = 10 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)
    abs(goang) < 90 ? (goang = ball.ang * 1.5) : (goang = ball.ang + 90);

    if(270 < abs(goang)){
      if(goang < 0){
        goang = -270;
      }
      else{
        goang = 270;
      }
    }
    
    if(abs(goang) > 180){
      if(goang < 0){
        goang += 360;
      }
      else{
        goang -= 360;
      }
    }

    if(ball.far_x == 0 || ball.far_y == 0){
      int flag = 0;
      while(flag != 1){
        moter_0();
        if(ball.far_x != 0 || ball.far_y != 0){
          flag == 1;
        }
      }
    }
    A = 30;
  }
  
  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(どういう風に動くか決めるよ!)
        B_line = A_line;
          if(60 < abs(line_dir) && abs(line_dir) < 120){  //真横にライン踏んでたら
            if(goang < 0 && line_dir < 0){  //左方向でライン踏んでて左に進もうとしてたら
              line_flag = 1;  //これはライン離れるまで同じ動きするための変数(ラインを通り越して左で踏んでたはずが右で踏んじゃった~みたいなことになったら困るから)
            }
            else if(goang > 0 && line_dir > 0){  //右方向でライン踏んでて右に進もうとしてたら
              line_flag = 2;
            }
          }
        else if(abs(line_dir) < 30){  //前でライン踏んでたら
          if(abs(goang) < 90){  //前方向に進もうとしてたら
            line_flag = 3;
          }
        }
        else if(abs(line_dir) > 150){  //後ろでライン踏んでたら
          if(90 < abs(goang)){  //後ろ向きに進もうとしてたら
            line_flag = 4;
          }
        }
        else if(30 < abs(line_dir) && abs(line_dir) < 70){  //前斜め方向にラインあったら
          if(line_dir < 0){  //左前斜め方向にラインあったら
            if(-180 < goang && goang < 90){  //右後ろ以外に進もうとしてたら
              line_flag = 5;
            }
          }
          else{  //右前斜め方向にラインあったら
            if(-90 < goang && goang < 180){  //左後ろ以外に進もうとしてたら
              line_flag = 6;
            }
          }
        }
        else if(120 < abs(line_dir) && abs(line_dir) < 150){  //後ろ斜め方向にラインあったら
          if(line_dir < 0){  //左後ろ斜めにラインあったら
            if(goang < 0 || -90 < goang ){  //右前以外に進もうとしてたら
              line_flag = 7;
            }
          }
          else{  //右後ろ斜めにラインあったら
            if(goang < -90 || 0 < goang){  //左前以外に進もうとしてたら
              line_flag = 8;
            }
          }
        }
      }

      else{  //連続でライン踏んでたら(踏んだまま斜めのとこ来て動き続けてたら怖いから斜めのとこ対策)
        int flag = 0;
        if(line_flag == 5 || line_flag == 6 || line_flag == 7 || line_flag == 8){
          flag = line_flag;
        }
        if(line_flag == 1 || line_flag == 2 || line_flag == 3 || line_flag == 4){
          if(30 < abs(line_dir) && abs(line_dir) < 60){  //前斜め方向にラインあったら
            if(line_dir < 0){  //左前斜め方向にラインあったら
              if(-180 < goang && goang < 90){
                line_flag = 5;
              }
            }
            else{  //右前斜め方向にラインあったら
              if(-90 < goang && goang < 180){
                line_flag = 6;
              }
            }
          }
          else if(120 < abs(line_dir) && abs(line_dir) < 150){  //後ろ斜め方向にラインあったら
            if(line_dir < 0){  //左後ろ斜めにラインあったら
              if(45 < goang || goang < -135 ){
                line_flag = 7;
              }
            }
            else{  //右後ろ斜めにラインあったら
              if(goang < -45 || 135 < goang ){
                line_flag = 8;
              }
            }
          }
        }


        if(line_flag == 1 || line_flag == 2){
          if(abs(line_dir) < 15){
            line_flag = 3;
          }
          else if(abs(line_dir) > 165){
            line_flag = 4;
          }
        }
        else if(line_flag == 3 || line_flag == 4){
          if(60 < abs(line_dir) && abs(line_dir) < 120){  //真横にライン踏んでたら
            if(goang < 0 && line_dir < 0){  //左方向でライン踏んでて左に進もうとしてたら
              line_flag = 1;
            }
            else if(goang > 0 && line_dir > 0){  //右方向でライン踏んでて右に進もうとしてたら
              line_flag = 2;
            }
          }
        }

        if(flag != 0){
          line_flag = flag;
        }
        go_flag = line_flag;
      }
      if(go_flag == 0){  //ライン踏んでるけど別に進んでいいよ～って時
        B_line = 0;  //ラインで特に影響受けてないからライン踏んでないのと扱い同じのほうが都合いいよね!
      }
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
      }
      go_flag = 0;  //ライン踏んでない
      line_flag = 0;
    }
    A = 40;
  }

  if(A == 40){  //最終的に処理するとこ(モーターとかも)    
    moter(goang,AC_val,go_flag);  //モーターの処理(ここで渡してるのは進みたい角度,姿勢制御の値,ライン踏んでその時どうするか~ってやつだよ!)

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
      digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
      for(int i = 0; i < 4; i++){  //モーター止めるよ
        digitalWrite(pah[i],LOW);
        analogWrite(ena[i],0);
      }
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


void moter(double ang,double ac_val,int go_flag){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double val = val_max;        //モーターの値の上限値
  double mval_x = cos(radians(ang));  //進みたいベクトルのx成分
  double mval_y = sin(radians(ang));  //進みたいベクトルのy成分

  float back_val = 2;
  
  val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    if(go_flag == 0){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    }
    
    else if(go_flag == 1){  //左のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * back_val;
    }
    else if(go_flag == 2){  //右のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * -back_val;
    }
    else if(go_flag == 3){  //前のストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * -back_val;
    }
    else if(go_flag == 4){  //後ろのストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * back_val;
    }
    else if(go_flag == 5){  //ストップ
      Mval[i] = -mSin[i] * -back_val + mCos[i] * back_val;
    }
    else if(go_flag == 6){
      Mval[i] = -mSin[i] * -back_val + mCos[i] * -back_val;
    }
    else if(go_flag == 7){
      Mval[i] = -mSin[i] * back_val + mCos[i] * back_val;
    }
    else if(go_flag == 8){
      Mval[i] = -mSin[i] * back_val + mCos[i] * -back_val;
    }
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }
  
  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] / g * val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)

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