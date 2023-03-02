#pragma once


#include <Arduino.h>
#include <timer.h>

#define MAX 20

class Ball{
public:
  double far;  //ボールまでの距離
  double ang;  //ボールまでの角度
  double far_x;
  double far_y;
  int getBallposition();  //ボールの位置を取得
  void print();  //ボールの距離と角度を表示
  void setup();  //セットアップ

private:
  int cou = 0;  //ボールを見た回数(getBallpositionに入った回数をカウントするやつ)
  double low_acc[MAX];  //ボールまでの距離(最新100回分をはかるように、円環バッファを使う)
  double far_x_acc[MAX];
  double far_y_acc[MAX];
  double Sin[16]; //sinの値(22.5°ずつ)
  double Cos[16]; //cosの値(22.5°ずつ)

  const int ch_num = 1000; //センサーの値取る回数
  const int sen_lowest = 200; //センサーがボールを見てないと判断する値
  const int ball_sen[16] ={
  9,10,11,12,13,34,35,36,37,38,39,40,41,6,7,8};
  timer timer_ball;
};