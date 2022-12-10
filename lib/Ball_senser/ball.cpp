#include "ball.h"




#define pi 3.1415926535897932384


void Ball::getBallposition(){  //ボールの位置を極座標系で取得
  double Bfar = 0;  //グローバル変数に戻す前の変数(直接代入するのはは何となく不安)
  double Bang = 0;  //グローバル変数に戻す前の変数
  int Bval[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //ボールの値

  double Bfar_x = 0; //ボールの距離のx成分
  double Bfar_y = 0; //ボールの距離のy成分

  double Bfar_y_all = 0;
  int low_cou = 0;    //一回ボールの値を集計して値がch_num以下だったセンサーの数
  double low_all = 0; //最新100回のボールの値を集計して値がch_num以下だったセンサーの数


  for(int ch_cou = 0; ch_cou < ch_num; ch_cou++){   //ch_num回センサーの値を取得
    for(int sen_num = 0; sen_num < 16; sen_num++){  //16個のセンサーの値を取得

      if(digitalReadFast(ball_sen[sen_num]) == 0){
        Bval[sen_num]++;    //センサーの値が0だったらBvalに1を足す(遠くにあるほど値が大きくなる)
      }

    }
  }

  for(int i = 0; i < 16; i++){   //値を集計するところ
    Bfar_x += Bval[i] * Cos[i];  //ボールの距離のx成分を抽出
    Bfar_y += Bval[i] * Sin[i];  //ボールの距離のy成分を抽出

    if(Bval[i] < sen_lowest){
      low_cou++;  //値がsen_lowest以下だったセンサーの数をカウント
    }
  }

  low_acc[cou % MAX] = low_cou;  //最新の値を配列に入れる(リングバッファ使ってる)
  far_y_acc[cou % MAX] = Bfar_y;
  cou++;  //この関数の呼び出し回数をカウント

  for(int i = 0; i < MAX; i++){
    low_all += low_acc[i];  //値がsen_lowest以下だったセンサーの数を合計
    Bfar_y_all += far_y_acc[i]; 
  }

  Bfar = low_all / (cou < MAX ? cou : MAX) - 2;  //ボールの距離を計算
  Bang = atan2(Bfar_y,Bfar_x) * 180 / pi;    //ボールの角度を計算(atan2はラジアンで返すので角度に変換)  

  ang = Bang;
  far = Bfar;
  far_x = Bfar_x;
  far_y = Bfar_y_all / (MAX * 100);
}




void Ball::print(){  //ボールの位置を表示
  Serial.print(" ボールの距離 : ");
  Serial.print(far);
  Serial.print(" ボールの角度 : ");
  Serial.print(ang);
  Serial.print(" ボールの横軸での距離 : ");
  Serial.print(far_y);
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