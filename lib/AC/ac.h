#pragma once


#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SPIDevice.h>
#include <MA.h>


class AC{
public:
  double getAC_val(); //姿勢制御用の値返す関数
  float getnowdir();
  int flag = 0;  //モーターが突然反転しないようにするやつ
  void print();  //姿勢制御関連のやつを表示
  void setup();  //姿勢制御のセットアップ
  void setup_2();
  double dir = 0;  //現Fの方向
  double dir_target;  //目標の方向(正面方向)

private:
  double nowTime = 0;  //関数で見た時の時間
  double time_old = 0; //1F前の時間

  
  double val = 0;  //姿勢制御の値
  double val_old = 0;  //1F前の姿勢制御の値

  double kkp_old = 0;  //前Fの方向

  const float kp = 3;  //比例制御の比例定数
  const float kd = 5;  //微分制御の定数
  const float ki = 0.001;

  MA kki;

  sensors_event_t event;  //ジャイロのいろんな値入れるやつ
  Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
};