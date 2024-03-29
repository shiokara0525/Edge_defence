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
#include<Cam.h>
#include<A_B.h>

/*---------------------------------------------------ディスプレイの宣言-----------------------------------------------------------------------------------*/

int RA_size;
int val_max;
int OutB_flag = 999;
const int Tact_Switch = 15;
const int Toggle_Switch = 14;  //スイッチのピン番号
float goDir = 0;
int goVal = 0;
int lFla;

void OLED_moving();

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

BALL ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
Cam cam_back;
motor_deffence MOTOR;
oled_deffence OLED;
timer c_;
timer Timer_sentor;
timer Timer_side;
A_B side(500);

int A = 0;  //どのチャプターに移動するかを決める変数

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int x = 0;
int y = 0;
int num = 0;

int stop_range = 10;
int P_range = 25;
int stop_flag = 0;
int s_b = 999;
int go_range = 25;
timer stop_t;

int cam_LR = 0; //ロボットが右側にいたら0、左側にいたら1
int flag = 0;
int count = 0;
/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Wire.begin();  //I2Cできるよ
  angle go_ang(0,true);
  ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)(ただ通信を成功させときたいだけ)
  line.setup();  //ラインとかのセットアップ
  OLED.setup();
  ball.begin();
  cam_back.begin();
  OLED.OLED();

  val_max = OLED.val_max;
  A = 10;
}




