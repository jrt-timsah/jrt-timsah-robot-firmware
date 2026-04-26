/*
【定義】
　SW_SHOT：射出ボタン．押すと1，離すと0になる．
　SW_ROLLER：ローラー回転ボタン．押すと1，離すと0になる．

【変数】
RxData[0]~RxData[3]:コントローラから受信したデータが入った配列
  AS_LeftX ：左スティック左右のアナログ量(-100～100)(配列：RxData[0])
  AS_LeftY ：左スティック上下のアナログ量(-100～100)(配列：RxData[1])
  AS_RightX：右スティックのアナログ量(-100～100)(配列：RxData[2])
  AS_Vol  ：ボリュームのアナログ量(0～255)(配列：RxData[3])
ShotSeq：射出のシーケンス管理用変数
RollerSeq：ローラー回転シーケンスの管理用変数
RollerOnOff：ローラー回転状態の管理用変数

【関数】
MotorON(motor, pwm)：モータを回すための関数　motor：回すモータ　pwm：-100~100
  ROLLER ：射出ローラー
  例)　MotorON(WHEEL_A, -50);
MotorOFF(motor)：モータを停止するための関数　motor：停止するモータ

ServoON(servo, angle)：サーボを動かすための関数　servo：動かすサーボ　angle：90~180
  PITCH ：射出角度調整用のサーボ
  SHOT  ：ディスク射出用のサーボ
  例)　ServoON(PITCH, 100);
  
*/

// 足回りの制御
// 定数
float Gain[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float Offset[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float RejectLine = 20.0f;
float VmSignSet[3][4] = {
  {1.0f,1.0f,-1.0f,-1.0f},
  {-1.0f,1.0f,-1.0f,1.0f},
  {-1.0f,-1.0f,-1.0f,-1.0f}
};
// モーター4つ分の出力を計算して格納する
float motorPower[4];
float mostStrongestPower = 0.0f;
void updateMotorPower(float ly, float lx, float rx) {
  if (abs(ly) <= RejectLine) ly = 0.0f; if (abs(lx) <= RejectLine) lx = 0.0f; if (abs(rx) <= RejectLine) rx = 0.0f;
  bool isNegative [4];
  for(int i=0; i<4; i++) {
    motorPower[i] = (VmSignSet[0][i] * ly) + (VmSignSet[1][i] * lx) + (VmSignSet[2][i] * rx);
    motorPower[i] = motorPower[i] * Gain[i] + Offset[i];
    motorPower[i] = min(100, motorPower[i]);
    isNegative[i] = (motorPower[i] < 0);
    motorPower[i] = pow(abs(motorPower[i]), 1.8) / 48.0;
    if (isNegative[i] == true){
      motorPower[i] = -motorPower[i];
    }
    mostStrongestPower = max(mostStrongestPower, abs(motorPower[i]));
  }
}
void Wheel(void){
  updateMotorPower(AS_LeftY, AS_LeftX, AS_RightX);
  float Rate = 75.0f / mostStrongestPower; // Rate = (Speed) / mostStrongestPower; Speed部分の数字を調節することで出力を制限可能。
  if (Rate > 1.0f) Rate = 1.0f;
  // Serial.print("Rate: ");
  // Serial.print(Rate);
  for (int i=0; i<4; i++){ motorPower[i] *= Rate; }
  // Serial.print(", A: ");
  // Serial.print(motorPower[0]);
  // Serial.print(", B: ");
  // Serial.print(motorPower[1]);
  // Serial.print(", C: ");
  // Serial.print(motorPower[2]);
  // Serial.print(", D: ");
  // Serial.println(motorPower[3]);
  MotorON(WHEEL_A, motorPower[0]); // OMNI - A
  MotorON(WHEEL_B, motorPower[1]); // OMNI - B
  MotorON(WHEEL_C, motorPower[2]); // OMNI - C
  MotorON(WHEEL_D, motorPower[3]); // OMNI - D
}

//射出角度の制御
void Pitch(void){
  //コントローラからの信号をサーボモータの角度に変換する
  long d = AS_Vol;
  d /= 2.83; //★AS_Vol の値(0～255) を 0～90に変換
  pitchangle = (int)d;
  ServoON(PITCH, pitchangle);  //サーボモータに角度を指令
}

// 射出用Util
void ShotRollerControl(void){
  if( RollerOnOff == 1 && ((millis() - RollerTime) > 1000) ){
    //射出ボタンが押され，ローラーが回っているとき
    if(((millis() - ShotTime) > 500) && (Shotmove == 0)){
      ServoON(SHOT, shotangle);   //サーボモータ射出位置設定
      ShotTime = millis();        //サーボモータ動作開始時間記録
      Shotmove = 1;               //射出位置動作記録
      //Serial.println("Shot");
    }else{
      //Serial.println("Waiting fot Shot delay");
    }
  }
}

void ShotIdle(void){
  if((Shotmove == 1) && ((millis() - ShotTime) > 500)){
    //射出位置動作中に，500msec経過後
    ServoON(SHOT, waitangle);     //サーボモータ待機位置設定
    ShotTime = millis();          //サーボモータ動作開始時間記録
    Shotmove = 0;                 //待機位置動作記録
    //Serial.println("Return");
  }else{
    //Serial.println("Waiting fot Shot delay");
  }
}

//射出シーケンス
void Shot(void){
  if (SW_SHOT && Shotmove == 0){
    ShotRollerControl();
  }
  ShotIdle();
}

//ローラー回転のシーケンス
// CurrentRollerValue += 10;
// CurrentRollerValue = min(CurrentRollerValue, 100);
void Roller(void){
  if(SW_ROLLER && ((millis() - RollerTime) > 500)){ //押した検出　チャタリング防止で500ms間は連打できない
    RollerSeq = 1;                  //ローラー回転シーケンスを1に
    RollerTime = millis();          //ローラー回転開始時間記録
    delay(50);
  }else if((!SW_ROLLER) && (RollerSeq == 1)){       //離した検出
    RollerSeq = 2;                  //ローラー回転シーケンスを2に
    delay(50);
  }else if(RollerSeq == 2){         //ローラー回転/停止処理
    if(RollerOnOff == 0){           //ローラー停止中の時
      RollerOnOff = 1;              //ローラー回転中に設定
    }else{                          //ローラー回転中の時
      RollerOnOff = 0;              //ローラー停止中に設定
    }
    RollerSeq = 0;                  //ローラー回転シーケンスをリセット
  }
  if (RollerOnOff == 0){
    CurrentRollerValue = 0;
  }
  else {
    CurrentRollerValue += 10;
    CurrentRollerValue = min(CurrentRollerValue, 100);
  }
  ServoON(ROLLER, CurrentRollerValue); // ローラーループ
}


