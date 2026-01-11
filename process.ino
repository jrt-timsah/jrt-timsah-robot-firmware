/*
【定義】
　SW_SHOT：射出ボタン．押すと1，離すと0になる．
　SW_ROLLER：ローラー回転ボタン．押すと1，離すと0になる．

【変数】
RxData[0]~RxData[3]:コントローラから受信したデータが入った配列
  AS_Left ：左スティックのアナログ量(-100～100)(配列：RxData[0])
  AS_Right：右スティックのアナログ量(-100～100)(配列：RxData[1])
  AS_Vol  ：ボリュームのアナログ量(0～255)(配列：RxData[2])
ShotSeq：射出のシーケンス管理用変数
RollerSeq：ローラー回転シーケンスの管理用変数
RollerOnOff：ローラー回転状態の管理用変数

【関数】
MotorON(motor, pwm)：モータを回すための関数　motor：回すモータ　pwm：-100~100
  WHEEL_L：左タイヤ
  WHEEL_R：右タイヤ
  ROLLER ：射出ローラー
  例)　MotorON(WHEEL_L, -50);
MotorOFF(motor)：モータを停止するための関数　motor：停止するモータ

ServoON(servo, angle)：サーボを動かすための関数　servo：動かすサーボ　angle：90~180
  PITCH ：射出角度調整用のサーボ
  SHOT  ：ディスク射出用のサーボ
  例)　ServoON(PITCH, 100);
  
*/
// シリアルモニタ検証用コード
// 上手くいったら色んなコードに仕込む予定
// 基盤をつなげられたら試そう！！！！
void JapaneseSerialMessageTest(){
  Serial.println("Shot");
}
// 足回りの制御
void Wheel(void){
  //★右タイヤの制御 (AS_Right が -10 未満、+10 より大きい時に動作)
  if(AS_Right < -20 || AS_Right > 20){
    MotorON(WHEEL_R, AS_Right);
  }
  else{
    MotorOFF(WHEEL_R);
  }
  //★左タイヤの制御 (AS_Left が -10 未満、+10 より大きい時に動作)
  if(AS_Left < -20 || AS_Left > 20){
    MotorON(WHEEL_L, AS_Left);
  }
  else{
    MotorOFF(WHEEL_L);
  }
}

//射出角度の制御
void Pitch(void){
  //コントローラからの信号をサーボモータの角度に変換する
  pitchangle = 2.5*AS_Vol-342.5; //★AS_Vol の値(-100～100) を 0～90に変換
  ServoON(SERVO1, pitchangle);  //サーボモータに角度を指令
}

// 射出用Util
void ShotRollerControl(void){
  /*
  if((RollerOnOff == 0) && (ShotSeq == 0)){
    //射出ボタンが押され，ローラーが回っていないとき
    //motor[ROLLER].TxVel = 15000;//ローラー速度に15000を設定 (CAN制御)
    //MotorON(ROLLER, 50);          //ローラー速度に50[%]を設定(PWM制御) // [80 -> 50]に変更中
    RollerTime = millis();        //ローラー回転開始時間記録
    ShotSeq = 1;                  //射出シーケンスを1に
  }else if((ShotSeq == 1) && ((millis() - RollerTime) > 1000)){
    //射出シーケンスが1になり，1000msec経過後
    RollerOnOff = 1;              //ローラー回転状態に設定
    ShotSeq = 0;                  //射出シーケンスを0に
  }else */
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
  waitangle = 30;   //待機(装填)位置 90～110程度で調整
  shotangle = 160;  //射出位置 160～180程度で調整
  //Serial.println("SW_SHOT");
  //Serial.println(SW_SHOT);

  if (SW_SHOT && Shotmove == 0){
    ShotRollerControl();
  }
  ShotIdle();
}

//ローラー回転のシーケンス
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
      MotorON(ROLLER, 50);          //ローラー速度に50[%]を設定(PWM制御)
      RollerOnOff = 1;              //ローラー回転中に設定
    }else{                          //ローラー回転中の時
      MotorOFF(ROLLER);             //ローラー停止を指令(PWM制御)
      RollerOnOff = 0;              //ローラー停止中に設定
    }
    RollerSeq = 0;                  //ローラー回転シーケンスをリセット
  }
}
