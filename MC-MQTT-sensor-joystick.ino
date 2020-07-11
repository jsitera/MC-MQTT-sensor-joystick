// ESP32 sketch sending data from sensor to MQTT
// sensor: joystick with two analog outputs (potentiometers), wifi connection

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"        //configuration not included in github


// configuration of MQTT server ==============================================
//#define mqtt_server "your mqtt server IP address"
// no MQTT security
// #define mqtt_user "your_username"
// #define mqtt_password "your_password"

// =============== configuration of MQTT topics / OpenHAB items setup =======
#define joystick_topic_X "PI1/joystickX"
#define joystick_topic_Y "PI1/joystickY"
#define joystick_topic_B "PI1/joystickB"


// =============== configuration of joystick ==============================
int JoyStick_X = 32; // Analog Pin  X
int JoyStick_Y = 33; // // Analog Pin  Y
int JoyStick_button = 25; // IO Pin

// ===================== config end ==========================================



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


void loop(){
  int x, y, button;
  
  //auto reconnect MQTT if needed
  if (!client.connected()) {
    reconnect();
  }
  //call MQTT library internal loop
  client.loop();

  long now = millis();
  if (now - lastMsg > 100) { // call the following code each x miliseconds
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
