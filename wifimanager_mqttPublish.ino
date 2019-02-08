#include <Bounce2.h> // Used for "debouncing" the pushbutton
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include  <ESP8266mDNS.h>     //mDSN library (needed to detect local mqtt transparently)
#define RESETWIFI_PIN 0
// const int ledPin = 0; // GPIO0 LED
int buttonPin = 2; // GPIO2 INGRESSO

// MQTT server settings static/global DNS definitions.
const char* MQTT_SERVER = "raspberrypi"; // Test  mosquitto server (remove xxxx.).
const int MQTT_PORT = 1883;                   // Your server port.
const char* MQTT_CLIENT_ID = "ESP8266_IN";  // Client name.
//const char* MQTT_USER = "xxxxxxxx";            // Your xxxxxxxx MQTT server user.
//const char* MQTT_PASS = "xxxxxxxxxxxx";        // Your xxxxxxxxxxxx MQTT server user password.
const char* MQTT_TOPIC = "events"; // MQTT topics

WiFiClient wifiClient; // Declares a ESP8266WiFi client.
PubSubClient client(wifiClient); // Declare a MQTT client.
Bounce bouncer = Bounce(); // Initialise the Pushbutton Bouncer object
WiFiManager wifiManager; //WiFiManager. Local initialization. Once its business is done, there is no need to keep it around

void setup() {
    // You can open the Arduino IDE Serial Monitor window to see what the code is doing
    Serial.begin(115200); // Serial connection from ESP-01 via 3.3v console cable
    Serial.setDebugOutput(false); 
    delay(3000); //Ritardo per inizializzare seriale;
    Serial.println("avvio");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    
    pinMode(RESETWIFI_PIN, INPUT);
  //  pinMode(ledPin, OUTPUT); // Inizializzazione PIN LED
    pinMode(buttonPin, INPUT); // Inizializzazione PIN INGRESSO
    //digitalWrite(ledPin, HIGH); // Switch the on-board LED off to start with
    bouncer.attach(buttonPin, INPUT_PULLUP); // Setup pushbutton Bouncer object
    bouncer.interval(5); // Sets the debounce interval in milliseconds.
    
    wifiManager.autoConnect(); 
    client.setServer(MQTT_SERVER, 1883);
}

void checkButton()  {
    if (digitalRead(RESETWIFI_PIN) == LOW) { // check for button press
        delay(50); // poor mans debounce/press-hold, code not ideal for production
        if (digitalRead(RESETWIFI_PIN) == LOW) {
            Serial.println("PulsanteReset...");
            // still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(3000); // reset delay hold
            if (digitalRead(RESETWIFI_PIN) == LOW) {
                Serial.println("Reset acquisito");
                Serial.println("Esp restarting");
                wifiManager.resetSettings();
                ESP.restart();
            }
        }
    }
}

void loop() {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    checkButton();
   // ConnectMqtt();
   //client.subscribe(mqtt_topic);
    //  PublishInformation(); // Publish information in MQTT.
    bouncer.update(); //Appena ricevi un update dal PIN bouncher
    if (bouncer.rose()) {
        client.publish(MQTT_TOPIC, "Alarm");
        Serial.println("Inviato a MQTT Alarm");
    }
}

//bool ConnectWiFi()  {
//    if (WiFi.status() != WL_CONNECTED) {
//                //    Serial.print("Stato WIFI ")
//                //   Serial.println(WiFi.status()) //Stampa stato WIFI
//        Serial.print("Reconnecting [");
//        Serial.print(WiFi.SSID());
//        Serial.println("]...");
//        WiFi.begin();
//        if (WiFi.waitForConnectResult() != WL_CONNECTED)
//        Serial.println("Non connesso ...");   {
//            return false;
//        }
//        Serial.print("IP address: ");
//        Serial.println(WiFi.localIP());
//    }
//return true;
//}
//
//bool ConnectMqtt() {
//    
//            client.subscribe(mqtt_topic);
//       
//}
void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attendo connessione MQTT ...");
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (client.connect(clientId.c_str())) {
        Serial.println("Connesso");
        // Once connected, publish an announcement...
        client.publish(MQTT_TOPIC, "Hello world");
        Serial.println("Inviato a MQTT Hello world");
        // ... and resubscribe
        //client.subscribe("inTopic"); 
      } 
      else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      }
    }
}
