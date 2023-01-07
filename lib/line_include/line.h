#pragma once

#include <timer.h>
#include <Arduino.h>


class LINE{
public:
    int getLINE_Vec(); //ラインセンサのベクトル(距離,角度)を取得する関数
    void print();
    void setup();

    double Lvec_Long; //ラインのベクトルの長さ
    double Lvec_Dir; //ラインの和のベクトルの角度
    double Lvec_Long_move; //ラインのベクトルの長さの移動量
    double Lvec_Dir_move; //ラインの和のベクトルの角度の移動量
    double LP_X; //ラインのX成分のPゲイン
    double LP_Y; //ラインのY成分のPゲイン
    double LP_X_old; //ラインのX成分の過去のPゲイン
    double LP_Y_old; //ラインのY成分の過去のPゲイン
    double LD_X; //ラインのX成分のDゲイン
    double LD_Y; //ラインのY成分のDゲイン

    int LINE_on; //ラインがロボットの下になかったら0,あったら1にする
    double Lvec_X_target = 0.7; //ラインの目標ベクトルのX座標（ゴールキーパーの時で、PD制御で使う）

    const float kp = 15;
    const float kd = 10;
    const int LINE_light = 27; //ラインセンサのLEDを光らせるかの制御をするためのピン

private:
    const int LINE_1 = 23; //ラインセンサIC1の出力を読み取るためのピン
    const int LINE_2 = 22; //ラインセンサIC2の出力を読み取るためのピン
    const int LINE_3 = 21; //ラインセンサIC3の出力を読み取るためのピン
    const int LINE_A = 26; //ラインセンサのマルチプレクサを制御するためのピンA
    const int LINE_B = 25; //ラインセンサのマルチプレクサを制御するためのピンB
    const int LINE_C = 24; //ラインセンサのマルチプレクサを制御するためのピンC

    const int Lselect[3] = {LINE_A, LINE_B, LINE_C}; //ラインセンサのマルチプレクサを制御するためのピンを配列に格納
    const int Lread[3] = {LINE_1, LINE_2, LINE_3}; //ラインセンサICの出力を読み取るためのピンを配列に格納
    double PI = 3.1415926535897932384626; //円周率

    double Lrad; //ラインの角度のラジアン

    int LINE_Level = 600; //ラインの閾値

    double Lsencer_Dir[24]; //ラインセンサの角度
    double LINE_X[27]; //ラインセンサのX座標
    double LINE_Y[27]; //ラインセンサのY座標
    double Lvec_X_old = 0; //ラインベクトルの初期値を記録する
    double Lvec_Y_old = 0; //ラインベクトルの初期値を記録する
    
    double Lvec_Y_target = 0; //ラインの目標ベクトルのY座標（ゴールキーパーの時で、PD制御で使う）
    
    timer timer1; //タイマーの宣言
};