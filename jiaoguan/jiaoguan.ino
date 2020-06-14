#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Arduino.h"
#include "PCF8574.h"
#include  <Ticker.h>//Ticker Library

//blinker.attach(0.5, CallTime);
//blinker.detach();

Ticker blinker;
//blinker.attach(0.5, CallTime);
//blinker.detach();
int settime=0;//定时时间
int timeadd=0;//定时器计数
bool timeflag = false;//定时结束标志位

bool updatasta = false;//数据更新标志

//int delayt = 1000;

//设置故障工作指示灯
int run_led = 16;     // GPIO16

//设置i方c线路引脚
uint8_t sda = D6;
uint8_t scl = D5;

// 设置i2c十六进制地址
PCF8574 pcf8574_1(0x20,sda,scl);
PCF8574 pcf8574_2(0x21,sda,scl);

const char* mqtt_server = "mq.tongxinmao.com";//服务器的地址 
const int port=18831;//服务器端口号

WiFiClient espClient;
PubSubClient client(espClient);
//unsigned long lastMsg = 0;
//#define MSG_BUFFER_SIZE  (50)
//char msg[MSG_BUFFER_SIZE];
//int value = 0;
//
int count=0;
bool WIFI_Status = true;

void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  digitalWrite(run_led, HIGH);
  WiFi.beginSmartConfig();//等待手机端发出的用户名与密码
  digitalWrite(run_led, LOW);
  while (1)
  {
    Serial.print(".");  
    digitalWrite(run_led, HIGH);
    delay(500);
    digitalWrite(run_led, LOW);
    delay(500);                    
    if (WiFi.smartConfigDone())//退出等待
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}

//定时器回调函数
void CallTime()
{
  //digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
  timeadd++;
  if(timeadd == settime)
  {
    updatasta = true;//需要更新
    if(pcf8574_1.digitalRead(P7))
      {
        pcf8574_1.digitalWrite(P7, LOW);
        //delay(3000);
        timeflag = true;
      }
  }
}

//MQTT消息回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += String((char)payload[i]);
  }
  String top = String(topic);
  //打印主题和消息
  Serial.print("Message arrived [");
  Serial.print(top);
  Serial.print("] ");
  Serial.print(msg);
  Serial.println();

  updatasta = true;//需要更新
