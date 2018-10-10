/**********************************************************************
 * 项目：bilibili关注数监视器
 * 硬件：NodeMCU ESP8266(10-25RMB) + MAX7219(4-8RMB)
 * 功能：连接WiFi后获取指定用户的哔哩哔哩实时播放数并在8位数码管上居中显示
 * 作者：改过自新的季  bilibili UID:95105053
 * 日期：2018/10/10
 * 声明：该项目已开源，禁止由于商用。若有侵权联系我。
 * 特别声明：该项目是在up主会飞的阿卡林(UID:751219)的项目https://git.io/fASb9上进行修改  只是改了api和加了一些注释
 * 语言参考: https://www.arduino.cc/reference/en/
 **********************************************************************/
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
//---------------修改此处""内的信息--------------------
const char* ssid     = "WAVLINK_8D35";   //WiFi名
const char* password = "845720701dlf";   //WiFi密码
String biliuid       = "95105053";       //bilibili UID
//----------------------------------------------------
DynamicJsonDocument jsonBuffer(200);//动态数据
WiFiClient client;
const int slaveSelect = 5;
const int scanLimit = 7;
int number = 0;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  pinMode(slaveSelect, OUTPUT);//卧槽5号引脚为输出
  digitalWrite(slaveSelect, LOW);//5号低电平
  sendCommand(12, 1);              //Shutdown,open来自系统的命令 关机 开机
  sendCommand(15, 0);              //DisplayTest,no 显示测试
  sendCommand(10, 15);             //Intensity,15(max) 强度
  sendCommand(11, scanLimit);      //ScanLimit,8-1=7 扫描限制
  sendCommand(9, 255);             //DecodeMode,Code B decode for digits 7-0 解码模式, 代码 B 解码数字
  digitalWrite(slaveSelect, HIGH);//5号又变高电平了
  WiFi.mode(WIFI_STA);//使你的板子其变为攻模式（sta为攻ap为受）
  WiFi.begin(ssid, password);//告诉他WiFi以及密码
  while (WiFi.status() != WL_CONNECTED)//大概意思是WiFi还未连接
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");//////这些看不懂→_→
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) //此处感谢av30129522视频作者的开源代码
  {
    HTTPClient http;
    http.begin("http://api.bilibili.com/x/relation/stat?vmid=" + biliuid);//这是api
    auto httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0)
    {
      String resBuff = http.getString();
      DeserializationError error = deserializeJson(jsonBuffer, resBuff);
      if (error)//卧槽这单词格外刺眼
      {
        Serial.println("json error");
        while (1);
      }
      JsonObject root = jsonBuffer.as<JsonObject>();//提取缓存
      long play = root["data"]["following"];//从缓存中找到你想要的
      Serial.println(play);
      displayNumber(play);//直接调用函数
      delay(1000);
    }
  }
  if (Serial.available())   //显示测试编号
  {
    Serial.println("available");
    char ch = Serial.read();
    if (ch == '\n')
    {
      displayNumber(number);//楼上调用我也要调用
      Serial.println(number);
      number = 0;
    }
    else
    {
      number = (number * 10) + ch - '0';
    }
  }
}

void sendCommand(int command, int value) 
{
  digitalWrite(slaveSelect, LOW);
  SPI.transfer(command);
  SPI.transfer(value);
  digitalWrite(slaveSelect, HIGH);
}
///////////////居中显示数字//////////////////
void displayNumber(int number) //我就是函数听说有人想调用我
{
  if (number < 0 || number > 99999999) return;
  int x = 1;
  int tmp = number;
  for (x = 1; tmp /= 10; x++);
  for (int i = 1; i < 9; i++)
  {
    if (i < (10 - x) / 2 || i >= (x / 2 + 5))
    {
      sendCommand(i, 0xf);
    }
    else
    {
      int character = number % 10;
      sendCommand(i, character);
      number /= 10;
    }
  }
}
