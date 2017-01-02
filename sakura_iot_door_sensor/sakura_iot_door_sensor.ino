#include <SakuraIO.h>

SakuraIO_I2C sakuraio;

// 接続するセンサの数
const int numSensors = 3;

// ドアセンサを接続するArduino上のピン
const int connectedPins[] = {2,3,4};

// 上記sensorPinに該当するチャンネル
const int dataChannels[] = {0,1,2};

// 上記sensorPinの前回の値 (0:ドア開放 1:ドア閉扉）
int prevStatus[numSensors];

void setup() {

  // さくらIoTのセットアップ
  Serial.begin(9600);
  Serial.print("Waiting to come online");
  for(;;){
    if( (sakuraio.getConnectionStatus() & 0x80) == 0x80 ) break;
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  
  uint8_t request[33] = {};
  uint8_t response[33] = {};


  // ProductID
  uint16_t pid = sakuraio.getProductID();
  Serial.print("PID ");
  Serial.println(pid);

  // UniqueID
  sakuraio.getUniqueID((char *)response);
  Serial.print("UID ");
  Serial.println((char *)response);

  // Version
  sakuraio.getFirmwareVersion((char *)response);
  Serial.print("Ver ");
  Serial.println((char *)response);

  // Connection Status
  uint8_t connectionStatus = sakuraio.getConnectionStatus();
  Serial.print("Status ");
  Serial.println(connectionStatus);

  //getSignalQuarity
  uint8_t signalQuarity = sakuraio.getSignalQuarity();
  Serial.print("Quality ");
  Serial.println(signalQuarity);


  // センサーピンの設定
  // Pullup抵抗を接続するのが面倒なので、INPUTではなくINPUT_PULLUPを設定。
  // ただしOn/OffとHIGH/LOWが逆になる点に注意。
  
  for(int i =0; i < numSensors; i++){
    pinMode(connectedPins[i], INPUT_PULLUP);
  }
  
}


void loop() {

  // Unixtime
  uint32_t unixtime = (uint32_t)(sakuraio.getUnixtime()/1000UL);
  Serial.print("Unixtime ");
  Serial.println(unixtime);

  // Tx Queue
  uint8_t ret;
  
 int currentPinStatus;

for(int i =0; i < numSensors; i++){
    // 現時点のドアセンサの状態を取得
    // INPUT_PULLUPなので、HIGH=off(0), LOW=on(1)
    if( digitalRead(connectedPins[i])== HIGH) {
      currentPinStatus = 0;
    }else{
      currentPinStatus = 1;
    }

    // もし前回の値と異なっていたら、queueに登録
    if( prevStatus[i] != currentPinStatus) {
      prevStatus[i] = currentPinStatus;
      Serial.print("Enqueue (Pin");
      Serial.print(connectedPins[i]);
      Serial.print(") ");
      Serial.println(currentPinStatus);
      
      ret = sakuraio.enqueueTx((uint8_t)dataChannels[i], (int32_t)currentPinStatus);
      Serial.println(ret);
    }
  }

  uint8_t avail;
  uint8_t queued;
  sakuraio.getTxQueueLength(&avail, &queued);
  Serial.print("Tx Available=");
  Serial.print(avail);
  Serial.print(" Queued=");
  Serial.println(queued);

  // queue に溜まっていたら送信
  if(queued > 0){
    ret = sakuraio.send();
    Serial.print("Send ");
    Serial.println(ret);
  }

  delay(5000);
}
