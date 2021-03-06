// ESP32 sketch sending data from sensor to MQTT
// sensor: joystick with two analog outputs (potentiometers), wifi connection
// multiple sensors button with led / animated version
// TNT button - when pushed sends MQTT message and starts animation

// Install ESP32 board by board manager, select your board (WEMOS LOLIN32)

#include <WiFi.h>
#include <PubSubClient.h>  //install pubsubclient O'Leary
#include "config.h"        //configuration not included in github

// library for button management using callbacks 
// https://github.com/r89m/Button https://github.com/r89m/PushButton
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h> // install bounce2 https://github.com/thomasfredericks/Bounce-Arduino-Wiring

// callback scheduling via SimpleTimer.h not widely used on ESP, why
// build-in Ticker.h (for ESP only)?
#include <Ticker.h>

// configuration of MQTT server ==============================================
//#define mqtt_server "your mqtt server IP address"
// no MQTT security
// #define mqtt_user "your_username"
// #define mqtt_password "your_password"

// =============== configuration of MQTT topics / OpenHAB items setup =======
#define joystick_topic_X "PI1/joystickX"
#define joystick_topic_Y "PI1/joystickY"
#define joystick_topic_B "PI1/joystickB"

#define button1_topic "PI1/buttonBlue" // blue
#define button2_topic "PI1/buttonGreen" // green
#define button3_topic "PI1/buttonRed" // red


// =============== configuration of joystick ==============================
int JoyStick_X = 32; // Analog Pin  X
int JoyStick_Y = 33; // // Analog Pin  Y
int JoyStick_button = 25; // IO Pin

// =============== configuration of buttons ===============================
const int button1Pin = 16;   //blue
const int led1Pin = 17;      
const int button2Pin = 18;   //green
const int led2Pin = 19;
// button3 is animated button / TNT      
const int button3Pin = 22;   //red  
const int led3Pin = 23;      

// ===================== config end ==========================================

// buttons
PushButton button1 = PushButton(button1Pin, ENABLE_INTERNAL_PULLUP);
PushButton button2 = PushButton(button2Pin, ENABLE_INTERNAL_PULLUP);
PushButton button3 = PushButton(button3Pin, ENABLE_INTERNAL_PULLUP);

Ticker animation; // Ticker.h object to call a callback 
int animationPhase = 0;


WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(JoyStick_X, INPUT);
  pinMode(JoyStick_Y, INPUT);
  pinMode(JoyStick_button, INPUT_PULLUP);
  Serial.println("Minecraft MQTT sensor started.");

  // initialize the LED pin as an output:
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  // button setup via callbacks - each callback can handle multiple buttons
  // When the button is released
  button1.onRelease(onButton1Released);
  button2.onRelease(onButton2Released);
  button3.onRelease(onButton3Released);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("MC-MQTT-sensor-joystick-ESP32")) { // name must be unique
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// lastMsg is time when last measurement was done
long lastMsg = millis();

boolean B1CurrState = LOW;
boolean B2CurrState = LOW;


void loop(){
  int x, y, button;
  
  //auto reconnect MQTT if needed
  if (!client.connected()) {
    reconnect();
  }
  //call MQTT library internal loop
  client.loop();
  //call button library internal loop
  button1.update();
  button2.update();
  button3.update();
  // Ticker.h doesn't need to call (it is attached to hardware interupt)
  
  long now = millis();
  if (now - lastMsg > 50) { // call the following code each x miliseconds
    lastMsg = now;

    // this is the interesting part / reading sensors, handling data and sending MQTT
    // ------------------------------------------------------------------------------

    //read the joystick
    x = analogRead(JoyStick_X);
    y = analogRead(JoyStick_Y);
    button = digitalRead(JoyStick_button); 

    // transform the readings into X/Y with beginnig in the center
    // note that there is no calibration / the beginning is not exact 0/0
    x = map(x, 0, 4095, -10, 10);
    y = map(y, 0, 4095, 10, -10);

    //publish only if the axis is not close to center
    //handle each axis independently

    if (x < -5 ) {
      Serial.println("L");
      client.publish(joystick_topic_X, "L", true);
    }
    if (x > 5 ) {
      Serial.println("R");
      client.publish(joystick_topic_X, "R", true);
    }
    if (y < -5 ) {
      Serial.println("D");
      client.publish(joystick_topic_Y, "D", true);
    }
    if (y > 5 ) {
      Serial.println("U");
      client.publish(joystick_topic_Y, "U", true);
    }

    if (button == 0) {   //reversed (pull up)
      Serial.println("B");
      client.publish(joystick_topic_B, "on", true);
      
    }

    Serial.println("-----------");
       
  }
}

void onButton1Released(Button& btn, uint16_t duration){
  // blue one
  Serial.println("Button1 blue pressed");
  if (B1CurrState) {
    B1CurrState = false;
    digitalWrite(led1Pin, LOW);
    client.publish(button1_topic, "off", true);
  } else {
    B1CurrState = true;
    digitalWrite(led1Pin, HIGH);
    client.publish(button1_topic, "on", true);
  }
}

void onButton2Released(Button& btn, uint16_t duration){
  // green one - the same as blue
  Serial.println("Button2 green pressed");
  if (B2CurrState) {
    B2CurrState = false;
    digitalWrite(led2Pin, LOW);
    client.publish(button2_topic, "off", true);
  } else {
    B2CurrState = true;
    digitalWrite(led2Pin, HIGH);
    client.publish(button2_topic, "on", true);
  }
}

void onButton3Released(Button& btn, uint16_t duration){

  Serial.println("Button3 red pressed");
  client.publish(button3_topic, "on", true);

  animationPhase=16;
  animation.attach_ms(250, animation1); //start calling animation procedure

}

void animation1() {
  Serial.print("Animation phase:");
  Serial.println(animationPhase);
  if (animationPhase > 3) {
    if (animationPhase % 2 == 0) { //even
      digitalWrite(led3Pin, HIGH);
      Serial.println("led3 ON");
    } else {
      digitalWrite(led3Pin, LOW);
      Serial.println("led3 OFF");
    }
  } else if (animationPhase == 1) {
    digitalWrite(led3Pin, LOW);
    animation.detach(); //stop calling this callback
    Serial.println("Animation end");
  }
  animationPhase--;
}
