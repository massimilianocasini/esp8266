#include <Bounce2.h> // Used for "debouncing" the pushbutton
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include  <ESP8266mDNS.h>     //mDSN library (needed to detect local mqtt transparently)
#define RESETWIFI_PIN 0 //Definizione GPIO0
#define RELAY 2 //Definizione GPIO2
    
// MQTT server settings static/global DNS definitions.
const char* MQTT_SERVER = "openhabianpi.local"; // Mosquitto server.
const int MQTT_PORT = 1883;              // Your server port.
const char* MQTT_CLIENT_ID = "ESP8266_OUT";  // Client name.
const char* MQTT_USER = "xxxxxxxx";            // Your xxxxxxxx MQTT server user.
const char* MQTT_PASS = "xxxxxxxxxxxx";        // Your xxxxxxxxxxxx MQTT server user password.
const char* MQTT_TOPIC_E = "eventi"; // MQTT topics eventi
const char* MQTT_TOPIC_A = "alive_out"; // MQTT topics sopravvivenza
long lastMsg = 0;
char msg[50];
int value = 0;

WiFiClient wifiClient; // Declares a ESP8266WiFi client.
PubSubClient _mqqtClient(wifiClient); // Declare a MQTT client.
WiFiManager wifiManager; //WiFiManager. Local initialization. Once its business is done, there is no need to keep it around

void setup(void) {
    Serial.begin(115200); // Serial connection from ESP-01 via 3.3v console cable
    Serial.setDebugOutput(false); 
    delay(3000); //Ritardo per inizializzare seriale;
    Serial.println("avvio");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    
    pinMode(RESETWIFI_PIN, INPUT); //Inizializzazione PIN RESET WIFI
    pinMode(RELAY, OUTPUT);
    wifiManager.autoConnect(); 
    _mqqtClient.setServer(MQTT_SERVER, 1883);
    //facciamo il subscribe del topic(canale) che vogliamo
    _mqqtClient.subscribe(MQTT_TOPIC_E);
    Serial.print("Sottoscritto topic [");
    Serial.print(MQTT_TOPIC_E);
    Serial.println("]");
    _mqqtClient.setCallback(callback);
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


void reconnect() {
  while (!_mqqtClient.connected()) {
    Serial.print("Connessione a server MQTT...");
    if (_mqqtClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS )) {
       Serial.println("connesso");
       _mqqtClient.subscribe(MQTT_TOPIC_E);
       Serial.print("Sottoscritto topic [");
       Serial.print(MQTT_TOPIC_E);
       Serial.println("]");  
     } else {
      Serial.print("fallito con errore: ");
      Serial.println(_mqqtClient.state());
      Serial.print("Attendo 2 secondi e riprovo ...");
      delay(2000);
     }
  }
}

//funzione di callback invocata quando riceviamo un messaggio MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrivato [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(RELAY, LOW);
    Serial.println("RELE LOW");// GPIO a massa
  } else {
    digitalWrite(RELAY, HIGH);// GPIO a positivo
    Serial.println("RELE HIGH");  
  }
}

void loop(void) {
        if (!ConnectWiFi() || !ConnectMqtt())
        {
            return;
        }
    _mqttClient.loop();
    // Publish information in MQTT.
    PublishInformation();  
    checkButton();
    long now = millis();
    if (now - lastMsg > 10000) {
      lastMsg = now;
      ++value;
      snprintf (msg, 75, "hello world #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      _mqqtClient.publish(MQTT_TOPIC_A, msg);
    }
}

bool ConnectWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Reconnecting [");
        Serial.print(WiFi.SSID());
        Serial.println("]...");

        WiFi.begin();
        //WiFi.begin(SSID, SSID_PASSWORD);

        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            return false;
        }

        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }

    return true;
}


bool ConnectMqtt()
    {
    if (!_mqttClient.connected())
    {
    // reset ther server IP address as it could be changed by mDNS procedure (see below)
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  
    Serial.print("Attempting MQTT connection to ");
    Serial.println(MQTT_SERVER);
    Serial.print("Port: ");
    Serial.println(MQTT_PORT);
    
    //if (_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
    if (_mqttClient.connect(MQTT_CLIENT_ID))
    {
    Serial.println("Connected.");
    _mqttClient.subscribe(TOPIC_COMMAND);
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(_mqttClient.state());
        Serial.println(" try again after 5 seconds");
        Serial.println("Search over mDNS");

        // search for MQTT server on the local segment
        // assume that MNQTT server published with the following command
        // avahi-publish -s "MTT server for ProdinoWiFi" _mqtt_prodino._tcp 1833 "string2"

        int n = MDNS.queryService(MQTT_SERVER_MNDS, "tcp"); // Send out query for esp tcp services
        Serial.println("mDNS query done");
        if (n == 0) {
            Serial.println("no services found");
        }
        else {
            Serial.print(n);
            Serial.println(" service(s) found");
            for (int i = 0; i < n; ++i) {
                // Print details for each service found
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(MDNS.hostname(i));
                Serial.print(" (");
                Serial.print(MDNS.IP(i));
                Serial.print(":");
                Serial.print(MDNS.port(i));
                Serial.println(")");
                //mqtt_server = MDNS.IP(i);
                //mqtt_server_found = true;
                //mdns_f = 1;
                    
                _mqttClient.setServer(MDNS.IP(i), MDNS.port(i));

                if (_mqttClient.connect(MQTT_CLIENT_ID)) {
                    Serial.println("Connected.");
                    _mqttClient.subscribe(TOPIC_COMMAND);
                } 
                else {
                    Serial.print("failed, rc=");
                    Serial.print(_mqttClient.state());
                    Serial.println(" try again after 5 seconds");
                }
            }
        }
        
        // Wait 5 seconds before retrying
        delay(5000);
    }
    return _mqttClient.connected();
}
