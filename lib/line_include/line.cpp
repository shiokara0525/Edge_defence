#include<line.h>
#include<angle.h>

void LINE::setup() {
  for (int i=0; i<3; i++)
  {
    pinMode(Lselect[i], OUTPUT); //ラインセンサのマルチプレクサを制御するためのピンを出力に設定
    pinMode(Lread[i], INPUT); //ラインセンサICの出力を読み取るためのピンを入力に設定
  }
  pinMode(LINE_light, OUTPUT); //ラインセンサのLEDを出力に設定

  for (int i=0; i<24; i++) //ラインの座標を配列に入れる・ラインセンサ
  {
    Lrad = PI_E / 12 * i; //ラインセンサのラジアンを計算
    ele_X[i] = cos(Lrad); //ラインセンサのX座標を求める
    ele_Y[i] = sin(Lrad); //ラインセンサのY座標を求める
    Lsencer_Dir[i] = 15.0 * i; //ラインセンサの角度を求める
  }
  digitalWrite(LINE_light,HIGH);
}




int LINE::getLINE_Vec() { //ラインのベクトル(距離,角度)を取得する関数
  int LniseF = 10;  //ラインセンサのノイズフィルタを行う回数
  int data[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //ラインセンサの値を格納する二次元配列
  int data_ave[24]; //ラインセンサの値の平均を格納する配列
  int data_on[24];
  int Lnum = 0;
  int flag = 0;
  int block_first[Long];
  int block_last[Long];
  int block_num = -1;
  float block_X[Long];
  float block_Y[Long];

  float X = 0;
  float Y = 0;

  for(int j=0; j<LniseF; j++) //ラインセンサを24個読み取るを1セットとし、100セット読み取る
  {
    for(int i=0; i<8; i++)  //8chマルチプレクサ×3なので8回まわす、そして、24個のラインセンサを指定する
    {
      if(i==1 || i==3 || i==5 || i==7){
        digitalWriteFast(Lselect[0],HIGH);
      }else{
        digitalWriteFast(Lselect[0],LOW);
      }
      if(i==2 || i==3 || i==6 || i==7){
        digitalWriteFast(Lselect[1],HIGH);
      }else{
        digitalWriteFast(Lselect[1],LOW);
      }
      if(i>=4){
        digitalWriteFast(Lselect[2],HIGH);
      }else{
        digitalWriteFast(Lselect[2],LOW);
      }

      for (int Lic_num=0; Lic_num<3; Lic_num++)
      {
        Lnum = i + Lic_num * 8; //ラインセンサの番号を指定
        data[Lnum] += analogRead(Lread[Lic_num]); //ラインセンサの値を記録
      }
    }
  }

  for(int i=0; i<24; i++) //24個のラインセンサを指定する
  {
    data_ave[i] = data[i] / LniseF;
    
    // Serial.print(data_sum[i]);
    // Serial.print(" ");
    // Serial.print(Lnone);

    if(LINE_Level < data_ave[i]){
      data_on[i] = 1;
    }
    else{
      data_on[i] = 0;
    }
    Serial.print(data_on[i]);
    Serial.print(" ");
  }
  Serial.println();



  for(int i = 0; i < 24; i++){
    if(i == 11 || i == 12){
      continue;
    }
    if(flag == 0){
      if(data_on[i] == 1){
        block_num++;
        block_first[block_num] = i;
        flag = 1;
      }
    }
    else{
      if(data_on[i] == 0){
        block_last[block_num] = i - 1;
        flag = 0;
      }
    }

    if(i == 23){
      if(data_on[23] == 1 && data_on[0] == 1){
        block_first[0] = block_first[block_num];
        block_first[block_num] = 0;
        block_num--;
      }

      if(data_on[23] == 1 && data_on[0] == 0){
        block_last[block_num] = 23;
      }
    }
  }

  for(int i = 0; i <= block_num; i++){
    block_X[i] = ele_X[block_first[i]] + ele_X[block_last[i]];
    block_Y[i] = ele_Y[block_first[i]] + ele_Y[block_last[i]];
    X += block_X[i];
    Y += block_Y[i];
  }
  block_num++;

  X /= block_num;
  Y /= block_num;
  dis_X = X;
  dis_Y = Y;
  dis = sqrt(X*X + Y*Y);
  ang = degrees(atan2(Y,X));
  num = block_num;

  if(block_num == 0){
    LINE_on = 0;
  }
  else{
    LINE_on = 1;
    ang_old = ang;
  }
  return LINE_on;
}


int LINE::switchLineflag(angle linedir){
  linedir.to_range(-45,false);
  line_flag = 0;
  for(int i = 0; i < 4; i++){  //角度を四つに区分して、それぞれどの区分にいるか判定するよ
    if(-45 +(i * 90) < linedir.degree && linedir.degree < 45 +(i * 90)){  //それ以外の三つの区分(右、後ろ、左で判定してるよ)
      line_flag = i + 1;
    }
  }
  
  return line_flag;
}


double line_switch(int,double,int);


float LINE::decideGoang(angle linedir,int line_flag){
  float goang = 0;
  linedir.to_range(-15,false);
  for(int i = 0; i < 12; i++){  //角度を12つに区分して、それぞれどの区分にいるか判定する
    if(-15 +(i * 30) < linedir.degree && linedir.degree < 15 +(i * 30)){  //時計回りにどの区分にいるか判定してるよ
      goang = line_switch(i,linedir.degree,line_flag);
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


  return goang;
}




void LINE::print(){
  Serial.print(" 個数 : ");
  Serial.print(num);
  Serial.print(" 角度 : ");
  Serial.print(ang); //ラインのベクトルを表示
  // Serial.print(" 距離 : ");
  // Serial.print(dis); //ラインのベクトルを表示
  // Serial.print("  X : ");
  // Serial.print(dis_X); //ラインのベクトルを表示
  // Serial.print("  Y : ");
  // Serial.print(dis_Y); //ラインのベクトルを表示
}