void loop(){
  double AC_val = 0;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);  //進む角度だぜいえいえ
  int goval = val_max;  //進む速度だぜいえいえ
  flag = 0;

  if(A == 10){  //ボールを追うのか、ちょっと復帰するのか、だいぶ復帰するのか決める
    ball.getBallposition();
    line.getLINE_Vec();
    AC_val = ac.getAC_val();
    if(line.LINE_on == 1){
      A = 20;  //ライン踏んでたらいったんボール追う
    }
    else{
      A = 11;  //ライン踏んでなかったらいったん戻る
    }

    int A_side = 0;
    if(cam_back.on == 1 && cam_back.Size < 30){
      if(abs(go_ang.degree) < 20 || 160 < abs(go_ang.degree)){
        A_side = 1;
      }
    }
    else if(cam_back.on == 0){
      if(60 < abs(go_ang.degree) && abs(go_ang.degree) < 120){
        A_side = 2;
      }
    }
    Serial.print(A_side);
    Serial.print(" ");

    int s_flag = side.setA(A_side);
    if(s_flag == 1){
      A = 12;
    }
    else if(s_flag == 2){
      A = 13;
    }

    if(OLED.OutB_flag != 999){
      A = 12;
    }
  }


  if(A == 11){
    go_ang = line.ang_old;
    A = 50;
    flag = 11;
  }


  if(A == 12){
    flag = 12;
    if(line.LINE_on == 1){
      while(line.getLINE_Vec()){
        go_ang = 180 + cam_back.ang;
        MOTOR.moveMotor_0(go_ang,100,ac.getAC_val());
        goDir = go_ang.degree;
        OLED_moving();
      }
    }
    while(!line.getLINE_Vec()){
      go_ang = 180 + cam_back.ang;
      MOTOR.moveMotor_0(go_ang,110,ac.getAC_val());
      goDir = go_ang.degree;
      OLED_moving();
    }
    OLED.OutB_flag = 999;
    A = 50;
  }


  if(A == 13){
    flag = 13;
    if(cam_back.LR == 1){
      go_ang = -90;
    }
    else if(cam_back.LR == 0){
      go_ang = 90;
    }
    A = 50;
  }


  if(A == 14){
    flag = 14;
    while(!line.getLINE_Vec()){
      go_ang = 180 + cam_back.ang;
      MOTOR.moveMotor_0(go_ang,110,ac.getAC_val());
      goDir = go_ang.degree;
      OLED_moving();
    }
    A = 10;
  }


  if(A == 20){  //ライン踏んでるとき(ライントレース)
    int go_flag = 0;
    double go_border[2];  //ボールの角度によって進む方向を変えるためのボーダーの変数(ラインに対して垂直な直線で進む角度の区分を分けるイメージ)
    angle balldir(ball.ang,true);  //ボールの角度を入れるオブジェクト
    if(2 < line.num){
      line.ang = 90;
    }

    if(line.ang < 0){
      go_border[0] = line.ang;
      go_border[1] = line.ang + 180;
    }
    else{
      go_border[0] = line.ang - 180;
      go_border[1] = line.ang;
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


    if(120 < abs(go_ang.degree)){       //進む角度が真後ろにあるとき
      goval = 0;
    }
    else if(110 < abs(go_ang.degree)){  //進む角度が後ろめな時
      goval = 50;
      MOTOR.line_val = 2;
    }
    else if(abs(go_ang.degree) < 60){  //前めに進むとき
      MOTOR.line_val = 2;
    }
    else{                              //横に進むとき
      MOTOR.line_val = 0.90;
    }
    stop_flag = 0;
    for(int i = 0; i < 2; i++){
      int dif_val = abs(ball.ang - go_border[i]);
      if(dif_val < stop_range){  //正面方向にボールがあったら停止するよ
        goval = 0;
      }
      else if(dif_val < P_range){
        goval = val_max / (P_range - stop_range) * (dif_val - stop_range);
      }
    }
    if(abs(ball.ang) < go_range){
      stop_flag = 1;
    }
    A = 50;

    if(stop_flag == 0){
      if(stop_flag != s_b){
        s_b = stop_flag;
      }
    }
    else if(stop_flag == 1){
      if(stop_flag != s_b){
        s_b = stop_flag;
        stop_t.reset();
      }
      if(5000 < stop_t.read_ms()){
        A = 30;
        stop_t.reset();
      }
    }
    flag = 20;
  }

  if(A == 30){
    goval = 120;

    if(abs(ball.ang) < go_range + 10){
      OutB_flag = 1;

      while(stop_t.read_ms() < 1100){  //0.9秒前進するよ
        float ac_val = ac.getAC_val();
        ball.getBallposition();

        go_ang = ball.ang;
        MOTOR.moveMotor_0(go_ang,goval,ac_val);
        if(go_range + 10 < abs(ball.ang)){  //前にボールがなくなったらすぐ戻るよ
          break;
        }
      }
    }
    goval = 80;
    A = 14;
    Serial.print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }

  if(A == 50){
    if(flag == 11 || flag == 13){
      MOTOR.moveMotor_0(go_ang,goval,AC_val);
    }
    else{
      MOTOR.moveMotor_l(go_ang,goval,AC_val,line);
    }
    // Serial.print("go_ang : ");
    // Serial.print(go_ang.degree);
    // line.print();
    Serial.println();
    A = 10;
  }
  OLED_moving();
  goDir = go_ang.degree;
  goVal = goval;
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/
void serialEvent1(){
  uint8_t reBuf[5];
  if(Serial1.available() < 5){
    return;
  }

  for(int i = 0; i < 5; i++){
    reBuf[i] = Serial1.read();
    // Serial.print(reBuf[i]);
    // Serial.print(" ");
  }
  // Serial.print(1000.0 / c_.read_ms());
  // c_.reset();
  // Serial.println();
  while(Serial1.available()){
    Serial1.read();
  }

  if(reBuf[0] == 38 && reBuf[4] == 37){
    if(reBuf[3] == 0){
      cam_back.on = 0;
    }
    else{
      if(cam_back.color == reBuf[1]){
        cam_back.on = 1;
        cam_back.Size = reBuf[3];
        cam_back.ang = -(reBuf[2] - 127);
      }
    }
  }

  if(cam_back.ang < 0){
    cam_back.LR = 0;
  }
  else{
    cam_back.LR = 1;
  }
  // Serial.print("sawa");
}




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
  OLED.display.println("flag");  //現在向いてる角度
  OLED.display.setCursor(30,0);
  OLED.display.println(":");
  OLED.display.setCursor(36,0);
  OLED.display.println(flag);    //現在向いてる角度を表示

  OLED.display.setCursor(0,10);  //2列目
  OLED.display.println("B_A");  //この中に変数名を入力
  OLED.display.setCursor(30,10);
  OLED.display.println(":");
  OLED.display.setCursor(36,10);
  OLED.display.println(ball.ang);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,20); //3列目
  OLED.display.println("G_a");  //この中に変数名を入力
  OLED.display.setCursor(30,20);
  OLED.display.println(":");
  OLED.display.setCursor(36,20);
  OLED.display.println(goDir);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,30); //4列目
  OLED.display.println("c_s");  //この中に変数名を入力
  OLED.display.setCursor(30,30);
  OLED.display.println(":");
  OLED.display.setCursor(36,30);
  OLED.display.println(cam_back.Size);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,40); //5列目
  OLED.display.println("c_o");  //この中に変数名を入力
  OLED.display.setCursor(30,40);
  OLED.display.println(":");
  OLED.display.setCursor(36,40);
  OLED.display.println(cam_back.on);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,50); //6列目
  OLED.display.println("c_a");  //この中に変数名を入力
  OLED.display.setCursor(30,50);
  OLED.display.println(":");
  OLED.display.setCursor(36,50);
  OLED.display.println(cam_back.ang);    //この中に知りたい変数を入力

  if(digitalRead(Tact_Switch) == LOW){
    OLED.startOLED();
    A = 10;
  }
}