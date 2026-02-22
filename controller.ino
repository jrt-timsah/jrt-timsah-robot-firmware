/*
この設定ファイルはコントローラ基板Ver.2用である
【定義】
　SW_ENABLE ：操作許可スイッチ．1で操作許可，0で操作不可．
　SW_SHOT   ：射出ボタン．押すと1，離すと0になる．
　SW_ROLLER ：ローラー回転ボタン．押すと1，離すと0になる．

【変数】
RxData[0]~RxData[3]:コントローラから受信したデータが入った配列
  AS_LeftX ：左スティックのアナログ量X(-100～100)(配列：RxData[0])
  AS_RightX：右スティックのアナログ量X(-100～100)(配列：RxData[1])
  AS_Vol  ：ボリュームのアナログ量(0～255)(配列：RxData[2])
*/

bool RxController(void){

  if(Serial1.available()){
    ControllerRxTime = millis();
    String StrData = Serial1.readStringUntil('\n');
    delay(10);
    StrData.remove(0,14);     //ダミー，ノードNo，RSSI，機能IDまで捨てる
    // Serial.println(StrData);
    
    for (int i = 0; i < 7; i++){
      int check = StrData.indexOf(',');             //最初のカンマの位置確認
      String SubData = StrData.substring(0, check); //先頭のカンマまでを取得
      StrData = StrData.substring(check+1);         //データ文字列から先頭を削除

      char temp[5];
      SubData.toCharArray(temp, 5);
      RxData[i] = (int)(strtol(temp, NULL, 16));
    }
    
    //元々-100～100だったのが0～255に変更になったのでここで変換
    AS_LeftX -= 127; 
    AS_LeftX = -1*RxData[0]*100/127;//コントローラーの配線の都合上-1をかけて左右反転させています
    AS_LeftY -= 127;
    AS_LeftY = RxData[1]*100/127;
    AS_RightX -= 127;
    AS_RightX = -1*RxData[2]*100/127;//コントローラーの配線の都合上-1をかけて左右反転させています


    Serial.print("AS_LeftX =  ");
    Serial.print(AS_LeftX);
    Serial.print(", AS_LeftY = ");
    Serial.print(AS_LeftY);
    Serial.print(", AS_RightX = ");
    Serial.print(AS_RightX);
    Serial.print(", AS_Vol = ");
    Serial.print(AS_Vol);
    Serial.print(", SW_ENABLE = ");
    Serial.print(SW_ENABLE);
    Serial.print(", SW_SHOT = ");
    Serial.print(SW_SHOT);
    Serial.print(", SW_ROLLER = ");
    Serial.print(SW_ROLLER);
    Serial.print(", OperationReady = ");
    Serial.println(OperationReady);


    //Serial.print(", RxData[0]= ");
    //Serial.print(RxData[0]);
    //Serial.print(", RxData[1]= ");
    //Serial.print(RxData[1]);
    //Serial.print(", RxData[2]= ");
    //Serial.print(RxData[2]);
    //Serial.print(", RxData[3]= ");
    //Serial.print(RxData[3]);
    //Serial.print(", RxData[4]= ");
    //Serial.print(RxData[4]);
    //Serial.print(", RxData[5]= ");
    //Serial.print(RxData[5]);
    //Serial.print(", RxData[6]= ");
    //Serial.println(RxData[6]);

    return ControllerTimeout = false;

  }else if((millis() - ControllerRxTime) > 300){  //タイムアウト
    for (int i = 0; i < 7; i++){
      RxData[i] = 0;
    }
    return ControllerTimeout = true;
  }
}