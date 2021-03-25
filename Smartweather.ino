#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"
#include "Tone32.h"
#include "music_joy.h"

uint8_t time_data = 0;
uint8_t temp_val,hum_val;
hw_timer_t *timer = NULL;
//static void IRAM_ATTR Timer0_CallBack(void);

#define SPEARKER_PIN  A4
/*配置WIFI名和密码*/
const char * WIFI_SSID     = "note10";
const char * WIFI_PASSWORD = "1234567890";

/*配置设备证书信息*/
String ProductKey = "a1e8bfTKTge";
String ClientId = "12345";
String DeviceName = "ds8RNC8XetPHDnF5WDud";
String DeviceSecret = "ebaee18fb916f20247bd2b1f981aaba3";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*需要操作的产品标识符*/
String Identifier = "mymusic";
String temp_Identifier = "temp";
String hum_Identifier = "hum";

/*需要订阅的TOPIC*/
const char * subTopic = "/sys/a1e8bfTKTge/ds8RNC8XetPHDnF5WDud/thing/service/property/set";//****set
const char * pubTopic = "/sys/a1e8bfTKTge/ds8RNC8XetPHDnF5WDud/thing/event/property/post";

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t beginPlay = 0;
uint8_t playWitchOne = 0;

/*播放凉凉*/
static void playLiangLiang(){
  for (int i = 0; i < sizeof(liangliangData)/sizeof(liangliangData[0]); i++) { 
    int noteDuration = liangliangDurations[i] *600;
    tone(SPEARKER_PIN, liangliangData[i],noteDuration); 
  }
  noTone(SPEARKER_PIN);
} 

/*播放天空之城*/
static void playCastleInTheSky(){
  for (int i = 0; i < sizeof(CastleInTheSkyData)/sizeof(CastleInTheSkyData[0]); i++) { 
    int noteDuration = CastleInTheSkyDurations[i] *600;
    tone(SPEARKER_PIN, CastleInTheSkyData[i],noteDuration); 
  }
  noTone(SPEARKER_PIN);
}

static void playMusic(){
  if(playWitchOne == 0){
    playLiangLiang();
  }else{
    playCastleInTheSky();
  }
}

void connectWiFi(){
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Adderss: ");
  Serial.println(WiFi.localIP());
}
char tt[300],locatal_temp;
uint8_t buff_temp[10];
uint8_t buff_hum[10];
void callback(char * topic, byte * payload, unsigned int len){
  Serial.print("Recevice [");
  Serial.print(topic);
  Serial.println("] ");
  for (int i = 0; i < len; i++){
    Serial.print((char)payload[i]);
  }
  int k = 0, x = 0, j = 0, y = 0;
  while(payload[k] != '\0')
  {
    if(payload[k] == 't' || payload[k+1] == 'e'|| payload[k+2] == 'm' || payload[k+3] == 'p')
    {
        break;
      }
    }
    while(payload[j] != '\0')
  {
    if(payload[j] == 'h' || payload[j+1] == 'u'|| payload[j+2] == 'm')
    {
        break;
      }
    }
   
   while(payload[k+7] != '\"')
   {  buff_temp[x++] = payload[(k+7)+1];
      k++;
    } 
   buff_temp[x++] = '\0';
   while(payload[j+11] != '\"')
   {  buff_hum = payload[(j+7)+1];
      temp_val = (payload[(j+7)+1] - '0') * 10+ temp_val
      temp_val = (payload[(j+7)+1] - '0') * 10+ temp_val
      j++;
    } 
    buff_hum[y++] = '\0';
//  StaticJsonBuffer<300> jsonBuffer;
//  JsonObject& root = jsonBuffer.parseObject((const char*)payload);
//  if(!root.success()){
//    Serial.println("parseObject() failed");
//    return;
//  }
//  playWitchOne = root["params"][Identifier];
//  Serial.print("playWitchOne=");
//  Serial.println(playWitchOne);
Serial.print(buff_temp);
Serial.print(buff_hum);
  
  beginPlay = 1;
}

void ConnectAliyun(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    /*根据自动计算的用户名和密码连接到Alinyun的设备，不需要更改*/
    if(client.connect(myAliyun.client_id,myAliyun.username,myAliyun.password)){
      Serial.println("connected");
      client.subscribe(subTopic);
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("start\n");
   /*连接WIFI*/
  connectWiFi();
  
  /*初始化Alinyun的配置，可自动计算用户名和密码*/
  myAliyun.init(ALIYUN_SERVER,ProductKey,ClientId,DeviceName,DeviceSecret);
  
  client.setServer(myAliyun.mqtt_server,PORT);
  
  /*设置回调函数，当收到订阅信息时会执行回调函数*/
  client.setCallback(callback);
  /*连接到Aliyun*/
  ConnectAliyun();
}
uint8_t tempTime = 0;
void loop(){
  if(!client.connected()){
    ConnectAliyun();
  }
    /*一分钟上报两次温湿度信息*/
  if(tempTime > 60){
    tempTime = 0;
    client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+temp_Identifier+"\":"+temp_val+",\""+hum_Identifier+"\":"+hum_val+"},\"method\":\"thing.event.property.post\"}").c_str());
  }else{
    tempTime++;
    delay(500);
  }
  if(beginPlay == 1){
    playMusic();
    beginPlay =0;
  }
  client.loop();
}

