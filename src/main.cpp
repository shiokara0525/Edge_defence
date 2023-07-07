#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<moter.h>
#include<US.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>

/*---------------------------------------------------ディスプレイの宣言-----------------------------------------------------------------------------------*/


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

const int Encoder_A = 17;  //エンコーダーのピン番号
const int Encoder_B = 16;  //エンコーダーのピン番号
Encoder myEnc(17, 16);  //エンコーダのピン番号
long oldPosition  = -999;  //エンコーダのオールドポジの初期化
long new_encVal = 0;  //エンコーダーの現在値を示す変数
long old_encVal = 0;  //エンコーダーの過去値を示す変数
unsigned int address = 0x00;  //EEPROMのアドレス
int toogle = 0;  //トグルスイッチの値を記録（トグルを引くときに使う）
const int Toggle_Switch = 14;  //スイッチのピン番号
void OLED_moving();

int lFla = 0;
int gb[2];
/*-------------------------------------------------------ピン番号＆定数---------------------------------------------------------------------*/

const int Tact_Switch = 15;  //スイッチのピン番号 
const int pingPin = 32;
const double pi = 3.1415926535897932384;  //円周率

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

BALL ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer_sentor;
moter MOTER;
us US;
timer timer_OLED; //タイマーの宣言(OLED用)
timer Timer_dog;  //ここ限定のタイマーだよ(何秒前進するかとか決めるよ)
timer timer_BA;

int A = 0;  //どのチャプターに移動するかを決める変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか
//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int A_sentor = 0;
int B_sentor = 999;

int A_BA = 0;
int B_BA = 0;

const int stop_range[2]= {10,20};
double goval_a;

int side_flag = 0;
int OutB_flag = 0;  //ロボットが復帰するときに右側におかれるか左側におかれるかを表示する変数(0が右 1が左 999がなし)(デフォルトは999)

void OLED_setup();
void OLED();

int val_max = 120;         //モーターの出力の最大値
float RA_size = 80;
float goDir = 0;
int A20_flag = 0;
int Back_flag = 0;
/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial8.begin(57600);
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)(ただ通信を成功させときたいだけ)
  line.setup();  //ラインとかのセットアップ

  OLED_setup();
  OLED();
  goval_a = val_max / (stop_range[1] - stop_range[0]);
  A = 10;
}




void loop(){
  double AC_val = 0;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);  //進む角度だぜいえいえ
  int goval = val_max;  //進む速度だぜいえいえ
  int stop_flag = 5;  //ライン踏んでるときどんな進み方をするか決めるぜ
  int Line_flag = 0;  //ライン踏んでるか踏んでないかを判定する変数
  int ac_flag = 0;
  A20_flag = 0;

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
      //OLED_moving();
      MOTER.moveMoter(go_ang,100,ac_val,0,line);  //後ろに下がるよ
      int line_flag = line.getLINE_Vec();

      if(line_flag == 1){  //ラインに当たったら抜けるよ
        break;
      }

      if(digitalReadFast(Tact_Switch) == LOW){
        A = 10;
      }
    }
    Timer_sentor.reset();

    OutB_flag = 999;
    A = 30;
  }

  if(A == 20){  //ラインに戻るとき
    go_ang = line.Lvec_Dir;
    A20_flag = 1;
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


    if(100 < abs(go_ang.degree)){
      goval = 0;
      stop_flag = 999;
      B_BA = 0;
    }
    else if(120 < abs(go_ang.degree)){
      goval = 50;
      MOTER.line_val = 3;
      B_BA = 0;
    }
    else if(abs(go_ang.degree) < 60){
      MOTER.line_val = 3;
      B_BA = 0;
    }
    else{
      if(B_BA != 1){
        B_BA = 1;
        timer_BA.reset();
      }
      for(int i = 0; i < 2; i++){
        if((go_border[i] - stop_range[0] < ball.ang && ball.ang < go_border[i] + stop_range[0])){  //正面方向にボールがあったら停止するよ
          ac_flag = 1;
        }
      }
      MOTER.line_val = 0.75;
      if(100 < timer_BA.read_ms()){
        goval -= 35;
      }
    }  //進む方向から、スピードとかを決めてるよ
    A = 50;
    gb[0] = go_border[0];
    gb[1] = go_border[1];

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
      if(ball.ang < 0){
        OutB_flag = 1;
      }
      else{
        OutB_flag = 2;
      }
      Timer_dog.reset();

      while(Timer_dog.read_ms() < 700){  //0.9秒前進するよ
        float ac_val = ac.getAC_val();
        ball.getBallposition();
        go_ang = ball.ang;
        MOTER.moveMoter(go_ang,goval,ac_val,0,line);
        //OLED_moving();
        if(25 < abs(ball.ang)){  //前にボールがなくなったらすぐ戻るよ
          break;
        }
      }
    }
    goval = 80;
    OutB_flag = 999;
    A = 15;
  }

  if(A == 50){
    if(ac_flag == 0){
      MOTER.moveMoter(go_ang,goval,AC_val,stop_flag,line);
    }
    else{
      MOTER.moter_ac(AC_val);
    }
    OLED_moving();
    
    A = 10;
    goDir = go_ang.degree;
  }

  if(digitalRead(Tact_Switch) == LOW){
    MOTER.moter_0();
    toogle = digitalRead(Toggle_Switch);
    OLED();
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


void OLED_setup(){
  EEPROM.get(address,line.LINE_Level);//EEPROMから読み出し
  address += sizeof(line.LINE_Level);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,RA_size);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  address += sizeof(RA_size);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,val_max);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}




