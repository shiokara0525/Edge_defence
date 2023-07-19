#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<motor_d.h>
#include<OLED_d.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>

/*---------------------------------------------------ディスプレイの宣言-----------------------------------------------------------------------------------*/

int RA_size;
int val_max;
int OutB_flag = 999;
const int Tact_Switch = 15;
const int Toggle_Switch = 14;  //スイッチのピン番号
float goDir = 0;
int lFla;

void OLED_moving();

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

BALL ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
motor_deffence MOTER;
oled_deffence OLED;
timer Timer_dog;  //ここ限定のタイマーだよ(何秒前進するかとか決めるよ)
timer Timer_sentor;

int A = 0;  //どのチャプターに移動するかを決める変数

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int A_sentor = 0;
int B_sentor = 999;

const int stop_range = 10;
/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial8.begin(57600);
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)(ただ通信を成功させときたいだけ)
  line.setup();  //ラインとかのセットアップ
  OLED.OLED();
  A = 10;
}




void loop(){
  double AC_val = 0;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);  //進む角度だぜいえいえ
  int goval = val_max;  //進む速度だぜいえいえ
  int stop_flag = 0;  //ライン踏んでるときどんな進み方をするか決めるぜ
  int Line_flag = 0;  //ライン踏んでるか踏んでないかを判定する変数
  int ac_flag = 0;

  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val(); //姿勢制御の値入手
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    lFla = Line_flag;
    A = 30;
    if(Line_flag == 0){  //ライン踏んでないとき(ラインを踏んでいるときはAを30にしているのでここには来ない)
      if(OutB_flag == 1 || OutB_flag == 0){
        A = 15;  //アウトオブバウンズから復帰するとき
      }
      else{
        A = 20;  //がんばってラインに戻るとき
      }
    }
  }

  if(A == 15){  //アウトオブバウンズから復帰するとき
    if(OutB_flag == 0){
      go_ang = -165.0;
    }
    else if(OutB_flag == 1){
      go_ang = 165.0;
    }
    else{
      go_ang = 179.9;
    }

    while(1){
      int ac_val = ac.getAC_val();
      goDir = go_ang.degree;
      OLED_moving();
      MOTER.moveMoter_0(go_ang,80,ac_val);  //後ろに下がるよ
      int line_flag = line.getLINE_Vec();

      if(line_flag == 1){  //ラインに当たったら抜けるよ
        Timer_sentor.reset();
        MOTER.moter_0();
        delay(50);
        if(line_flag == 1){
          break;
        }
      }

      if(digitalReadFast(Tact_Switch) == LOW){
        A = 10;
      }
    }
    OutB_flag = 999;
    A = 30;
  }

  if(A == 20){  //ラインに戻るとき
    go_ang = line.Lvec_Dir;
    A = 50;
  }

  if(A == 30){  //ライン踏んでるとき(ライントレース)
    int go_flag = 0;
    double go_border[2];  //ボールの角度によって進む方向を変えるためのボーダーの変数(ラインに対して垂直な直線で進む角度の区分を分けるイメージ)
    angle balldir(ball.ang,true);  //ボールの角度を入れるオブジェクト
    if(2 < line.Lrange_num){
      line.Lvec_Dir = 90;
    }

    if(line.Lvec_Dir < 0){
      go_border[0] = line.Lvec_Dir;
      go_border[1] = line.Lvec_Dir + 180;
    }
    else{
      go_border[0] = line.Lvec_Dir - 180;
      go_border[1] = line.Lvec_Dir;
    }

    balldir.to_range(go_border[0],false);  //ボールの角度をボーダーの範囲に収める(go_border[0] ~ go_border[1]+180)

    if(go_border[0] < balldir.degree && balldir.degree < go_border[1]){  //ボールの角度を区分分けする
      go_flag = 0;
    }
    else{
      go_flag = 1;
    }

    go_ang = go_border[go_flag] + 90;  //進む角度決定
    go_ang.to_range(180,true);  //進む角度を-180 ~ 180の範囲に収める


    if(160 < abs(go_ang.degree)){       //進む角度が真後ろにあるとき
      goval = 0;
      stop_flag = 1;
    }
    else if(120 < abs(go_ang.degree)){  //進む角度が後ろめな時
      goval = 50;
      MOTER.line_val = 2;
    }
    else if(abs(go_ang.degree) < 60){  //前めに進むとき
      MOTER.line_val = 2;
    }
    else{                              //横に進むとき
      for(int i = 0; i < 2; i++){
        if((go_border[i] - stop_range < ball.ang && ball.ang < go_border[i] + stop_range)){  //正面方向にボールがあったら停止するよ
          ac_flag = 1;
        }
      }
      MOTER.line_val = 0.8;
    }
    A = 50;

    if(abs(ball.ang) < 30){  //前にボールがあるとき
      A_sentor = 1;
      if(A_sentor != B_sentor){
        B_sentor = A_sentor;
        Timer_sentor.reset();  //ここに入ったらタイマースタートするよ
      }

      if(5000 < Timer_sentor.read_ms()){
        A = 40;  //7秒続けてボールが前にあったら前進するよ
      }
    }
    else{
      A_sentor = 0;
      if(A_sentor != B_sentor){
        B_sentor = A_sentor;
        Timer_sentor.reset();  //前から外れたらリセットするよ
      }
    }
  }

  if(A == 40){  //ボールが前にあるから前進するよ
    goval = 100;

    if(abs(ball.ang) < 25){
      if(ball.ang < -15){
        OutB_flag = 1;
      }
      else if(15 < ball.ang){
        OutB_flag = 2;
      }
      else{
        OutB_flag = 999;
      }
      Timer_dog.reset();

      while(Timer_dog.read_ms() < 700){  //0.9秒前進するよ
        float ac_val = ac.getAC_val();
        ball.getBallposition();

        go_ang = ball.ang;
        MOTER.moveMoter_0(go_ang,goval,ac_val);
        if(25 < abs(ball.ang)){  //前にボールがなくなったらすぐ戻るよ
          break;
        }
      }
    }
    goval = 80;
    A = 15;
  }

  if(A == 50){
    if(stop_flag == 0){
      if(ac_flag == 0){
        MOTER.moveMoter_l(go_ang,goval,AC_val,line);
      }
      else{
        MOTER.moter_ac(AC_val);
      }
    }
    else{
      MOTER.moter_0();
    }
    OLED_moving();

    A = 10;
    goDir = go_ang.degree;
  }

  if(digitalRead(Tact_Switch) == LOW){
    MOTER.moter_0();
    OLED.toogle = digitalRead(Toggle_Switch);
    OLED.OLED();
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/

void serialEvent8(){
  int n;
  int x,y;
  word revBuf_word[6];
  byte revBuf_byte[6];
  //受信データ数が、一定時間同じであれば、受信完了としてデータ読み出しを開始処理を開始する。
  //受信データあり ※6バイト以上になるまでまつ
  if(Serial8.available()>= 6){
    //---------------------------
    //受信データをバッファに格納
    //---------------------------
    n = 0;
    while(Serial8.available()>0 ){ //受信データがなくなるまで読み続ける
      //6バイト目まではデータを格納、それ以上は不要なデータであるため捨てる。
      if(n < 6){
        revBuf_byte[n] = Serial8.read();   //revBuf_byte[n] = Serial2.read();
      }
      else{
        Serial8.read(); //Serial2.read();  //読みだすのみで格納しない。
      }
      n++;
    }
    //---------------------------
    //データの中身を確認
    //---------------------------
    //データの先頭、終了コードあることを確認
    if((revBuf_byte[0] == 0xFF ) && ( revBuf_byte[5] == 0xAA )){
    //いったんWORD型（16bitデータ）としてから、int16_tとする。
      revBuf_word[0] = (uint16_t(revBuf_byte[1])<< 8);//上位8ビットをbyteから、Wordに型変換して格納　上位桁にするため8ビットシフト
      revBuf_word[1] = uint16_t(revBuf_byte[2]);//下位8ビットをbyteから、Wordに型変換して格納      
      x = int16_t(revBuf_word[0]|revBuf_word[1]);//上位8ビット、下位ビットを合成（ビットのORを取ることで格納する。）
      // ※int ではなく　int16_t　にすることが必要。intだけだと、32ビットのintと解釈されマイナス値がマイナスとみなされなくなる、int16_tは、16ビット指定の整数型変換
      revBuf_word[2] = (uint16_t(revBuf_byte[3])<< 8);//上位8ビットをbyteから、Wordに型変換して格納　上位桁にするため8ビットシフト
      revBuf_word[3] = uint16_t(revBuf_byte[4]);//下位8ビットをbyteから、Wordに型変換して格納      
      y = int16_t(revBuf_word[2]|revBuf_word[3]);//上位8ビット、下位ビットを合成（ビットのORを取ることで格納する。）
      // ※int ではなく　int16_t　にすることが必要。intだけだと、32ビットのintと解釈されマイナス値がマイナスとみなされなくなる、int16_tは、16ビット指定の整数型変換
      
      x = ball.ball_x.demandAve(x);
      y = ball.ball_y.demandAve(y);
    }
    else{
      // printf("ERR_REV");
    }
  }
}




void OLED_moving(){
  //OLEDの初期化
  OLED.display.display();
  OLED.display.clearDisplay();

  //テキストサイズと色の設定
  OLED.display.setTextSize(1);
  OLED.display.setTextColor(WHITE);
  
  OLED.display.setCursor(0,0);  //1列目
  OLED.display.println("Bang");  //現在向いてる角度
  OLED.display.setCursor(30,0);
  OLED.display.println(":");
  OLED.display.setCursor(36,0);
  OLED.display.println(ball.ang);    //現在向いてる角度を表示

  OLED.display.setCursor(0,10);  //2列目
  OLED.display.println("goang");  //この中に変数名を入力
  OLED.display.setCursor(30,10);
  OLED.display.println(":");
  OLED.display.setCursor(36,10);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,20); //3列目
  OLED.display.println("C_x");  //この中に変数名を入力
  OLED.display.setCursor(30,20);
  OLED.display.println(":");
  OLED.display.setCursor(36,20);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,30); //4列目
  OLED.display.println("bcf");  //この中に変数名を入力
  OLED.display.setCursor(30,30);
  OLED.display.println(":");
  OLED.display.setCursor(36,30);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,40); //5列目
  OLED.display.println("LF");  //この中に変数名を入力
  OLED.display.setCursor(30,40);
  OLED.display.println(":");
  OLED.display.setCursor(36,40);
  OLED.display.println(line.LINE_on);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,50); //6列目
  OLED.display.println("A");  //この中に変数名を入力
  OLED.display.setCursor(30,50);
  OLED.display.println(":");
  OLED.display.setCursor(36,50);
  OLED.display.println(A);    //この中に知りたい変数を入力
}