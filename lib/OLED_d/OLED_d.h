#pragma once


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>
#include <timer.h>
#include<line.h>
#include<ball.h>
#include<ac.h>
#include<motor_d.h>
#include<Cam.h>

extern AC ac;
extern BALL ball;
extern LINE line;
extern motor_deffence MOTOR;
extern Cam cam_back;

/*------------------------------------------------------------------------------------------------------------*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16


class oled_deffence{
    public:
    Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    const int bluetooth = 11;
    const int Encoder_A = 17;  //エンコーダーのピン番号
    const int Encoder_B = 16;  //エンコーダーのピン番号
    Encoder myEnc = Encoder(17, 16);  //エンコーダのピン番号
    timer timer_OLED;
    long oldPosition  = -999;  //エンコーダのオールドポジの初期化
    long new_encVal = 0;  //エンコーダーの現在値を示す変数
    long old_encVal = 0;  //エンコーダーの過去値を示す変数  
    unsigned int address = 0x00;  //EEPROMのアドレス
    int toogle = 0;  //トグルスイッチの値を記録（トグルを引くときに使う）
    int goDir;  //loop関数ないしか使えないangle go_ang.degressの値をぶち込んでグローバルに使うために作った税
    oled_deffence();
    void setup();
    void OLED();
    void startOLED();
    int Button_selectCF = 0;  //コートの方向を決めるときに特殊なことをするので、セレクト変数を変えときますぜよ
    int RA_size;
    int val_max;
    int OutB_flag;

    const int Tact_Switch = 15;
    const int Toggle_Switch = 14;  //スイッチのピン番号
};
/*------------------------------------------------------------------------------------------------------------*/