void OLED() {
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  timer_OLED.reset(); //タイマーのリセット(OLED用)
  toogle = digitalRead(Toggle_Switch);

  OutB_flag = 999;  //ロボットが復帰するときに右側におかれるか左側におかれるかを表示する変数(0が右 1が左 999がなし)(デフォルトは999)
  int A_OLED = 0;
  int B_OLED = 999;  //ステート初期化のための変数
  int aa = 0;  //タクトスイッチのルーレット状態防止用変数

  int flash_OLED = 0;  //ディスプレイの中で白黒点滅させたいときにつかう
  int OLED_select = 1;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によって選択画面の表示が変化する)
  int Button_select = 0;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によってexitかnextかが決まる)

  int OLED_ball_x = 0;
  int OLED_ball_y = 0;

  int line_x = 0;
  int line_y = 0;

  int Ax = 0;
  int Ay = 0;
  int Bx = 0;
  int By = 0;

  int OLED_line_ax = 0;
  int OLED_line_ay = 0;
  int OLED_line_bx = 0;
  int OLED_line_by = 0;

  float b = 0;
  float La = 0;
  float Lb = 0;
  float Lc = 0;
  
  while(1){
    if(timer_OLED.read_ms() > 500) //0.5秒ごとに実行(OLEDにかかれてある文字を点滅させるときにこの周期で点滅させる)
    {
      if(flash_OLED == 0){
        flash_OLED = 1;
      }
      else{
        flash_OLED = 0;
      }
      timer_OLED.reset(); //タイマーのリセット(OLED用)
    }


    if(A_OLED == 0)  //メインメニュー
    {
      if(A_OLED != B_OLED)  //ステートが変わったときのみ実行(初期化)
      {
        OLED_select = 1;  //選択画面をデフォルトにする
        B_OLED = A_OLED;
      }

      //OLEDの初期化
      display.display();
      display.clearDisplay();

      //選択画面だということをしらせる言葉を表示
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Hi! bro!");
      display.setCursor(0,10);
      display.println("What's up?");

      //文字と選択画面の境目の横線を表示
      display.drawLine(0, 21, 128, 21, WHITE);

      //選択画面の表示
      if(OLED_select == 1)  //STARTを選択しているとき
      {
        //START値を調整
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(0,35);
        display.println("START");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(90,35);
        display.println("Set");
        display.setCursor(88,45);
        display.println("Line");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 10;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
      else if(OLED_select == 2)  //Set Lineを選択しているとき
      {
        //Line値を調整
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(12,27);
        display.println("Set");
        display.setCursor(6,44);
        display.println("Line");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(85,35);
        display.println("Check");
        display.setCursor(88,45);
        display.println("Line");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 20;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
      else if(OLED_select == 3)  //Check Lineを選択しているとき
      {
        //Check Lineの文字設定
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(0,27);
        display.println("Check");
        display.setCursor(6,44);
        display.println("Line");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(90,35);
        display.println("Set");
        display.setCursor(94,45);
        display.println("RA");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 30;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
      else if(OLED_select == 4)  //Set RA（回り込みの大きさ）を選択しているとき
      {
        //回り込みの大きさを調整
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(12,27);
        display.println("Set");
        display.setCursor(18,44);
        display.println("RA");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(85,35);
        display.println("Check");
        display.setCursor(88,45);
        display.println("Ball");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 40;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
      else if(OLED_select == 5)  //Check Ballを選択しているとき
      {
        //Check Ballの文字設定
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(0,27);
        display.println("Check");
        display.setCursor(6,44);
        display.println("Ball");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(90,35);
        display.println("Set");
        display.setCursor(88,44);
        display.println("Motar");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 50;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
      else if(OLED_select == 6)  //Set Motarを選択しているとき
      {
        //Motar値を調整
        display.setTextSize(2);
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
        display.setCursor(12,27);
        display.println("Set");
        display.setCursor(0,44);
        display.println("Motar");

        //選択画面で矢印マークを中央に表示
        display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(85,40);
        display.println("START");

        //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
        if(aa == 0){
          if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
            aa = 1;
          }
        }else{
          if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
            A_OLED = 60;  //その選択されているステートにレッツゴー
            aa = 0;
          }
        }
      }
    }
    else if(A_OLED == 10)  //START
    { //機体の中心となるコート上での0°の位置を決めるところ
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };

      //OLEDの初期化
      display.display();
      display.clearDisplay();

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(30,0);
      display.println("Please");
      display.setCursor(0,20);
      display.println("CAL");
      display.setCursor(40,20);
      display.println("&");
      display.setCursor(56,20);
      display.println("SetDir");

      display.setTextSize(1);
      display.setCursor(38,40);
      display.println("of BNO055");

      display.setTextColor(WHITE);
      if(Button_select == 1)  //exitが選択されていたら
      {
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
      }
      display.setCursor(0,56);
      display.println("Exit");

      display.setTextColor(WHITE);
      if(Button_select == 0)  //nextが選択されていたら（デフォルトはこれ）
      {
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
      }
      display.setCursor(104,56);
      display.println("Next");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          if(Button_select == 0)  //nextが選択されていたら
          {
            ac.setup_2();  //姿勢制御の値リセットするぜい
            A_OLED = 15;  //スタート画面に行く
          }
          else if(Button_select == 1)  //exitが選択されていたら
          {
            A_OLED = 0;  //メニュー画面に戻る
          }
          aa = 0;
        }
      }
    }
    else if(A_OLED == 15)  //ボタン押したらロボット動作開始
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 1;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };

      //OLEDの初期化
      display.display();
      display.clearDisplay();

      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(38,35);
      display.println("Dir :");
      display.setTextSize(2);
      display.setCursor(80,30);
      display.println(int(ac.getnowdir()));
      
      //角度を再設定させるか、もとの選択画面に戻るかを決めるスイッチについての設定

      display.setTextColor(WHITE);
      if(Button_select == 0)  //Leftが選択されていたら
      {
        display.setTextSize(3);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("START");

        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }

        OutB_flag = 1;  //左側にセットするとフラグを立てる
      }
      display.setTextSize(1);
      display.setCursor(0,56);
      display.println("Left");

      display.setTextColor(WHITE);
      if(Button_select == 1)  //SetDir Againが選択されていたら(デフォルト)
      {
        display.setTextSize(3);
        display.setTextColor(WHITE);
        display.setCursor(22,0);
        display.println("START");

        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }

        OutB_flag = 999;  //デフォルトに戻す
      }
      display.setTextSize(1);
      display.setCursor(50,56);
      display.println("Set");

      display.setTextColor(WHITE);
      if(Button_select == 2)  //Rightが選択されていたら
      {
        display.setTextSize(3);
        display.setTextColor(WHITE);
        display.setCursor(34,0);
        display.println("START");

        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }

        OutB_flag = 0;  //右側にセットするとフラグを立てる
      }
      display.setTextSize(1);
      display.setCursor(90,56);
      display.println("Right");

      if(Button_select == 3)
      {
        display.setTextSize(3);
        display.setTextColor(WHITE);
        display.setCursor(22,0);
        display.println("START");
      }

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          if(Button_select == 1)  //SetDir Againが選択されていたら
          {
            ac.setup_2();  //姿勢制御の値リセットするぜい
          }
          else if(Button_select == 3)  //Exitが選択されていたら
          {
            A_OLED = 0;  //ホーム画面に戻る
          }
          aa = 0;
        }
      }
      if(digitalRead(Toggle_Switch) != toogle)  //
      {
        display.clearDisplay(); //初期化してI2Cバスを解放する
        break;
      }
    }
    else if(A_OLED == 20)  //Set Line
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };

      display.display();
      display.clearDisplay();

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(16,0);
      display.println("Set Line");

      display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
      display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

      //数字を中央揃えにするためのコード
      display.setTextSize(3);
      display.setTextColor(WHITE);
      if(line.LINE_Level >= 1000){      //4桁の場合
        display.setCursor(28,22);
      }else if(line.LINE_Level >= 100){ //3桁の場合
        display.setCursor(40,22);
      }else if(line.LINE_Level >= 10){  //2桁の場合
        display.setCursor(48,22);
      }else{                       //1桁の場合
        display.setCursor(56,22);
      }
      display.println(line.LINE_Level);  //ラインの閾値を表示

      display.setTextSize(1);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(44,56);
      display.println("Confirm");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      //タクトスイッチが押されたら、メニューに戻る
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          address = 0x00;  //EEPROMのアドレスを0x00にする
          // line.LINE_Level = 700;  //初めにデータをセットしておかなければならない
          EEPROM.put(address, line.LINE_Level);  //EEPROMにラインの閾値を保存
          A_OLED = 0;  //メニュー画面へ戻る
          aa = 0;
        }
      }
    }
    else if(A_OLED == 30)  //Check Line
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };
      
      display.display();
      display.clearDisplay();
      line.getLINE_Vec();
      //ラインの位置状況マップを表示する
      display.drawCircle(32, 32, 20, WHITE);  //○ 20

      //ラインの直線と円の交点の座標を求める
      line_x = line.Lvec_Long * cos(line.Lrad);  //ラインのx座標
      line_y = line.Lvec_Long * sin(line.Lrad);  //ラインのy座標

      b = line_y - tan(line.Lrad) * line_x;  //y = tanΘx + b の解の公式のb

      La = 1 + pow(tan(line.Lrad), 2);    //解の公式のa
      Lb = tan(line.Lrad) * b;            //解の公式のb
      Lc = pow(b, 2) - 900;               //解の公式のc

      Ax = (-Lb + sqrt(pow(Lb, 2) - 4 * La * Lc)) / (2 * La);  //A点のx座標
      Ay = tan(line.Lrad) * Ax + b;                            //A点のy座標

      Bx = (-Lb - sqrt(pow(Lb, 2) - 4 * La * Lc)) / (2 * La);  //B点のx座標
      By = tan(line.Lrad) * Bx + b;                            //B点のy座標

      //ラインの線の座標をOLEDでの座標に変換(-1~1の値を0~60の値に変換)
      OLED_line_ax = map(Ax, -1.5, 1.5, 0, 60);  //ラインの線のA点のx座標
      OLED_line_ay = map(Ay, -1.5, 1.5, 0, 60);  //ラインの線のA点のy座標
      OLED_line_bx = map(Bx, -1.5, 1.5, 0, 60);  //ラインの線のB点のx座標
      OLED_line_by = map(By, -1.5, 1.5, 0, 60);  //ラインの線のB点のy座標

      //ラインの線を表示
      display.drawLine((OLED_line_ax + 2), (62 - OLED_line_ay), (OLED_line_bx + 2), (62 - OLED_line_by), WHITE);

      //"Line"と表示する
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(68,0);
      display.println("Line");

      //ここから下のコードのテキストをsize1にする
      display.setTextSize(1);
      display.setTextColor(WHITE);

      //ラインの角度を表示する
      display.setCursor(68,25);
      display.println("Dir:");
      if(line.LINE_on == 1){  //ラインがロボットの下にある
        display.setCursor(96,25);
        display.println(int(line.Lvec_Dir));
      }
      else{  //ラインがロボットの下にない
        display.fillRect(96, 25, 34, 10, WHITE);
      }

      //ラインの距離を表示する
      display.setCursor(68,39);
      display.println("far:");
      if(line.LINE_on == 1){  //ラインがロボットの下にある
        display.setCursor(96,39);
        display.println(line.Lvec_Long);
      }
      else{  //ラインがロボットの下にない
        display.fillRect(96, 39, 34, 10, WHITE);
      }

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      //タクトスイッチが押されたら、メニューに戻る
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A_OLED = 0;  //メニュー画面へ戻る
          aa = 0;
        }
      }

      // //白線の平均値を表示する
      // display.setCursor(68,44);
      // display.println("Whi:");
      // display.setCursor(96,44);
      // display.println(Lwhite);

      // //緑コートの平均値を表示する
      // display.setCursor(68,56);
      // display.println("Gre:");
      // display.setCursor(96,56);
      // display.println(Lgreen);
    }
    else if(A_OLED == 40)  //Set RA
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };

      display.display();
      display.clearDisplay();

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(26,0);
      display.println("Set RA");

      display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
      display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

      //数字を中央揃えにするためのコード
      display.setTextSize(3);
      display.setTextColor(WHITE);
      if(RA_size >= 1000){      //4桁の場合
        display.setCursor(28,22);
      }else if(RA_size >= 100){ //3桁の場合
        display.setCursor(40,22);
      }else if(RA_size >= 10){  //2桁の場合
        display.setCursor(48,22);
      }else{                       //1桁の場合
        display.setCursor(56,22);
      }
      display.println(RA_size);  //ボールの閾値を表示

      display.setTextSize(1);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(44,56);
      display.println("Confirm");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      //タクトスイッチが押されたら、メニューに戻る
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          address = 0x00;  //EEPROMのアドレスを0x00にする（リセット）
          address += sizeof(line.LINE_Level);  //アドレスを次の変数のアドレスにする
          // RA_size = 80;  //初めにデータをセットしておかなければならない
          EEPROM.put(address, RA_size);  //EEPROMにボールの閾値を保存
          A_OLED = 0;  //メニュー画面へ戻る
          aa = 0;
        }
      }
    }
    else if(A_OLED == 50)  //Check Ball
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };
      ball.getBallposition();

      display.display();
      display.clearDisplay();

      //ボールの座標をOLED用にする（無理やりint型にしてOLEDのドットに合わせる）
      OLED_ball_x = map(ball.far * sin(radians(ball.ang)), -150, 150, 0, 60);  //
      OLED_ball_y = map(ball.far * cos(radians(ball.ang)), -150, 150, 0, 60);  //

      //ボールの位置状況マップを表示する
      display.drawCircle(32, 32, 30, WHITE);  //○ 30
      display.drawCircle(32, 32, 20, WHITE);  //○ 20
      display.drawCircle(32, 32, 10, WHITE);  //○ 10
      display.drawLine(2, 32, 62, 32, WHITE); //-
      display.drawLine(32, 2, 32, 62, WHITE); //|

      //ボールの位置を表示する
      if(ball.flag == 1)  //ボールがあれば
      {
        display.fillCircle((OLED_ball_x + 2), (62 - OLED_ball_y), 5, WHITE);
      }

      //"Ball"と表示する
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(68,0);
      display.println("Ball");

      //ここから下のコードのテキストをsize1にする
      display.setTextSize(1);
      display.setTextColor(WHITE);

      //ボールの角度を表示する
      display.setCursor(68,24);
      display.println("Dir:");
      if(ball.flag == 1){  //ボールがあれば値を表示
        display.setCursor(96,24);
        display.println(int(ball.ang));
      }
      else{  //ボールがなければ白い四角形を表示
        display.fillRect(96, 24, 34, 10, WHITE);
      }

      //ボールの距離を表示する
      display.setCursor(68,38);
      display.println("far:");
      if(ball.flag == 1){  //ボールがあれば値を表示
        display.setCursor(96,38);
        display.println(int(ball.far));
      }
      else{  //ボールがなければ白い四角形を表示
        display.fillRect(96, 38, 34, 10, WHITE);
      }

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      //タクトスイッチが押されたら、メニューに戻る
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A_OLED = 0;  //メニュー画面へ戻る
          aa = 0;
        }
      }
    }
    else if(A_OLED == 60)  //Set Motar
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(next)をデフォルトにする
        B_OLED = A_OLED;
      };

      display.display();
      display.clearDisplay();

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(14,0);
      display.println("Set Motar");

      display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
      display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

      //数字を中央揃えにするためのコード
      display.setTextSize(3);
      display.setTextColor(WHITE);
      if(val_max >= 1000){      //4桁の場合
        display.setCursor(28,22);
      }else if(val_max >= 100){ //3桁の場合
        display.setCursor(40,22);
      }else if(val_max >= 10){  //2桁の場合
        display.setCursor(48,22);
      }else{                       //1桁の場合
        display.setCursor(56,22);
      }
      display.println(val_max);  //ラインの閾値を表示

      display.setTextSize(1);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(44,56);
      display.println("Confirm");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      //タクトスイッチが押されたら、メニューに戻る
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          address = 0x00;  //EEPROMのアドレスを0x00にする（リセット）
          address = sizeof(line.LINE_Level) + sizeof(RA_size);  //アドレスを次の変数のアドレスにする
          // val_max = 100;  //初めにデータをセットしておかなければならない
          EEPROM.put(address, val_max);  //EEPROMにボールの閾値を保存
          A_OLED = 0;  //メニュー画面へ戻る
          aa = 0;
        }
      }
    }

    //ロータリーエンコーダーの値を取得し制御する
    long newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      if(newPosition % 4 == 0)  //4の倍数のときのみ実行
      {
        new_encVal = newPosition / 4;  //Aにステートを代入
        if(A_OLED == 0)  //選択画面にいるときはOLED_selectを変更する
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            OLED_select++;  //次の画面へ
            if(OLED_select > 6)  //選択画面の数以上になったら1に戻す
            {
              OLED_select = 1;
            }
          }
        }
        else if(A_OLED == 10)  //スタート画面にいるときはButton_selectを変更する
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            Button_select = 0;  //next
          }
          else if(new_encVal < old_encVal)
          {
            Button_select = 1;  //exit
          }
        }
        else if(A_OLED == 15)
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            if (Button_select < 3)
            {
              Button_select++;
            }
          }
          else if(new_encVal < old_encVal)
          {
            if(Button_select > 0)
            {
              Button_select--;
            }
          }
        }
        else if(A_OLED == 20)  //ラインの閾値を変更する
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            if(line.LINE_Level < 1023)
            {
              line.LINE_Level++;
            }
          }
          else if(new_encVal < old_encVal)
          {
            if(line.LINE_Level > 0)
            {
              line.LINE_Level--;
            }
          }
        }
        else if(A_OLED == 40)  //ボールの閾値を変更する
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            if(RA_size < 1023)
            {
              RA_size++;
            }
          }
          else if(new_encVal < old_encVal)
          {
            if(RA_size > 0)
            {
              RA_size--;
            }
          }
        }
        else if(A_OLED == 60)  //モーターの出力を変更する
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            if(val_max < 1023)
            {
              val_max++;
            }
          }
          else if(new_encVal < old_encVal)
          {
            if(val_max > 0)
            {
              val_max--;
            }
          }
        }
        old_encVal = new_encVal;
      }
    }
  }
}



