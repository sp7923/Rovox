#include <atomic>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SparkFun_TB6612.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include "NewPing.h"

#include <Servo_ESP32.h>

#include "../config/config.h"

#include <PubSubClient.h>

extern const char* PASSWORD;
extern const char* SSID;


const char* ssid = SSID;
const char* password = PASSWORD;


// MQTT Broker

const char* mqtt_broker = MQTT_BROKER;
const char* topic = TOPIC;
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;
const int mqtt_port = MQTT_PORT;

WiFiClient espClient;
PubSubClient client(espClient);


// Standard UUIds for Device Information
#define DEVINFO_UUID (uint16_t)0x180a
#define DEVINFO_MANUFACTURER_UUID (uint16_t)0x2a29
#define DEVINFO_NAME_UUID (uint16_t)0x2a24
#define DEVINFO_SERIAL_UUID (uint16_t)0x2a25

// constant loop delay

#define LOOPDELAY 1000


// Left Motor
const int Motor_1_Pin_1 = 15;
const int Motor_1_Pin_2 = 27;
const int Enable_1_Pin = 14;

// Right Motor
const int Motor_2_Pin_1 = 25;
const int Motor_2_Pin_2 = 26;
const int Enable_2_Pin = 4;

// Motor definitions
Motor* motor1;
Motor* motor2;




// Ultra sensor pins- single pin configuration
const int trigger = 32;
const int echo = 32;

boolean flag;

#define TRIGGER_DIRECTION_DISTANCE 30

#define MAX_DISTANCE 200 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 40 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

Servo_ESP32 servo; // Creates a servo object 

int pos; // stores servo position
const int Servo_Pin = 17;

//boolean volatile started=false;

std::atomic<bool> started(false);

#define ONBOARD_LED  13 // only for Adafruit Huzzah32


// Setting PWM properties
#define FREQUENCY 30000
#define pwmChannel1  0
#define pwmChannel2  1
#define pwmServoChannel 8
#define resolution  8

#define dutyCycle_7 100
#define dutyCycle_11 90

int speed;

// Ultrasonic Setup
NewPing sonar(trigger, echo,MAX_DISTANCE);

// For Aysnc
AsyncWebServer server(80);


void keepLow();

void deviateRight();
void deviateLeft();
void forward();
void reverse(boolean shouldDelay);



void handleEchoSensor(int pos);

void wifi();

void servoMotor();

void callback(char *topic, byte *payload, unsigned int length);

boolean checkObstacle(float distance);

void setup()
{
  Serial.begin(9600);
  wifi();
  pinMode(ONBOARD_LED,OUTPUT);
 
  // Pin mode of Motor pins internally handled by the library
  motor1=new Motor(Motor_1_Pin_1, Motor_1_Pin_2, Enable_1_Pin, 1,pwmChannel1,FREQUENCY,resolution);
  motor2=new Motor(Motor_2_Pin_1, Motor_2_Pin_2, Enable_2_Pin, 1,pwmChannel2,FREQUENCY,resolution);

  digitalWrite(ONBOARD_LED,HIGH);
  keepLow();


  // Servo Motor Setup
  servo.attach(Servo_Pin,pwmServoChannel); // for 0 to 180 sweep

  // Connecting to an MQTT broker
  client.setServer(mqtt_broker,mqtt_port);
  client.setCallback(callback);

  while(!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n",client_id.c_str());
    if(client.connect(client_id.c_str(),mqtt_username,mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else{
            Serial.print("Failed with state ");
            Serial.print(client.state());
            delay(2000);
    }
  }

  // Publish and subsribe
  // client.publish(topic,"hello emqx");
  client.subscribe(topic);
  
}

void loop()
{
  client.loop();

  if(started.load()){
   servoMotor();
  }
  
  delay(LOOPDELAY);

}

void wifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

}

 void keepLow(){
    switchOff(*motor1,*motor2);
} 

void forward()
{
  motor1->drive(speed);
  motor2->drive(speed);
  Serial.println("Moving forward");
}

void reverse(boolean shouldDelay)
{
  motor1->drive(-speed);
  motor2->drive(-speed);
  //Serial.println("Moving backward");
  if(shouldDelay){
    delay(300);
  }
  
}

void deviateRight()
{
  reverse(true);
  right(*motor1,*motor2,speed);
  //Serial.println("Turning right");
  delay(750);
  brake(*motor1,*motor2);
  delay(200);
  forward();
}

void deviateLeft()
{
  reverse(true);
  left(*motor1,*motor2,speed);
  //Serial.println("Turning left");
  delay(750);
  brake(*motor1,*motor2);
  delay(200);
  forward();
}

void handleEchoSensor(int pos){
  // Send ping, get distance in cm
  float distance = sonar.ping_cm();
  delay(PING_INTERVAL);

 /*  Serial.println("Distances: ");
  Serial.println(distanceL);
  Serial.println(distanceC);
  Serial.println(distanceR); */
  //delay(1000);

  flag=checkObstacle(distance);
  
  if (pos<90){
    if(flag){
      deviateRight();
    }
    else{
      forward();
    }
  }

  else if(pos==90){
    if(flag){
      keepLow();
    }
    else{
      forward();
    }
  }

  else if(pos>90){
    if(flag){
      deviateLeft();
    }
    else{
      forward();
    }
  }

}

boolean checkObstacle(float distance){
  if(distance==0 || distance >TRIGGER_DIRECTION_DISTANCE){
    return false;
  } else{
    return true;
  }

}

void servoMotor() {
  for(pos = 0; pos <= 180; pos +=10 ) {
    if(!started.load()){
      keepLow();
      break;
    }
    servo.write(pos);
    delay(100);
    handleEchoSensor(pos);
  }
  for(pos = 180; pos >= 0; pos -= 10){
    if(!started.load()){
      keepLow();
      break;
    }
    servo.write(pos);
    delay(100);
    handleEchoSensor(pos);
  }
}

// callback function for mqtt

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String message;
  for (int i=0;i<length;i++) {
    message += (char) payload[i]; // converting byte to string
  }
  Serial.print(message);
  
  // using recieved message for communication
  if(message.equals("start"))
      {
        speed=dutyCycle_11;
        started=true;
        forward();
      }
  if(message.equals("stop"))
      {
        // This should be synchronized to avoid race condition with echo
        started=false;
        keepLow();
        servo.write(90);
      }
  Serial.println();
  Serial.println("-----------------------");
}
