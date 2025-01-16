#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

Preferences pref;

const char* ssid = "xxxxxxxx";
const char* password = "xxxxxxxx";
const char* mqtt_server = "test.mosquitto.org";  // test.mosquitto.org

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char fan = 0;
long long int sysTime = 0;
int delay_time = 1000;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  ///////////////////////////////////////////////////////////////////Serial.println();
  ///////////////////////////////////////////////////////////////////Serial.print("Connecting to ");
  ///////////////////////////////////////////////////////////////////Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ///////////////////////////////////////////////////////////////////Serial.print(".");
  }

  randomSeed(micros());

  ///////////////////////////////////////////////////////////////////Serial.println("");
  ///////////////////////////////////////////////////////////////////Serial.println("WiFi connected");
  ///////////////////////////////////////////////////////////////////Serial.println("IP address: ");
  ///////////////////////////////////////////////////////////////////Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  //for (int i = 0; i < length; i++) { Serial.print((char)payload[i]); }

  String topics = String(topic);

  if (topics == "TELESERA/instant_watering_time")
  {
    //Serial.print("Anlık sulama süresi: ");
    Serial.print('3');
    delay_time = 0;
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    delay_time = (payload[0] - '0') * 100;
    delay_time += (payload[1] - '0') * 10;
    delay_time += (payload[2] - '0');

    delay(50);
    //Serial.println("ms");
  }
  else if (topics == "TELESERA/watering_time")
  {
    //Serial.print("Sulama süresi: ");
    Serial.print('1');
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    delay(50);
    //Serial.println("ms");
  }
  else if (topics == "TELESERA/waiting_time")
  {
    //Serial.print("Bekleme süresi: ");
    Serial.print('2');
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    delay(50);
    //Serial.println("dk");
  }
  else if (topics == "TELESERA/fan")
  {
    Serial.print('4');
    Serial.print('0');
    Serial.print((char)payload[0]);
    fan = (char)payload[0];
    if((char)payload[0] == '1')
    {
      digitalWrite(D1, LOW);
      digitalWrite(D2, LOW);
    }
    else
    {
      digitalWrite(D1, HIGH);
      digitalWrite(D2, HIGH);
    }
    delay(50);
  }
  else if (topics == "TELESERA/instant_watering")
  {
    Serial.print('4');
    Serial.print((char)payload[0]);
    Serial.print(fan);
    delay(50);
    if((char)payload[0] == '1')
    {
      client.publish("TELESERA/instant_watering", "0");
      digitalWrite(D3, LOW);
      digitalWrite(D4, LOW);
      delay(delay_time);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
      
    }
  }

  topics = "";

  //Serial.println();

  //12.11.2023.10.33.45

  /*snprintf (msg, MSG_BUFFER_SIZE, "TurnOnHour is :%d", ssetTurnOnHour);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("GX5632AC8/TurnOnHour", msg);    */
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    ///////////////////////////////////////////////////////////////////Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      ///////////////////////////////////////////////////////////////////Serial.println("connected");
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
      ///////////////////////////////////////////////////////////////////Serial.print("failed, rc=");
      ///////////////////////////////////////////////////////////////////Serial.print(client.state());
      ///////////////////////////////////////////////////////////////////Serial.println(" try again in 5 seconds");
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

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
}

int counter = 0;
char gelenVeri[3];
int sayac = 0;
char mqtt_message[2];

void loop() {

  if (!client.connected()) {
    if (counter < 3)
    {
      counter++;
      reconnect();
    }
  }
  client.loop();

  /*if((millis() - sysTime) > 60000)
  {
    sysTime = millis();
    if((sysTime/600000) > 0)
    {
      mqtt_message[0] = '0' + (sysTime/600000);
    }
    else
    {
      mqtt_message[0] = '  ';
    }
    mqtt_message[1] = '0' + ((sysTime/60000)%10);
    client.publish("TELESERA/systime", mqtt_message);
  }*/

  if (Serial.available() > 0) 
    { /* bilgisayardan veri gelmesini bekliyoruz */
      gelenVeri[sayac] = Serial.read(); /* bilgisayardan gelen karakteri oku */
      sayac++;
      if(sayac == 3)
      {
        sayac = 0;
        /*for(int j = 0; j < 3; j++)
        {
          Serial.print(gelenVeri[j]);
        }*/
        if(gelenVeri[0] == 123)
        {
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          client.publish("TELESERA/terr_hum", mqtt_message);
        }
        else if(gelenVeri[0] == 124)
        {
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          client.publish("TELESERA/light", mqtt_message);
        }
        else if(gelenVeri[0] == 125)
        {
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          client.publish("TELESERA/hum", mqtt_message);
        }
        else if(gelenVeri[0] == 126)
        {
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          client.publish("TELESERA/temp", mqtt_message);
        }
      }
   }

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
  ///////////////////////////////////////////////////////////////////Serial.println("AAAAA");

  client.publish(result, val1);
  ///////////////////////////////////////////////////////////////////Serial.println(result);
  ///////////////////////////////////////////////////////////////////Serial.println(val);
}
