/*
このモータ設定ファイルはArduinoシールドVer.2024用である
PWM制御 ： MOTOR1 ~ MOTOR8
CAN制御 ： 無し
SERVO   ： SERVO1 ~ SERVO3
*/
int8_t Dir[10][2] = {};
Servo servo1, servo2, servo3;

void InitMotor(void){
  pinMode(DIR1,  OUTPUT);
  pinMode(DIR2,  OUTPUT);
  pinMode(DIR3,  OUTPUT);
  pinMode(DIR4,  OUTPUT);
  pinMode(DIR5,  OUTPUT);
  pinMode(DIR6,  OUTPUT);
  pinMode(DIR7,  OUTPUT);
  pinMode(DIR8,  OUTPUT);

  //PWMピンはピンモードによる初期化は不要

  Dir[MOTOR1][0] = DIR1;
  Dir[MOTOR2][0] = DIR2;
  Dir[MOTOR3][0] = DIR3;
  Dir[MOTOR4][0] = DIR4;
  Dir[MOTOR5][0] = DIR5;
  Dir[MOTOR6][0] = DIR6;
  Dir[MOTOR7][0] = DIR7;
  Dir[MOTOR8][0] = DIR8;

  //ここで正転逆転を変える
  Dir[MOTOR1][1] = -1;  //左タイヤ
  Dir[MOTOR2][1] = 1;   //右タイヤ
  Dir[MOTOR3][1] = 1;   //射出ローラー
  Dir[MOTOR4][1] = 1;
  Dir[MOTOR5][1] = 1;
  Dir[MOTOR6][1] = 1;
  Dir[MOTOR7][1] = 1;
  Dir[MOTOR8][1] = 1;

  servo1.attach(SERVO1, 1000, 2000);    //ピッチ用
  servo2.attach(SERVO2, 1000, 2000);    //射出用
  servo3.attach(SERVO3, 1000, 2000);    //射出ローラーBrushless
}

/********  DC Motor  ********/
//pwmは-100～100％
void MotorON(uint8_t motor, int pwm){
  
  if((pwm*Dir[motor][1]) > 0){
    digitalWrite(Dir[motor][0], HIGH);
    analogWrite(motor, abs(255*pwm/100));
  }else if((pwm*Dir[motor][1]) < 0){
    digitalWrite(Dir[motor][0], LOW);
    analogWrite(motor, abs(255*pwm/100));
  }else{
    analogWrite(motor, 0);
    digitalWrite(Dir[motor][0], !digitalRead(Dir[motor][0]));
  }
  
}
void MotorOFF(uint8_t motor){
  digitalWrite(Dir[motor], !digitalRead(Dir[motor]));
  // digitalWrite(Dir[motor], LOW);
  analogWrite(motor, 0);
}

void MotorAllOFF(void){
  MotorOFF(MOTOR1);
  MotorOFF(MOTOR2);
  MotorOFF(MOTOR3);
  MotorOFF(MOTOR4);
  MotorOFF(MOTOR5);
  MotorOFF(MOTOR6);
  MotorOFF(MOTOR7);
  MotorOFF(MOTOR8);
}


/********  Servo  ********/
void ServoON(uint8_t servo, uint8_t angle){
  if(servo == SERVO1){
    servo1.write(angle);
  }else if(servo == SERVO2){
    servo2.write(angle);
  }else if(servo == SERVO3){
    servo3.write(angle);
  }
}


/********  RM Motor  ********
void RMmotorRxData(void){
  for(int i = 0; i < RMmotorNUM; i++){
    if (mcp2515.readMessage(&MotorRxData) == MCP2515::ERROR_OK) {
        // IDごとに入れる配列を変える
        // 0x201：モータ1　～　0x208：モータ8
        if ((MotorRxData.can_id > 0x0200) && (MotorRxData.can_id < 0x020F)){
          // 角度，速度，電流，温度が取れる(速度と電流のみ取得)
          motor[MotorRxData.can_id - 0x0201].RxVel = MotorRxData.data[2] << 8 | MotorRxData.data[3];
          motor[MotorRxData.can_id - 0x0201].RxAmp = MotorRxData.data[4] << 8 | MotorRxData.data[5];
        }
    }
  }
}

//速度→指令電流への変換とPD制御
void VelToAmp(void){
  double e;

  // ローラ
  if(motor[ROLLER].RxAmp < LimitAmp[ROLLER]){   // 一定以上の電流で停止
    e = motor[ROLLER].TxVel - motor[ROLLER].RxVel;
    motor[ROLLER].TxAmp = (int)(e*RollerP + ( e - PIDdiff[ROLLER])/(float)period*RollerD) ;
    PIDdiff[ROLLER] = e;
  }else{
    motor[ROLLER].TxAmp = 0;
    PIDdiff[ROLLER] = 0;
  }

  // クリップ
  for(int i = 0; i < RMmotorNUM; i++ ){
    if(motor[i].TxAmp > 16380){
      motor[i].TxAmp = 16380;
    }else if(motor[i].TxAmp < -16380){
      motor[i].TxAmp = -16380;
    }
  }
}

// モータへ送信
void RMmotorTxData(void){
  if(!OperationReady){
    for (int i = 0; i < RMmotorNUM; i++){
      motor[i].TxVel = 0;  //モータの指令リセット
    }
  }

  VelToAmp();
  
  //電流値を送信
  MotorTxData.can_id  = 0x0200;
  MotorTxData.can_dlc = 8;
  MotorTxData.data[0] = motor[0].TxAmp>>8;
  MotorTxData.data[1] = motor[0].TxAmp;
  MotorTxData.data[2] = motor[1].TxAmp>>8;
  MotorTxData.data[3] = motor[1].TxAmp;
  MotorTxData.data[4] = motor[2].TxAmp>>8;
  MotorTxData.data[5] = motor[2].TxAmp;
  MotorTxData.data[6] = motor[3].TxAmp>>8;
  MotorTxData.data[7] = motor[3].TxAmp;

  mcp2515.sendMessage(&MotorTxData);
  delay(1);

  MotorTxData.can_id  = 0x01FF;
  MotorTxData.can_dlc = 8;
  MotorTxData.data[0] = motor[4].TxAmp>>8;
  MotorTxData.data[1] = motor[4].TxAmp;
  MotorTxData.data[2] = motor[5].TxAmp>>8;
  MotorTxData.data[3] = motor[5].TxAmp;
  MotorTxData.data[4] = motor[6].TxAmp>>8;
  MotorTxData.data[5] = motor[6].TxAmp;
  MotorTxData.data[6] = motor[7].TxAmp>>8;
  MotorTxData.data[7] = motor[7].TxAmp;

  mcp2515.sendMessage(&MotorTxData);
}
*/
