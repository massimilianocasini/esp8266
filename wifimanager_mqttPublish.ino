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
int chipId = ESP.getChipId()
    
// MQTT server settings static/global DNS definitions.
const char* MQTT_SERVER = "raspberrypi"; // Mosquitto server.
const int MQTT_PORT = 1883;              // Your server port.
const char* MQTT_CLIENT_ID = "ESP8266_IN";  // Client name.
//const char* MQTT_USER = "xxxxxxxx";            // Your xxxxxxxx MQTT server user.
//const char* MQTT_PASS = "xxxxxxxxxxxx";        // Your xxxxxxxxxxxx MQTT server user password.
const char* MQTT_TOPIC = "events/"+chipId; // MQTT topics

WiFiClient wifiClient; // Declares a ESP8266WiFi client.
PubSubClient clientmqtt(wifiClient); // Declare a MQTT client.
Bounce bouncer = Bounce(); // Initialise the Pushbutton Bouncer object
WiFiManager wifiManager; //WiFiManager. Local initialization. Once its business is done, there is no need to keep it around

void setup() {
    Serial.begin(115200); // Serial connection from ESP-01 via 3.3v console cable
    Serial.setDebugOutput(false); 
    delay(3000); //Ritardo per inizializzare seriale;
    Serial.println("avvio");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    
    pinMode(RESETWIFI_PIN, INPUT); //Inizializzazione PIN RESET WIFI
    pinMode(buttonPin, INPUT); // Inizializzazione PIN INGRESSO
    bouncer.attach(buttonPin, INPUT_PULLUP); // Setup pushbutton Bouncer object
    bouncer.interval(5); // Sets the debounce interval in milliseconds. 
    wifiManager.autoConnect(); 
    clientmqtt.setServer(MQTT_SERVER, 1883);
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
    if (!clientmqtt.connected()) {  // verifica stato della connessione al server MQTT, se false chiama funzione di riconnessione
      reconnect();
    }
    clientmqtt.loop(); //Controlla messaggi e mantiene la connessione al server MQTT
    checkButton();
   // ConnectMqtt();
   //client.subscribe(mqtt_topic);
    //  PublishInformation(); // Publish information in MQTT.
    bouncer.update(); //Appena ricevi un update dal PIN bouncher
    if (bouncer.rose()) {
        clientmqtt.publish(MQTT_TOPIC, "Alarm"); //Pubblica nel Topic il messaggio
        Serial.println("Inviato a MQTT Alarm");
    }
}
void reconnect() {
  while (!clientmqtt.connected()) {
    Serial.println("Connessione a server MQTT...");
    if (clientmqtt.connect(MQTT_CLIENT_ID, mqttUser, mqttPassword )) {
       Serial.println("connesso");  
     } else {
      Serial.print("fallito con errore: ");
      Serial.print(clientmqtt.state());
      Serial.print("Attendo 2 secondi e riprovo ...")
      delay(2000);
     }
  }
}
