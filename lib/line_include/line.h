#pragma once

#include <timer.h>
#include <Arduino.h>
#include <angle.h>
#define Long 5

class LINE{
public:
    int getLINE_Vec(); //ラインセンサのベクトル(距離,角度)を取得する関数
    void print();
    void setup();
    int switchLineflag(angle);
    float decideGoang(angle,int);

    double dis; //ラインのベクトルの長さ
    double ang; //ラインの和のベクトルの角度
    double ang_old = 0;
    float dis_X;
    float dis_Y;
    int num;
 
    int LINE_on; //ラインがロボットの下になかったら0,あったら1にする
    const int LINE_light = 27; //ラインセンサのLEDを光らせるかの制御をするためのピン
    int LINE_Level = 735; //ラインの閾値
    double Lrad; //ラインの角度のラジアン
    double Lvec_X = 0; //ラインセンサのX座標の和のベクトル
    double Lvec_Y = 0; //ラインセンサのY座標の和のベクトル
    int line_flag = 0;

private:
    const int LINE_1 = 23; //ラインセンサIC1の出力を読み取るためのピン
    const int LINE_2 = 22; //ラインセンサIC2の出力を読み取るためのピン
    const int LINE_3 = 21; //ラインセンサIC3の出力を読み取るためのピン
    const int LINE_A = 26; //ラインセンサのマルチプレクサを制御するためのピンA
    const int LINE_B = 25; //ラインセンサのマルチプレクサを制御するためのピンB
    const int LINE_C = 24; //ラインセンサのマルチプレクサを制御するためのピンC
    

    const int Lselect[3] = {LINE_A, LINE_B, LINE_C}; //ラインセンサのマルチプレクサを制御するためのピンを配列に格納
    const int Lread[3] = {LINE_1, LINE_2, LINE_3}; //ラインセンサICの出力を読み取るためのピンを配列に格納

    

    double Lsencer_Dir[24]; //ラインセンサの角度
    double ele_X[27]; //ラインセンサのX座標
    double ele_Y[27]; //ラインセンサのY座標
    double Lvec_X_old = 0; //ラインベクトルの初期値を記録する
    double Lvec_Y_old = 0; //ラインベクトルの初期値を記録する
    timer timer1; //タイマーの宣言
};