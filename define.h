/*
このヘッダファイルはArduinoシールドVer.2024用である
入力SW  ： 1 ~12
LED     ： 1 ~12
SERVO   ： 1 ~ 3
*/
#include <stdint.h>
#ifndef _INIT_H_
#define _INIT_H_

#define LED1  34
#define LED2  35
#define LED3  36
#define LED4  37
#define LED5  38
#define LED6  39
#define LED7  40
#define LED8  41
#define LED9  42
#define LED10 43
#define LED11 44
#define LED12 45


#define SW1   22
#define SW2   23
#define SW3   24
#define SW4   25
#define SW5   26
#define SW6   27
#define SW7   28
#define SW8   29
#define SW9   30
#define SW10  31
#define SW11  32
#define SW12  33


#define BUZZER  10

#define MOTOR1  9
#define MOTOR2  8
#define MOTOR3  7
#define MOTOR4  6
#define MOTOR5  5
#define MOTOR6  4
#define MOTOR7  3
#define MOTOR8  2

#define DIR1  54
#define DIR2  55
#define DIR3  56
#define DIR4  57
#define DIR5  58
#define DIR6  59
#define DIR7  60
#define DIR8  61

#define SERVO1 13

// SERVO2,3に問題あり。現在defineを古いほうに切り替えてます。変更するときは両defineのコメント状態を逆にすること。
#define SERVO2 12 // Old Define
#define SERVO3 11 // Too
//#define SERVO2 11 // New Define
//#define SERVO3 12 // Too

#define AF1 62
#define AF2 63

#define RMmotorNUM 8

/********  制御周期  ********/
const uint8_t period = 10;    // 制御周期
uint64_t StartTime = 0;       // 周期固定用　開始時間

/********  コントローラ  ********/
int RxData[10] = {};   //受信したデータをセミコロン毎に分割
uint64_t ControllerRxTime = 0;  //タイムアウト用で使用予定
uint8_t OperationReady = 0;     //操作禁止orオートレフェリでON
bool ControllerTimeout = true;  //タイムアウト検知戻り値

/********  オートレフェリ  ********/
#define AF_Signal1 digitalRead(AF1)     //オートレフェリからの停止信号

/********  射出  ********/
bool Shotmove = 1;            //射出動作中信号
uint8_t ShotSeq = 0;          //射出動作シーケンスカウント用
uint8_t RollerSeq = 0;        //チャタリング＆連打対策用
uint8_t RollerOnOff = 0;      //ローラーON/OFFの切り替え記憶用
uint64_t RollerTime = 0;      //ローラースイッチのチャタリング対策
unsigned long ShotTime = 0;   //射出動作シーケンスの静定時間設定用
unsigned int ShotAngle;       //射出用サーボモータ角度
unsigned int waitangle = 90;  //射出用サーボモータ待機位置
unsigned int shotangle;       //射出用サーボモータ射出位置
unsigned int pitchangle = 0;  //ピッチ用サーボモータ角度

/********  RMモータ制御に関する構造体  ********
typedef struct RMmotor {

  // 送信するデータ
  int32_t TxVel; //速度[rpm]
  int32_t TxAmp; //電流[mA]

  // 受信するデータ
  int32_t RxVel; //速度[rpm]
  int32_t RxAmp; //電流[mA]
} RMmotor;
RMmotor motor[RMmotorNUM] = {};  // 各モータのデータ
uint16_t LimitAmp[RMmotorNUM] = {5000, 5000}; //電流の制限

const uint8_t ROLLER = 1;

/********  CAN  ********
struct can_frame MotorTxData; // CANでモータに送信するデータ
struct can_frame MotorRxData; // CANでモータから受信するデータ

/********  PID  ********
float RollerP = 0.35;    // ローラ用のPゲイン
float RollerI = 0.0;     // ローラ用のIゲイン
float RollerD = 0.1;     // ローラ用のDゲイン

float PIDdiff[RMmotorNUM] = {};  // D制御用の過去の誤差量
*/
#endif
