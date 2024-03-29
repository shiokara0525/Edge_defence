#include<OLED_d.h>




oled_deffence::oled_deffence(){
}



void oled_deffence::setup(){
  EEPROM.get(address,line.LINE_Level);//EEPROMから読み出し
  address += sizeof(line.LINE_Level);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,RA_size);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  address += sizeof(RA_size);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,val_max);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  address += sizeof(val_max);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,Button_selectCF);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(Tact_Switch,INPUT);
  pinMode(Toggle_Switch,INPUT);
}



void oled_deffence::startOLED(){
  MOTOR.motor_0();
  toogle = digitalRead(Toggle_Switch);
  OLED();
}



void oled_deffence::OLED(){
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
            A_OLED = 12;  //ゴール方向判定画面に行く
          }
          else if(Button_select == 1)  //exitが選択されていたら
          {
            A_OLED = 0;  //メニュー画面に戻る
          }
          aa = 0;
        }
      }
    }
    else if(A_OLED == 12)  //ゴール方向判定するよ
    {
      if(A_OLED != B_OLED){  //ステートが変わったときのみ実行(初期化)
        Button_select = 0;  //ボタンの選択(setDir)をデフォルトにする
        B_OLED = A_OLED;
      };

      display.display();
      display.clearDisplay();

      display.setTextSize(1);

      display.setTextColor(WHITE);
      if(Button_selectCF == 0)  //exitが選択されていたら
      {
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
      }
      display.setCursor(0,30);
      display.println("Yellow");

      display.setTextColor(WHITE);
      if(Button_selectCF == 1)  //exitが選択されていたら
      {
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
      }
      display.setCursor(50,55);
      display.println("Exit");

      display.setTextColor(WHITE);
      if(Button_selectCF == 2)  //exitが選択されていたら
      {
        if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
          display.setTextColor(BLACK, WHITE);
        }
        else{
          display.setTextColor(WHITE);
        }
      }
      display.setCursor(90,30);
      display.println("Blue");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          if(Button_selectCF == 0)  //yellowが選択されていたら
          {
            cam_back.color = 1;
            A_OLED = 15;  //スタート画面に行く
          }
          else if(Button_selectCF == 2)  //blueが選択されていたら
          {
            cam_back.color = 0;
            A_OLED = 15;  //スタート画面に行く
          }
          else if(Button_selectCF == 1)  //exitが選択されていたら
          {
            A_OLED = 0;  //メニュー画面に戻る
          }
          address = 0x00;  //EEPROMのアドレスを0x00にする（リセット）
          address = sizeof(line.LINE_Level) + sizeof(RA_size) + sizeof(val_max);  //アドレスを次の変数のアドレスにする
          EEPROM.put(address, Button_selectCF);  //EEPROMにボールの閾値を保存
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
      line_x = line.dis * cos(line.ang);  //ラインのx座標
      line_y = line.dis * sin(line.ang);  //ラインのy座標

      b = line_y - tan(line.ang) * line_x;  //y = tanΘx + b の解の公式のb

      La = 1 + pow(tan(line.ang), 2);    //解の公式のa
      Lb = tan(line.ang) * b;            //解の公式のb
      Lc = pow(b, 2) - 900;               //解の公式のc

      Ax = (-Lb + sqrt(pow(Lb, 2) - 4 * La * Lc)) / (2 * La);  //A点のx座標
      Ay = tan(line.ang) * Ax + b;                            //A点のy座標

      Bx = (-Lb - sqrt(pow(Lb, 2) - 4 * La * Lc)) / (2 * La);  //B点のx座標
      By = tan(line.ang) * Bx + b;                            //B点のy座標

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
        display.println(int(line.ang));
      }
      else{  //ラインがロボットの下にない
        display.fillRect(96, 25, 34, 10, WHITE);
      }

      //ラインの距離を表示する
      display.setCursor(68,39);
      display.println("far:");
      if(line.LINE_on == 1){  //ラインがロボットの下にある
        display.setCursor(96,39);
        display.println(line.dis);
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
        else if(A_OLED == 12)
        {
          if(new_encVal > old_encVal)  //回転方向を判定
          {
            if(Button_selectCF < 2){
              Button_selectCF++;  //next
            }
          }
          else if(new_encVal < old_encVal)
          {
            if(Button_selectCF  > 0){
              Button_selectCF--;  //next
            }
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