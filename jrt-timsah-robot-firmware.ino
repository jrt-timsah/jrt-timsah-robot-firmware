/*
このファームウェアは以下の仕様のCoRE-2ベースロボット用である
Arduinoシールド ：Ver.2024
  → define.h と init.ino の両方がVer.2024用であることを確認せよ
射出用モータ    ：サーボ制御
  → motor.ino 内 servo2 の設定を確認せよ
射出用ローラ    ：PWM制御
  → motor.ino 内 servo2 の設定を確認せよ
コントローラ基板：Ver.2
  → メインプログラムの入力割付設定用define文を確認せよ
  → controller.ino がVer.2用であることを確認せよ
*/

#include <Servo.h>
#include <string.h>
#include <SPI.h>
#include "define.h"
#include <math.h>

#define BITRATE 115200

//====================
// コントローラ入力割付（Ver.2）
//====================
#define SW_ENABLE ((RxData[4] & 0b000001) >> 0) //SW1 操作禁止
#define SW_SHOT   ((RxData[4] & 0b000010) >> 1) //SW2 ディスク押出
#define SW_ROLLER ((RxData[4] & 0b000100) >> 2) //SW3 ローラー回転
#define SW4       ((RxData[4] & 0b001000) >> 3) //SW4 予備
#define SDA       ((RxData[4] & 0b010000) >> 4) //予備
#define SCL       ((RxData[4] & 0b100000) >> 5) //予備
#define AS_LeftX  (RxData[0])                   //A1 左スティックのX
#define AS_LeftY  (RxData[1])                   //A2 左スティックのY
#define AS_RightX (RxData[2])                   //A3 右スティックのX
#define AS_Vol    (RxData[3])                   //A4 角度調整

//====================
// モータ・サーボ割付
//====================
#define WHEEL_L MOTOR1
#define WHEEL_R MOTOR2
#define ROLLER  SERVO3
#define PITCH   SERVO1
#define SHOT    SERVO2

//====================
// ロボット側入力
//====================
#define EMG_Stop digitalRead(SW12)  //非常停止

//====================
// setup
//====================
void setup() {

  Serial.begin(BITRATE);     // PC通信用
  Serial1.begin(BITRATE);    // コントローラ用
  Serial2.begin(BITRATE);    // オートレフェリ用

  Init();                    // init.ino
  InitMotor();               // init.ino

  // サーボ初期位置
  ServoON(SHOT, waitangle);  // 射出サーボ待機
  ServoON(SERVO3, 0);        // 射出ローラーBrushless待機

  delay(1000);

  BuzzerInitOK();
  Serial.println("Init OK");
}

//====================
// loop
//====================
void loop() {

  // 周期固定用
  StartTime = millis();

  RxController();     // controller.ino

  // 操作可能判定
  if ((AF_Signal1 == 0) && (SW_ENABLE == 1) && !ControllerTimeout) {
    OperationReady = 1;
  } else {
    OperationReady = 0;
  }

  SensorDebugLED();

  //====================
  // 操作可能時
  //====================
  if (OperationReady == 1) {

    Wheel();     // 走行
    Roller();    // ローラー制御（ここで RollerOnOff が決まる）
    Pitch();   // ピッチサーボ
    // ===== 重要 =====
    // ローラーが回っている時だけサーボを許可
    if (RollerOnOff == true) {
      Shot();    // 射出サーボ
    } else {
      // ローラー停止中はサーボを待機状態に固定
      ShotSeq  = 0;
      Shotmove = 0;
      ServoON(SHOT, waitangle);
    }

    BuzzerOFF();

  }
  //====================
  // 操作不可時
  //====================
  else {

    RollerOnOff = false;
    ShotSeq  = 0;
    Shotmove = 0;

    MotorAllOFF();
    ServoON(SHOT, waitangle);
    ServoON(ROLLER, 0);

    if (AF_Signal1 == 1) {
      Buzzer1ON();
    }
  }

  // 周期固定（オーバーしたら無視）
  while ((millis() - StartTime) < period) {
    delayMicroseconds(10);
  }
}