//uint8_t val = pcf8574.digitalRead(P1);
  // 判断接收到什么主题
  if (top == "control_1")
  {
    if (msg == "pump_on") 
    {//1主干和阀门有开或2主干和阀门有开
      if (((pcf8574_1.digitalReadAll()&0x00ff)>0x40||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)&&settime>0)
      {
        pcf8574_1.digitalWrite(P7, HIGH);
        blinker.attach(60, CallTime);
        timeflag = 0;
      }
      //Serial.print((~pcf8574_1.digitalReadAll())&0x00ff);  
    }
    else if (msg == "pump_off") {
      pcf8574_1.digitalWrite(P7, LOW); 
      blinker.detach();
    }
    else if (msg == "trunk_on") {
      pcf8574_1.digitalWrite(P6, HIGH); 
    }
    else if (msg == "trunk_off") {
      //水泵没开或第二块PCF开了主干和阀门
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P6, LOW);
      }
    }
    else if (msg == "ch1_on") {
      pcf8574_1.digitalWrite(P5, HIGH);  
    }
    else if (msg == "ch1_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xe0||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P5, LOW);
      }
    }
    else if (msg == "ch2_on") {
      pcf8574_1.digitalWrite(P4, HIGH);  
    }
    else if (msg == "ch2_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xd0||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P4, LOW);
      }
    }
    else if (msg == "ch3_on") {
      pcf8574_1.digitalWrite(P3, HIGH);    
    }
    else if (msg == "ch3_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc8||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P3, LOW);
      }   
    }
    else if (msg == "ch4_on") {
      pcf8574_1.digitalWrite(P2, HIGH); 
    }
    else if (msg == "ch4_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc4||(pcf8574_2.digitalReadAll())&0x00ff>0x80)
      {
        pcf8574_1.digitalWrite(P2, LOW);
      } 
    }
    else if (msg == "ch5_on") {
      pcf8574_1.digitalWrite(P1, HIGH); 
    }
    else if (msg == "ch5_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc2||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P1, LOW);
      }     
    }
    else if (msg == "ch6_on") {
      pcf8574_1.digitalWrite(P0, HIGH);    
    }
    else if (msg == "ch6_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc1||(pcf8574_2.digitalReadAll()&0x00ff)>0x80)
      {
        pcf8574_1.digitalWrite(P0, LOW);
      }     
    }
    else if (msg == "all_on") {
      pcf8574_1.digitalWrite(P1, HIGH);
      pcf8574_1.digitalWrite(P2, HIGH);
      pcf8574_1.digitalWrite(P3, HIGH);
      pcf8574_1.digitalWrite(P4, HIGH);
      pcf8574_1.digitalWrite(P5, HIGH);
      pcf8574_1.digitalWrite(P6, HIGH);
      pcf8574_1.digitalWrite(P0, HIGH);
    }
    else if (msg == "all_off") {
      if(pcf8574_1.digitalRead(P7))
      {
        if ((pcf8574_2.digitalReadAll()&0x00ff)<0x80)
        {
          pcf8574_1.digitalWrite(P7, LOW);
          delay(3000);
        }
      }
      pcf8574_1.digitalWrite(P1, LOW);
      pcf8574_1.digitalWrite(P2, LOW);
      pcf8574_1.digitalWrite(P3, LOW);
      pcf8574_1.digitalWrite(P4, LOW);
      pcf8574_1.digitalWrite(P5, LOW);
      pcf8574_1.digitalWrite(P6, LOW);
      pcf8574_1.digitalWrite(P0, LOW);
      
      //定时器也要关
      blinker.detach();
      settime = 0;
      timeadd = 0; 
    }
  }
  else if (top == "control_2"){
    if (msg == "trunk_on") {
      pcf8574_2.digitalWrite(P7, HIGH);  
    }
    else if (msg == "trunk_off") {
      //水泵没开或第一块PCF开了主干和阀门
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0)
      {
        pcf8574_2.digitalWrite(P7, LOW);
      }
    }
    else if (msg == "ch1_on") {
      pcf8574_2.digitalWrite(P6, HIGH);  
    }
    else if (msg == "ch1_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0xc0)
      {
        pcf8574_2.digitalWrite(P6, LOW);
      }
    }
    else if (msg == "ch2_on") {
      pcf8574_2.digitalWrite(P5, HIGH); 
    }
    else if (msg == "ch2_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0xa0)
      {
        pcf8574_2.digitalWrite(P5, LOW);
      }
    }
    else if (msg == "ch3_on") {
      pcf8574_2.digitalWrite(P4, HIGH);  
    }
    else if (msg == "ch3_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0x90)
      {
        pcf8574_2.digitalWrite(P4, LOW);
      }   
    }
    else if (msg == "ch4_on") {
      pcf8574_2.digitalWrite(P3, HIGH); 
    }
    else if (msg == "ch4_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll())&0x00ff>0x88)
      {
        pcf8574_2.digitalWrite(P3, LOW);
      } 
    }
    else if (msg == "ch5_on") {
      pcf8574_2.digitalWrite(P2, HIGH); 
    }
    else if (msg == "ch5_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0x84)
      {
        pcf8574_2.digitalWrite(P2, LOW);
      }     
    }
    else if (msg == "ch6_on") {
      pcf8574_2.digitalWrite(P1, HIGH);  
    }
    else if (msg == "ch6_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0x82)
      {
        pcf8574_2.digitalWrite(P1, LOW);
      }     
    }
    else if (msg == "ch7_on") {
      pcf8574_2.digitalWrite(P0, HIGH);  
    }
    else if (msg == "ch7_off") {
      //水泵没开或1主干和其他阀门开了或2主干和阀门开了
      if (pcf8574_1.digitalRead(P7)==0||(pcf8574_1.digitalReadAll()&0x00ff)>0xc0||(pcf8574_2.digitalReadAll()&0x00ff)>0x81)
      {
        pcf8574_2.digitalWrite(P0, LOW);
      }     
    }
    else if (msg == "all_on") {
      pcf8574_2.digitalWrite(P7, HIGH);
      pcf8574_2.digitalWrite(P1, HIGH);
      pcf8574_2.digitalWrite(P2, HIGH);
      pcf8574_2.digitalWrite(P3, HIGH);
      pcf8574_2.digitalWrite(P4, HIGH);
      pcf8574_2.digitalWrite(P5, HIGH);
      pcf8574_2.digitalWrite(P6, HIGH);
      pcf8574_2.digitalWrite(P0, HIGH);
      
    }
    else if (msg == "all_off") {
      if(pcf8574_1.digitalRead(P7))
      {
        if ((pcf8574_1.digitalReadAll()&0x00ff)<0xc0)
        {
          pcf8574_1.digitalWrite(P7, LOW);
          delay(3000);
        }
      }
      pcf8574_2.digitalWrite(P7, LOW);
      pcf8574_2.digitalWrite(P1, LOW);
      pcf8574_2.digitalWrite(P2, LOW);
      pcf8574_2.digitalWrite(P3, LOW);
      pcf8574_2.digitalWrite(P4, LOW);
      pcf8574_2.digitalWrite(P5, LOW);
      pcf8574_2.digitalWrite(P6, LOW);
      pcf8574_2.digitalWrite(P0, LOW);
      //定时器也要关
      blinker.detach();
      settime = 0;
      timeadd = 0;
      
    }
  }
  else if (top == "settime"){
    settime = msg.toInt();
    timeflag = 0;
  }
}

