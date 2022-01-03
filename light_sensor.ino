#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <WiFi.h>
#define LIGHT_SENSOR_PIN 36



unsigned long delayTime;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


const char* ssid = "Redmi";
const char* password = "ffffdddd7777";
const int wifiConnectRetries = 3;
const int wifiConnectDelayBetweenRetries = 2000;

const char* mqtt_server = "mqtt.flespi.io";
const int mqtt_port = 1883;
//const int mqtt_port = 8883;
//const char* mqtt_user = "FvMpotv7lSBlGogjEhjjt5BQAT5t9L0E4CSiTbf2RHdnTjuopDvIxXpVEkcBT24O";
const char* mqtt_user = "5ndhw6uFeLx2P5kBMhapxux4jtHpc2R7fTcBPv4R4qKJlS9VZ99mtueALMNNCtJH";
const char* mqtt_pass = "";

const int screenDelay = 4000; // Screen LCD update interval
const int mqttPublishDelay = 15000; // Send to MQTT with this interval
const int mqttConnectRetries = 3;
const int mqttConnectDelayBetweenRetries = 1000;
 
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

float light = 0.0;
int countDelays = 0;

 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  init_lcd();
 
  connectWifi(wifiConnectRetries, wifiConnectDelayBetweenRetries);
  initMQTT();
}



void init_lcd(){
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  delayTime = 1000;
}
 
void loop() {
  readDataValues();
  printValues(screenDelay); // Print values from variables to LCD 
  
  client.loop(); // loop the pubsub client for callbacks (subscribes)
  countDelays++;
  if(countDelays>=(mqttPublishDelay/(screenDelay*2))){
    countDelays=0;
    connectPublishMQTT(); // Connect to MQTT and publish data
  }
}

void connectPublishMQTT(){
  if(WiFi.status() != WL_CONNECTED){
    connectWifi(wifiConnectRetries, wifiConnectDelayBetweenRetries);
  } else {
    if(!client.connected()){
      connectMQTT(mqttConnectRetries, mqttConnectDelayBetweenRetries);
    } else {
      publishData();
    }
  }
}

void publishData(){
  String lght = String(light).c_str();
  String id = "2";
  
  String s2 = "{\"light_level\": " + lght + ", \"id\": " + id + "}";
    client.publish("easv/light", String(s2).c_str());    
}

void initMQTT(){
  Serial.println("Initializing MQTT");
   
  randomSeed(micros());
  client.setServer(mqtt_server, mqtt_port);
  
  connectMQTT(mqttConnectRetries, mqttConnectDelayBetweenRetries);
}
 
void connectMQTT(int retries, int delayBetweenRetries){
  while(!client.connected() && retries>=0){
    String clientId = "Client-";
    clientId += String(random(0xffff), HEX);
    if(client.connect(clientId.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("Successfully connected MQTT");
    } else {
      Serial.println("Error!");
      Serial.println(client.state());
      delay(delayBetweenRetries);
      retries--;
    }
  }
}
 
void connectWifi(int retries, int delayBetweenRetries){
  WiFi.begin(ssid, password);
   
  while(status != WL_CONNECTED && retries>=0){
    status = WiFi.status();
    delay(delayBetweenRetries);
    Serial.print(".");
    retries--;
  }
}

void readDataValues(){
    light = analogRead(LIGHT_SENSOR_PIN);
}


void printValues(int screenDelay) {
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  lcd.setCursor(0,0);
  lcd.print("Light ");
  Serial.print(analogValue);
  lcd.print(analogRead(LIGHT_SENSOR_PIN));
  
   if (analogValue < 40) {
    Serial.println(" => Dark");
  } else if (analogValue < 800) {
    Serial.println(" => Dim");
  } else if (analogValue < 2000) {
    Serial.println(" => Light");
  } else if (analogValue < 3200) {
    Serial.println(" => Bright");
  } else {
    Serial.println(" => Very bright");
  }
 
  //delay(screenDelay);
  //lcd.clear();
  
  lcd.setCursor(0,1);
  lcd.print("Net ");
  lcd.print(netStateToString(client.state()));
  
  delay(screenDelay);  
  lcd.clear();


}

/*
 * https://pubsubclient.knolleary.net/api#state
Returns
-4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
-3 : MQTT_CONNECTION_LOST - the network connection was broken
-2 : MQTT_CONNECT_FAILED - the network connection failed
-1 : MQTT_DISCONNECTED - the client is disconnected cleanly
0 : MQTT_CONNECTED - the client is connected
1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
*/

String netStateToString(int state){ // LCD short friendly string
    switch(state){
      case MQTT_CONNECTION_TIMEOUT:
        return "Timeout con";
        break;
      case MQTT_CONNECTION_LOST:
        return "Lost con";
        break;
      case MQTT_CONNECT_FAILED:
        return "Con fail";
        break;
      case MQTT_DISCONNECTED:
        return "Disconnect";
        break;
      case MQTT_CONNECTED:
        return "Connected";
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        return "Bad protocol";
        break;  
      case MQTT_CONNECT_BAD_CLIENT_ID:
        return "Bad Client Id";
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        return "Unavailable";
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        return "Bad Creds";
        break;  
      case MQTT_CONNECT_UNAUTHORIZED:
        return "Unauthorized";
        break;  
      default:
        return "Unknown ERROR!";
    }
}
