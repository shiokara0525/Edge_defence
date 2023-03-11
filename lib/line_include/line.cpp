#include<line.h>


void LINE::setup() {
  for (int i=0; i<3; i++)
  {
    pinMode(Lselect[i], OUTPUT); //ラインセンサのマルチプレクサを制御するためのピンを出力に設定
    pinMode(Lread[i], INPUT); //ラインセンサICの出力を読み取るためのピンを入力に設定
  }
  pinMode(LINE_light, OUTPUT); //ラインセンサのLEDを出力に設定

  for (int i=0; i<24; i++) //ラインの座標を配列に入れる・ラインセンサ
  {
    Lrad = PI / 12 * i; //ラインセンサのラジアンを計算
    LINE_X[i] = cos(Lrad); //ラインセンサのX座標を求める
    LINE_Y[i] = sin(Lrad); //ラインセンサのY座標を求める
    Lsencer_Dir[i] = 15.0 * i; //ラインセンサの角度を求める
  }
  digitalWrite(LINE_light,HIGH);
}




int LINE::getLINE_Vec() { //ラインのベクトル(距離,角度)を取得する関数
  int LniseF = 50;  //ラインセンサのノイズフィルタを行う回数
  int Lnone = 0;  //ラインセンサのノイズフィルタリングをするために使う変数
  int data[24][LniseF]; //ラインセンサの値を格納する二次元配列
  int data_sum[24]; //ラインセンサの値の合計を格納する配列
  int data_ave[24]; //ラインセンサの値の平均を格納する配列
  int Lnum = 0; //ラインセンサの番号
  int detecting = 0; //ラインを検知しているかどうかのフラグをリセット
  int flag = 0; //ラインセンサの角度が0度をまたいでいるのかいないのかを判断するフラグをリセット
  int Lfirst[24]; //ラインセンサの範囲の開始点の番号を記録(配列が24つあるのは、間違って3つ以上の範囲が出てきても、バグが起こらないようにするため)
  int Llast[24]; //ラインセンサの範囲の終了点の番号を記録(配列が24つあるのは、間違って3つ以上の範囲が出てきても、バグが起こらないようにするため)
  double Lvec_X = 0; //ラインセンサのX座標の和のベクトル
  double Lvec_Y = 0; //ラインセンサのY座標の和のベクトル
  double Lvec_X_move = 0; //ラインベクトルの移動量を記録する
  double Lvec_Y_move = 0; //ラインベクトルの移動量を記録する
  double Lsencer_Dir_ave = 0; //ラインセンサの範囲の角度を出すときのラインセンサの始点と終点の和の平均
  unsigned long Ltime_move = 1000; //ラインの移動量を定期的に計測するための時間の間隔

  Lrange_num = 0;

  for(int j=0; j<LniseF; j++) //ラインセンサを24個読み取るを1セットとし、100セット読み取る
  {
    for(int i=0; i<8; i++)  //8chマルチプレクサ×3なので8回まわす、そして、24個のラインセンサを指定する
    {
      if(i==1 || i==3 || i==5 || i==7){
        digitalWrite(Lselect[0],HIGH);
      }else{
        digitalWrite(Lselect[0],LOW);
      }
      if(i==2 || i==3 || i==6 || i==7){
        digitalWrite(Lselect[1],HIGH);
      }else{
        digitalWrite(Lselect[1],LOW);
      }
      if(i>=4){
        digitalWrite(Lselect[2],HIGH);
      }else{
        digitalWrite(Lselect[2],LOW);
      }

      for (int Lic_num=0; Lic_num<3; Lic_num++)
      {
        Lnum = i + Lic_num * 8; //ラインセンサの番号を指定
        data[Lnum][j] = analogRead(Lread[Lic_num]); //ラインセンサの値を記録
      }
    }
  }

  for(int i=0; i<24; i++) //24個のラインセンサを指定する
  {
    for(int j=0; j<LniseF; j++) //ラインセンサを24個読み取るを1セットとし、100セット読み取る
    {
      // data_sum[i] = data_sum[i] + data[i][j]; //ラインセンサの値を合計する

      if(LINE_Level < data[i][j]) //ラインセンサの値が閾値より大きければ（ライン上にいるとき）
      {
        Lnone++; //ラインセンサの上にラインがあるということを数える
        data_sum[i] = data_sum[i] + data[i][j]; //ラインセンサ(ライン上のとき)の値を合計する
      }
    }

    if(Lnone > LniseF*0.6)  //ラインを調べた60%以上がライン上にいるとき
    {
      data_ave[i] = data_sum[i] / Lnone; //ラインセンサの値を平均する
    }
    else
    {
      data_ave[i] = 0; //ラインセンサの値を0(ライン上にいない)にする
    }
    
    // Serial.print(data_sum[i]);
    // Serial.print(" ");
    // Serial.print(Lnone);

    data_sum[i] = 0; //合計値をリセット
    Lnone = 0; //ラインセンサの上にラインがあるということを数えるをリセット(一つのセンサごとに)

    // Serial.print(data_ave[i]); //ラインセンサの値をシリアルモニタに表示
    // Serial.print(" ");
  }
  data_ave[4] = 0;
  data_ave[8] = 0;
  data_ave[20] = 0;

  for(int i=0; i<24; i++) //24個のラインセンサを指定する
  {
    if(data_ave[i] > LINE_Level) //ラインセンサの値が閾値より大きければ（ラインセンサの上にラインあり）
    {
      if(detecting == 0) //前のラインセンサが検知していなければ
      {
        Lrange_num++; //ラインセンサの範囲の番号を設定（もしラインを見ていなければ、この値は0になる）
        Lfirst[Lrange_num] = i; //ラインの範囲の最初のラインセンサを記録
        detecting = 1; //ラインを検知しているフラグを立てる
      }
    }
    else //ラインセンサの値が閾値より小さければ（ラインセンサの上にラインなし）
    {
      if(detecting == 1) //前のラインセンサが検知していれば
      {
        Llast[Lrange_num] = i - 1; //ラインの範囲の最後のラインセンサを記録
        detecting = 0; //ラインを検知しているフラグを下ろす
      }
    }
  }

  if(data_ave[23] > LINE_Level && data_ave[0] <= LINE_Level) //もしラインが0は反応していなくても23は反応していたら23の範囲のLlastを使いたいので23を代入する
  {
    Llast[Lrange_num] = 23;
  }

  if(Lfirst[1] == 0 && data_ave[23] > LINE_Level) //ラインセンサの範囲が23~0へと数字をまたいだ時、23までの範囲と0から終わりまでの範囲を合成させる
  {
    Lfirst[1] = Lfirst[Lrange_num];  //ラインの範囲を1番目の範囲に合成させる
    Lrange_num--; //ラインの範囲の数を1つ減らす
    flag = 1; //ラインが0度をまたいでいることを表す
  }

  if(Lrange_num == 1) //ラインセンサの範囲が1個だったとき、1番目の範囲のベクトルを計算する
  {
    Lvec_X = LINE_X[Lfirst[1]] + LINE_X[Llast[1]];
    Lvec_Y = LINE_Y[Lfirst[1]] + LINE_Y[Llast[1]]; //ラインのベクトルを計算する
  }
  else if(Lrange_num == 2) //ラインセンサの範囲が2個だったとき
  {
    for (int i=1; i<3; i++)
    {
      if(flag == 1) //ラインセンサの角度が0度をまたいでいたら、角度の平均がバグってしまうのでバグらないように調節する
      {
        Lsencer_Dir_ave = (Lsencer_Dir[Lfirst[i]] + Lsencer_Dir[Llast[i]] + 360.0) / 2.0;
        if(Lsencer_Dir_ave >= 360.0)
        {
          Lsencer_Dir_ave = Lsencer_Dir_ave - 360.0;
        }
        flag = 0; //リセット
      }
      else
      {
        Lsencer_Dir_ave = (Lsencer_Dir[Lfirst[i]] + Lsencer_Dir[Llast[i]]) / 2.0;  //ここおかしい
      }
      Lrad = (Lsencer_Dir_ave) * PI / 180.0; //iは1から始まる
      LINE_X[23 + i] = cos(Lrad); //ラインセンサのX座標を求める
      LINE_Y[23 + i] = sin(Lrad); //ラインセンサのY座標を求める
    }
    Lvec_X = LINE_X[24] + LINE_X[25];
    Lvec_Y = LINE_Y[24] + LINE_Y[25]; //ラインのベクトルを計算する
  }
  else if(Lrange_num == 3) //ラインセンサの範囲が3個だったとき
  {
    for (int i=1; i<4; i++)
    {
      Lrad = (Lsencer_Dir[Lfirst[i]] + Lsencer_Dir[Llast[i]]) * PI / 360.0; //iは1から始まる
      LINE_X[23 + i] = cos(Lrad); //ラインセンサのX座標を求める
      LINE_Y[23 + i] = sin(Lrad); //ラインセンサのY座標を求める
    }
    Lvec_X = LINE_X[24] + LINE_X[25] + LINE_X[26];
    Lvec_Y = LINE_Y[24] + LINE_Y[25] + LINE_Y[26]; //ラインのベクトルを計算する
  }
  else //もしラインセンサの範囲が3個以上あった場合おかしいものと考え、もう一度ラインセンサの値を読み取る（一からやり直す）
  {
    if(LINE_on == 1) //急にラインが検出されていたのに、されなくなったら(一応ラインがあるとして考える)
    {
      Lvec_X_move = Lvec_X - Lvec_X_old; //ラインの移動ベクトルを求める
      Lvec_Y_move = Lvec_Y - Lvec_Y_old; //ラインの移動ベクトルを求める
      Lvec_Dir_move = atan2(Lvec_Y_move, Lvec_X_move) * 180.0 / PI; //ラインのベクトルの角度を求める
      Lvec_Long_move = sqrt(pow(abs(Lvec_X_move),2) + pow(abs(Lvec_Y_move),2)); //ラインのベクトルの長さを求める
      LINE_on = 0; //ラインがロボットの下になかったとする（ラインの移動ベクトルを求めるときに使う）
      return 1; //ラインがロボットの上にはないがラインが移動していたので、ラインの処理をしてほしいからラインがあると返す
    }
    LINE_on = 0; //ラインがロボットの下になかったとする（ラインの移動ベクトルを求めるときに使う）
    Lvec_X_old = 0; //ラインがないので、過去の値をなかったものとする
    Lvec_Y_old = 0; //ラインがないので、過去の値をなかったものとする
    return 0; //関数から抜ける  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  };

  if(LINE_on == 0) //初めてラインを発見したら、ラインの最大のベクトルを記録する
  {
    timer1.reset(); //タイマーをリセット&スタートする（ラインの動きを一定時間たったら見るため）
    Lrad = atan2(Lvec_Y, Lvec_X);
    Lvec_X_old = cos(Lrad) * 2; //ラインセンサのX座標を求める
    Lvec_Y_old = sin(Lrad) * 2; //ラインセンサのY座標を求める
  }
  
  //ラインがロボットの下にあった場合
  if(timer1.read_ms() > Ltime_move) //もし1秒以上かかったら
  {
    Lvec_X_move = Lvec_X - Lvec_X_old; //ラインの移動ベクトルを求める
    Lvec_Y_move = Lvec_Y - Lvec_Y_old; //ラインの移動ベクトルを求める
    Lvec_Dir_move = atan2(Lvec_Y_move, Lvec_X_move) * 180.0 / PI; //ラインのベクトルの角度を求める
    Lvec_Long_move = sqrt(pow(abs(Lvec_X_move),2) + pow(abs(Lvec_Y_move),2)); //ラインのベクトルの長さを求める
    timer1.reset();  //もう一度Ltime_moveの時間の計測をする（定期的にラインの移動量を計測するため）
  }
  Lvec_Dir = degrees(atan2(Lvec_Y, Lvec_X)); //ラインのベクトルの角度を求める
  Lvec_Long = sqrt(pow(abs(Lvec_X),2) + pow(abs(Lvec_Y),2)); //ラインのベクトルの長さを求める

  LINE_on = 1; //ラインがロボットの下にあったとする（ラインの移動ベクトルを求めるときに使う）
  return 1; //関数から抜ける
}


int LINE::switchLineflag(float linedir){
  int line_flag = 0;
  for(int i = 0; i < 4; i++){  //角度を四つに区分して、それぞれどの区分にいるか判定するよ
    if(i == 0){  //-45°~45°の区分(ここだけ0°をまたいでいるので特別に処理)
      if(315 < linedir || linedir < 45){  //-45°~45°にいるとき
        line_flag = i + 1;  //ラインを前のほうで踏んでると判定する
      }
    }
    else{
      if(-45 +(i * 90) < linedir && linedir < 45 +(i * 90)){  //それ以外の三つの区分(右、後ろ、左で判定してるよ)
        line_flag = i + 1;
      }
    }
  }
  
  return line_flag;
}


double line_switch(int,double,int);


float LINE::decideGoang(float linedir,int line_flag){
  float goang = 0;
  
  for(int i = 0; i < 12; i++){  //角度を12つに区分して、それぞれどの区分にいるか判定する

    if(i == 0){  //-15°~15°の区分(ここだけ0°をまたいでいるので特別に処理)
      if(345 < linedir || linedir < 15){
        goang = line_switch(i,linedir,line_flag);  //ラインがロボットの中心を通り越すことがあるからそれも考慮してるよ(関数は下にあるよ)
      }
    }
    else{
      if(-15 +(i * 30) < linedir && linedir < 15 +(i * 30)){  //時計回りにどの区分にいるか判定してるよ
        goang = line_switch(i,linedir,line_flag);
      }
    }
  }
  return goang;
}


double line_switch(int i,double ang,int line_flag){  //ラインを踏みこしてるときの処理とか判定とか書いてあるよ
  if(i == 11 || i <= 1){
    if(line_flag == 3){
      return 0.0;
    }
  }
  else if(2 <= i && i <= 4){
    if(line_flag == 4){
      return 90.0;
    }
  }
  else if(5 <= i && i <= 7){
    if(line_flag == 1){
      return 180.0;
    }
  }
  else if(8 <= i && i <= 10){
    if(line_flag == 2){
      return -90.0;
    }
  }

  double goang = (i * 30.0)- 180.0;

  Serial.print(" 踏んだ角度 : ");
  Serial.print(goang);

  return goang;
}



void LINE::print(){
  Serial.print(" 角度 : ");
  Serial.print(Lvec_Dir); //ラインのベクトルを表示
  Serial.print(" 距離 : ");
  Serial.print(Lvec_Long); //ラインのベクトルを表示
}