void reconnect() {
  // 循环，直到重新连接MQTT服务器成功为止
  while (!client.connected()) {
    Serial.print("正在尝试连接MQTT服务器...");
    // 创建一个的客户端ID
    String clientId = "ESP8266Client1";
    // 尝试连接
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // 一旦连接上，发布主题…
      //client.publish("outTopic", "hello world");
      // 订阅主题
      client.subscribe("control_1");
      client.subscribe("control_2");
      client.subscribe("settime");
      
    } else {
      Serial.print("MQTT连接失败, rc=");
      Serial.print(client.state());
      Serial.println(" 5秒后再试一次");
      // 等待5秒钟再尝试
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);

  // 设置pcf8574_1的IO模式
   pcf8574_1.pinMode(P0, OUTPUT);
   pcf8574_1.pinMode(P1, OUTPUT);
   pcf8574_1.pinMode(P2, OUTPUT);
   pcf8574_1.pinMode(P3, OUTPUT);
   pcf8574_1.pinMode(P4, OUTPUT);
   pcf8574_1.pinMode(P5, OUTPUT);
   pcf8574_1.pinMode(P6, OUTPUT);
   pcf8574_1.pinMode(P7, OUTPUT);
   pcf8574_1.begin();

   // 设置pcf8574_2的IO模式
   pcf8574_2.pinMode(P0, OUTPUT);
   pcf8574_2.pinMode(P1, OUTPUT);
   pcf8574_2.pinMode(P2, OUTPUT);
   pcf8574_2.pinMode(P3, OUTPUT);
   pcf8574_2.pinMode(P4, OUTPUT);
   pcf8574_2.pinMode(P5, OUTPUT);
   pcf8574_2.pinMode(P6, OUTPUT);
   pcf8574_2.pinMode(P7, OUTPUT);
   pcf8574_2.begin();

    // 初始化pcf8574_1的IO电平
   pcf8574_1.digitalWrite(P7, LOW);
   pcf8574_1.digitalWrite(P0, LOW);
   pcf8574_1.digitalWrite(P1, LOW);
   pcf8574_1.digitalWrite(P2, LOW);
   pcf8574_1.digitalWrite(P3, LOW);
   pcf8574_1.digitalWrite(P4, LOW);
   pcf8574_1.digitalWrite(P5, LOW);
   pcf8574_1.digitalWrite(P6, LOW);
   

    // 初始化pcf8574_2的IO电平
   pcf8574_2.digitalWrite(P0, LOW);
   pcf8574_2.digitalWrite(P1, LOW);
   pcf8574_2.digitalWrite(P2, LOW);
   pcf8574_2.digitalWrite(P3, LOW);
   pcf8574_2.digitalWrite(P4, LOW);
   pcf8574_2.digitalWrite(P5, LOW);
   pcf8574_2.digitalWrite(P6, LOW);
   pcf8574_2.digitalWrite(P7, LOW);

  Serial.println("\r\n正在连接");
  while(WiFi.status()!=WL_CONNECTED)
  {
      if(WIFI_Status)
      {
          Serial.print(".");
          digitalWrite(run_led, HIGH);
          delay(1000);
          digitalWrite(run_led, LOW);
          delay(1000);               
          count++;
          if(count>=5)//5s
          {
              WIFI_Status = false;
              Serial.println("WiFi连接失败，请用手机进行配网"); 
          }
      }
      else
      {
          smartConfig();  //微信智能配网
      }
   }  
   Serial.println("WIFI连接成功");  
   Serial.print("IP:");
   Serial.println(WiFi.localIP());
   client.setServer(mqtt_server, port);
   client.setCallback(callback);//用于接收服务器接收的数据
   pinMode(run_led, OUTPUT);//设置故障灯模式

    // 初始化pcf8574_1的IO电平
   pcf8574_1.digitalWrite(P7, LOW);
   pcf8574_1.digitalWrite(P0, LOW);
   pcf8574_1.digitalWrite(P1, LOW);
   pcf8574_1.digitalWrite(P2, LOW);
   pcf8574_1.digitalWrite(P3, LOW);
   pcf8574_1.digitalWrite(P4, LOW);
   pcf8574_1.digitalWrite(P5, LOW);
   pcf8574_1.digitalWrite(P6, LOW);
   

    // 初始化pcf8574_2的IO电平
   pcf8574_2.digitalWrite(P0, LOW);
   pcf8574_2.digitalWrite(P1, LOW);
   pcf8574_2.digitalWrite(P2, LOW);
   pcf8574_2.digitalWrite(P3, LOW);
   pcf8574_2.digitalWrite(P4, LOW);
   pcf8574_2.digitalWrite(P5, LOW);
   pcf8574_2.digitalWrite(P6, LOW);
   pcf8574_2.digitalWrite(P7, LOW);
  
}
void loop() {
  if (!client.connected()) {
    //连接不上服务器，红色指示灯常亮，蓝灯灭
    digitalWrite(run_led, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    reconnect();
  }
  client.loop();

  //正常工作，红灯灭，蓝色指示灯闪烁
  digitalWrite(run_led, LOW);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  
  delay(500);                      


  //定时时间到，关闭所有阀门
  if(timeflag)
  {
    timeflag = false;
    delay(2500);
    pcf8574_1.digitalWrite(P1, LOW);
    pcf8574_1.digitalWrite(P2, LOW);
    pcf8574_1.digitalWrite(P3, LOW);
    pcf8574_1.digitalWrite(P4, LOW);
    pcf8574_1.digitalWrite(P5, LOW);
    pcf8574_1.digitalWrite(P6, LOW);
    pcf8574_1.digitalWrite(P0, LOW);
    
    pcf8574_2.digitalWrite(P7, LOW);
    pcf8574_2.digitalWrite(P1, LOW);
    pcf8574_2.digitalWrite(P2, LOW);
    pcf8574_2.digitalWrite(P3, LOW);
    pcf8574_2.digitalWrite(P4, LOW);
    pcf8574_2.digitalWrite(P5, LOW);
    pcf8574_2.digitalWrite(P6, LOW);
    pcf8574_2.digitalWrite(P0, LOW);
    //定时器也要关
    blinker.detach();
    settime = 0;
    timeadd = 0;
    updatasta = true;//需要更新
  }

  if (updatasta)
  {
    updatasta = false;
    // 发布主题…
    //client.publish("outTopic", "hello world");
    if (pcf8574_1.digitalRead(P7)) client.publish("sta_1", "pump_on");
    else client.publish("sta_1", "pump_off");
    if (pcf8574_1.digitalRead(P6)) client.publish("sta_1", "trunk_on");
    else client.publish("sta_1", "trunk_off");
    if (pcf8574_1.digitalRead(P5)) client.publish("sta_1", "ch1_on");
    else client.publish("sta_1", "ch1_off");
    if (pcf8574_1.digitalRead(P4)) client.publish("sta_1", "ch2_on");
    else client.publish("sta_1", "ch2_off");
    if (pcf8574_1.digitalRead(P3)) client.publish("sta_1", "ch3_on");
    else client.publish("sta_1", "ch3_off");
    if (pcf8574_1.digitalRead(P2)) client.publish("sta_1", "ch4_on");
    else client.publish("sta_1", "ch4_off");
    if (pcf8574_1.digitalRead(P1)) client.publish("sta_1", "ch5_on");
    else client.publish("sta_1", "ch5_off");
    if (pcf8574_1.digitalRead(P0)) client.publish("sta_1", "ch6_on");
    else client.publish("sta_1", "ch6_off");
  
    if (pcf8574_2.digitalRead(P7)) client.publish("sta_2", "trunk_on");
    else client.publish("sta_2", "trunk_off");
    if (pcf8574_2.digitalRead(P6)) client.publish("sta_2", "ch1_on");
    else client.publish("sta_2", "ch1_off");
    if (pcf8574_2.digitalRead(P5)) client.publish("sta_2", "ch2_on");
    else client.publish("sta_2", "ch2_off");
    if (pcf8574_2.digitalRead(P4)) client.publish("sta_2", "ch3_on");
    else client.publish("sta_2", "ch3_off");
    if (pcf8574_2.digitalRead(P3)) client.publish("sta_2", "ch4_on");
    else client.publish("sta_2", "ch4_off");
    if (pcf8574_2.digitalRead(P2)) client.publish("sta_2", "ch5_on");
    else client.publish("sta_2", "ch5_off");
    if (pcf8574_2.digitalRead(P1)) client.publish("sta_2", "ch6_on");
    else client.publish("sta_2", "ch6_off");
    if (pcf8574_2.digitalRead(P0)) client.publish("sta_2", "ch7_on");
    else client.publish("sta_2", "ch7_off");
  }
//  pcf8574_1.digitalWrite(P0, HIGH);
//  delay(500);
//  pcf8574_1.digitalWrite(P0, LOW);
//  delay(500);

}
