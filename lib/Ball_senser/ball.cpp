#include "ball.h"




#define pi 3.1415926535897932384


int Ball::getBallposition(){  //ボールの位置を極座標系で取得
  double Bfar_y_all = 0;
  double Bfar_x_all = 0;
  double Bfar = 0;  //グローバル変数に戻す前の変数(直接代入するのはは何となく不安)
  double Bang = 0;  //グローバル変数に戻す前の変数
  int Bmax_num = 0;
  int Bval[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //ボールの値

  double Bfar_x = 0; //ボールの距離のx成分
  double Bfar_y = 0; //ボールの距離のy成分

  int low_cou = 0;    //一回ボールの値を集計して値がch_num以下だったセンサーの数
  double low_all = 0; //最新100回のボールの値を集計して値がch_num以下だったセンサーの数

  timer_ball.reset();
  while(timer_ball.read_us() < 833){
    for(int sen_num = 0; sen_num < 16; sen_num++){  //16個のセンサーの値を取得

      if(digitalReadFast(ball_sen[sen_num]) == 0){
        Bval[sen_num]++;    //センサーの値が0だったらBvalに1を足す(遠くにあるほど値が大きくなる)
      }

    }
  }
  int Bval_max = 0;

  for(int i = 0; i < 16; i++){   //値を集計するところ
    Bfar_x += Bval[i] * Cos[i];  //ボールの距離のx成分を抽出
    Bfar_y += Bval[i] * Sin[i];  //ボールの距離のy成分を抽出
    if(Bval_max < Bval[i]){
      Bval_max = Bval[i];
    }
  }
  Bval_max -= 500;
  for(int i = 0; i < 16; i++){
    if(Bval[i] < Bval_max){
      low_cou++;
    }
  }

  low_acc[cou % MAX] = low_cou;  //最新の値を配列に入れる(リングバッファ使ってる)
  far_y_acc[cou % MAX] = Bfar_y;
  far_x_acc[cou % MAX] = Bfar_x;
  cou++;  //この関数の呼び出し回数をカウント

  for(int i = 0; i < MAX; i++){
    low_all += low_acc[i];  //値がsen_lowest以下だったセンサーの数を合計
    Bfar_y_all += far_y_acc[i]; 
    Bfar_x_all += far_x_acc[i];
  }

  Bang = degrees(atan2(Bfar_y,Bfar_x));    //ボールの角度を計算(atan2はラジアンで返すので角度に変換)  

  ang = Bang;
  far_x = Bfar_x_all * 0.005;
  far_y = Bfar_y_all * 0.005;
  Bfar = (2.5 * low_all) - 300;
  if(Bfar < 40){
    Bfar = 40;
  }
  far = Bfar;
  if(far_x == 0 && far_y == 0){  //ボールを見失ったとき止まっとく
    return 0;
  }
  else{  //ボールを見てたら返り値
    return 1;
  }
}




void Ball::print(){  //ボールの位置を表示
  Serial.print(" ボールの距離 : ");
  Serial.print(far);
  Serial.print(" ボールの角度 : ");
  Serial.print(ang);
}




void Ball::setup(){  //ボール関連のセットアップ
  for(int i = 0; i < 16; i++){
    pinMode(ball_sen[i],INPUT);
    Cos[i] = cos(radians(i * 22.5));  //cosの22.5度ごとの値を配列に入れる
    Sin[i] = sin(radians(i * 22.5));  //sinの22.5度ごとの値を配列に入れる
  }
  for(int i = 0; i < MAX; i++){
    low_acc[i] = 0;  //配列の中の値を0にする
  }
}