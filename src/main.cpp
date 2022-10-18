#include <Arduino.h>

const int ball_sen[16] ={
  9,10,11,12,13,34,35,36,37,38,39,40,41,6,7,8
}; //ピン番号

double Sin[16]; //sinの値(22.5°ずつ)
double Cos[16]; //cosの値(22.5°ずつ)

const double pi = 3.141592653589793238; //円周率
int ball_cou = 0; //一回ボールの値を集計して値がch_num以下だったセンサーの数
int ch_num = 100; //ボールを見てないと判断する最低値
int ave_cou = 0; //ボールの値を集計した回数
double ball_ave = 0; //ボールを見ていなかったセンサーの数の平均
int ball_val_fla[16]; //1000回センサーの値を見た結果


int read_num = 1000; //1Fでボールの値を見る回数
double ball_far_x = 0; //ボールのx軸上での距離
double ball_far_y = 0; //ボールのy軸上での距離
double ball_far = 0; //ボールとの距離
double ball_ang = 0; //ボールの角度



void setup() {
  for(int i = 0; i < 16; i++){
    pinMode(ball_sen[i],INPUT);
    Cos[i] = cos(i * 22.5 / 180 * pi); //cosの定数値
    Sin[i] = sin(i * 22.5 / 180 * pi); //sinの定数値
  }
  /*Serial.begin(9600);*/
}


void loop() {
  ball_far_x = 0; 
  ball_far_y = 0;
  ball_cou = 0;

  for(int i = 0; i < 16; i++){
    ball_val_fla[i] = 0;
  }
  //ここまでは1Fごとの値のリセット

  for(int read_cou = 0; read_cou < read_num; read_cou++){ //ボールセンサーの値を測る(read_num回)
    for(int sen_num = 0; sen_num < 16; sen_num++){ //ボールセンサーの値を取得する

      if(digitalReadFast(ball_sen[sen_num]) == 0){
        ball_val_fla[sen_num] += 1; //ボールの値を加算していく
      }

    }
  }

  //ここまではセンサーの値取るところ

  for(int i = 0; i < 16; i++){
    ball_far_x += (ball_val_fla[i] * 0.22 - 54) * Cos[i]; //ベクトルをx成分とy成分に分解
    ball_far_y += (ball_val_fla[i] * 0.22 - 54) * Sin[i];
    if(ball_val_fla[i] < ch_num){
      ball_cou++; //ボールを見ていない(値がch_num以下である)センサーの数(1集計分)を数える
    }
  }


  ball_ave += ball_cou; //ボールを見ていないセンサーの数を記録
  ave_cou++; //記録回数を追加


  if(ave_cou > 100){
    ball_far = ball_ave / 100; //ボールを見ていなかったセンサーの数の平均値をとる
    ave_cou = 0; 
    ball_ave = 0; //リセット
  }

  ball_ang = atan2(ball_far_y,ball_far_x);  //逆正接でボールとの角度を出す
  ball_ang = ball_ang / pi * 180; //度数法に直す

  /*Serial.println(ball_far);*/
}