#include<motor_d.h>



motor_deffence::motor_deffence(){
	for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
    Motor[i].setLenth(moter_max);
  }  //モーターのピンと行列式に使う定数の設定
}




void motor_deffence::moveMoter_l(angle ang,int val,double ac_val,LINE line){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(ang.radians);  //進みたいベクトルのx成分
  double mval_y = sin(ang.radians);  //進みたいベクトルのy成分
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] *(mval_x + line.Lvec_X * line_val)  + mCos[i] *(mval_y + line.Lvec_Y * line_val);
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //移動平均求めるゾーンだよ
    Mval[i] /= g;  //モーターの値を制御(常に一番大きい値が1になるようにする)

    Mval[i] = Motor[i].demandAve(Mval[i]);

    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの値を計算するところだよ
    Mval[i] = Mval[i] / h * max_val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    Val[i] = Mval[i];
    

    for(int i = 0; i < 4; i++){
      if(i == 0 || i == 1 || i == 2){
        if(Mval[i] < 0){
          digitalWrite(pah[i],LOW);
        }
        else{
          digitalWrite(pah[i],HIGH);
        }
      }
      else{
        if(Mval[i] < 0){
          digitalWrite(pah[i],HIGH);
        }
        else{
          digitalWrite(pah[i],LOW);
        }
      }
      analogWrite(ena[i],abs(Mval[i]));
    }
  }
}


void motor_deffence::moveMoter_0(angle ang,int val,double ac_val){
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(ang.radians);  //進みたいベクトルのx成分
  double mval_y = sin(ang.radians);  //進みたいベクトルのy成分
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //移動平均求めるゾーンだよ
    Mval[i] /= g;  //モーターの値を制御(常に一番大きい値が1になるようにする)

    Mval[i] = Motor[i].demandAve(Mval[i]);

    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの値を計算するところだよ
    
    Mval[i] = Mval[i] / h * max_val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)

    if(i == 0 || i == 1 || i == 2){
      if(0 < Mval[i]){            //モーターの回転方向が正の時
        digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      }
      else{  //モーターの回転方向が負の時
        digitalWrite(pah[i] , HIGH);     //モーターの回転方向を負にする
      }
    }
    else{
      if(0 < Mval[i]){            //モーターの回転方向が正の時
        digitalWrite(pah[i] , HIGH);    //モーターの回転方向を正にする
      }
      else{  //モーターの回転方向が負の時
        digitalWrite(pah[i] , LOW);     //モーターの回転方向を負にする
      }
    }
    analogWrite(ena[i] , abs(Mval[i])); //モーターの回転速度を設定

  }
}


void motor_deffence::moter_ac(float ac_val){
  for(int i = 0; i < 4; i++){
    if(i == 0 || i == 1 || i == 2){
      if(ac_val < 0){
        digitalWrite(pah[i],LOW);
      }
      else{
        digitalWrite(pah[i],HIGH);
      }
    }
    else{
      if(ac_val < 0){
        digitalWrite(pah[i],HIGH);
      }
      else{
        digitalWrite(pah[i],LOW);
      }
    }
    analogWrite(ena[i],abs(ac_val));
  }
}


void motor_deffence::moter_0(){  //モーターの値を0にする関数
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
    Motor[i].reset();
  }
}