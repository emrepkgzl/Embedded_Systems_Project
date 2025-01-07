#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

Preferences pref;

const char* ssid = "DUNDER MUFFLIN INC.";
const char* password = "FYNkjbrzyK";
const char* mqtt_server = "test.mosquitto.org";  // test.mosquitto.org

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //for (int i = 0; i < length; i++) { Serial.print((char)payload[i]); }

   String topics = String(topic);
   
    if(topics == "TELESERA/instant_watering_time")
    {
      Serial.print("Anlık sulama süresi: ");
      for (int i = 0; i < length; i++) 
      { 
        Serial.print((char)payload[i]); 
        }
      Serial.println("ms");
    }
    else if(topics == "TELESERA/watering_time")
    {
      Serial.print("Sulama süresi: ");
      for (int i = 0; i < length; i++) 
      { 
        Serial.print((char)payload[i]); 
        }
      Serial.println("ms");
    }
    else if(topics == "TELESERA/waiting_time")
    {
      Serial.print("Bekleme süresi: ");
      for (int i = 0; i < length; i++) 
      { 
        Serial.print((char)payload[i]); 
        }
      Serial.println("dk");
    }
    else if(topics == "TELESERA/fan")
    {
      Serial.print("Fan durumu: ");
      for (int i = 0; i < length; i++) 
      { 
        Serial.print((char)payload[i]); 
        }
      Serial.println("");
    }
    else if(topics == "TELESERA/instant_watering")
    {
      Serial.print("Anlık sulama: ");
      char anlik_sulama[length];
      for (int i = 0; i < length; i++) 
      { 
        Serial.print((char)payload[i]); 
        anlik_sulama[i] = (char)payload[i];

        if(anlik_sulama[i] == 'c')
        {
          client.publish("TELESERA/instant_watering", "Kapali");
        }
      }
        
        
      Serial.println("");
    }

    topics = "";
    
  Serial.println();

  //12.11.2023.10.33.45

    /*snprintf (msg, MSG_BUFFER_SIZE, "TurnOnHour is :%d", ssetTurnOnHour);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("GX5632AC8/TurnOnHour", msg);    */
  

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("TELESERA/temp", "20");
      // ... and resubscribe
      client.publish("TELESERA/hum", "78");
      // ... and resubscribe
      client.publish("TELESERA/light", "56");
      // ... and resubscribe
      client.publish("TELESERA/terr_hum", "20");
      // ... and resubscribe
      client.publish("TELESERA/fan", "Açık");
      // ... and resubscribe
      client.publish("TELESERA/systime", "10");
      // ... and resubscribe
      client.subscribe("TELESERA/instant_watering_time");
      client.subscribe("TELESERA/watering_time");
      client.subscribe("TELESERA/waiting_time");
      client.subscribe("TELESERA/fan");
      client.subscribe("TELESERA/instant_watering");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

int counter = 0;

void loop() {

  if (!client.connected()) {
    if(counter < 3)
    {
      counter++;
      reconnect();
    }
    
  }
  client.loop();

  /*snprintf (msg, MSG_BUFFER_SIZE, "TurnOnMinute is :%d", ssetTurnOnMinute);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("TELESERA/TurnOnMinute", msg);  */    
   
}

void publishInt(int val, char label1[11])
{

  char label[11] = "TELESERA/";
  char val1[3];
  char result[27];
  strcat(result, label);
  strcat(result, label1);
  snprintf(val1, sizeof(val1), "%d", val);
  Serial.println("AAAAA");

  client.publish(result, val1);
  Serial.println(result);
  Serial.println(val);
}
