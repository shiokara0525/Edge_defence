#include<ac.h>



double AC::getAC_val(){  //姿勢制御の値返す関数
  double kkp = 0;  //比例制御の値
  double kkd = 0;  //積分制御の値
    

  bno.getEvent(&event);  //方向チェック
  
  dir = event.orientation.x - dir_target;  //現在の方向を取得
  
  if(dir > 180){
    dir -= 360;  //方向を0~360から-180~180に変換
  }

  kkp = -dir;  //比例制御の値を計算
  kkd = -(dir - dir_old);  //微分制御の値を計算
  
  val = kkp * kp + kkd * kd;  //最終的に返す値を計算

  if(abs(dir - dir_old) > 350){
    flag = 1;  //モーターが急に反転してストップするのを防止するフラグ
  }
  

  dir_old = dir;  //前Fの方向を更新

  return val;  //値返す
}




void AC::print(){  //現在の角度、正面方向、姿勢制御の最終的な値を表示
  Serial.print(" 角度 : ");
  Serial.print(dir);
  Serial.print(" 正面方向 : ");
  Serial.print(dir_target);
  Serial.print(" 最終的に出たやつ : ");
  Serial.print(val);
}




void AC::setup(){  //セットアップ
  bno.begin();
  bno.getEvent(&event);  //方向入手
  delay(100);
  bno.getEvent(&event);

  if(event.orientation.x > 180){
    event.orientation.x -= 360;  //方向を0~360から-180~180に変換
  }

  dir_target = event.orientation.x;  //正面方向決定
}

void AC::setup_2(){
  bno.getEvent(&event);

   if(event.orientation.x > 180){
    event.orientation.x -= 360;  //方向を0~360から-180~180に変換
  }
  dir_target = event.orientation.x;
}