void OLED_moving(){
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  //テキストサイズと色の設定
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);  //1列目
  display.println("Bang");  //現在向いてる角度
  display.setCursor(30,0);
  display.println(":");
  display.setCursor(36,0);
  display.println(ball.ang);    //現在向いてる角度を表示

  display.setCursor(0,10);  //2列目
  display.println("goDir");  //この中に変数名を入力
  display.setCursor(30,10);
  display.println(":");
  display.setCursor(36,10);
  display.println(goDir);    //この中に知りたい変数を入力

  display.setCursor(0,20); //3列目
  display.println("Lang");  //この中に変数名を入力
  display.setCursor(30,20);
  display.println(":");
  display.setCursor(36,20);
  display.println(line.Lvec_Dir);    //この中に知りたい変数を入力

  display.setCursor(0,30); //4列目
  display.println("Lfla");  //この中に変数名を入力
  display.setCursor(30,30);
  display.println(":");
  display.setCursor(36,30);
  display.println(lFla);    //この中に知りたい変数を入力

  display.setCursor(0,40); //5列目
  display.println("A");  //この中に変数名を入力
  display.setCursor(30,40);
  display.println(":");
  display.setCursor(36,40);
  display.println(A);    //この中に知りたい変数を入力

  display.setCursor(0,50); //6列目
  display.println("out");  //この中に変数名を入力
  display.setCursor(30,50);
  display.println(":");
  display.setCursor(36,50);
  display.println(OutB_flag);  //ここ限定のタイマーだよ(何秒前進するかとか決めるよ));    //この中に知りたい変数を入力
}