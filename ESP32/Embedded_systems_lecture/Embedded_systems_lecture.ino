#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

Preferences pref;

/* Baglanilacak wifi ismi ve sifresini belirle */
const char* ssid = "xxxxxxxx";
const char* password = "xxxxxxxx";
const char* mqtt_server = "test.mosquitto.org";

/* Gerekli degiskenleri ve nesneleri olustur */
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char fan = 0;
long long int sysTime = 0;
int delay_time = 1000;
int waiting_time = 5;
long long int waited_time = 0;
bool water_flag = 0;
int watering_time = 1000;

/* Wifi baglantisini kur */
void setup_wifi() {

  delay(10);
  WiFi.mode(WIFI_STA);
  /* Wifi ismi ve sifresiyle baglanma komutu ver */
  WiFi.begin(ssid, password);

  /* Baglanti kurulana kadar bekle */
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }
  randomSeed(micros());
}

/* MQTT bildirimi geldiginde callback fonksiyonuna gir */
void callback(char* topic, byte* payload, unsigned int length) {

  /* Topic bilgisini stringe donustur ve kaydet */
  String topics = String(topic);

  /* Anlik sulama zamani guncellemesi gelirse yapilacaklar */
  if (topics == "TELESERA/instant_watering_time")
  {
    /* '3' ile STM tarafina anlik sulama zamani gonderildigini belirt */
    Serial.print('3');
    delay_time = 0;
    /* Verileri sirasiyla gonder */
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    /* Anlik sulama suresini hesaplayip degiskene kaydet */
    delay_time = (payload[0] - '0') * 100;
    delay_time += (payload[1] - '0') * 10;
    delay_time += (payload[2] - '0');

    delay(50);
  }
  /* Sulama zamani guncellemesi gelirse yapilacaklar */
  else if (topics == "TELESERA/watering_time")
  {
    /* '1' ile STM tarafina sulama zamani gonderildigini belirt */
    Serial.print('1');
    /* Verileri sirasiyla gonder */
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    /* Sulama suresini hesaplayip degiskene kaydet */
    watering_time = (payload[0] - '0') * 10;
    watering_time += (payload[1] - '0');
    watering_time *= 1000;
    delay(50);
  }
  /* Bekleme zamani guncellemesi gelirse yapilacaklar */
  else if (topics == "TELESERA/waiting_time")
  {
    /* '2' ile STM tarafina bekleme zamani gonderildigini belirt */
    Serial.print('2');
    /* Verileri sirasiyla gonder */
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    /* Bekleme suresini hesaplayip degiskene kaydet */
    waiting_time = (payload[0] - '0') * 10;
    waiting_time += (payload[1] - '0');
    waiting_time *= 60;
    waiting_time *= 1000;
    /* Tekrarli sulama bayragini etkinlestir */
    water_flag = 1;
    /* Sistem zamanini degiskene kaydet */
    waited_time = millis();
    delay(50);
  }
  /* Fan bilgisi guncellemesi gelirse yapilacaklar */
  else if (topics == "TELESERA/fan")
  {
    /* Fan acma istegini stm tarafina gonder ve gecerli fan durumunu kaydet */
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
  /* Anlik sulama guncellemesi gelirse yapilacaklar */
  else if (topics == "TELESERA/instant_watering")
  {
    /* Anlik sulama istegini stm tarafina gonder ve gecerli fan durumunu kaydet */
    Serial.print('4');
    Serial.print((char)payload[0]);
    Serial.print(fan);
    delay(50);
    if((char)payload[0] == '1')
    {
      /* Anlik sulama verisini MQTT uzerinde sifirla */
      client.publish("TELESERA/instant_watering", "0");
      digitalWrite(D3, LOW);
      digitalWrite(D4, LOW);
      delay(delay_time);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
      
    }
  }
  /* Topic bilgisini sifirla */
  topics = "";
}

void reconnect() {
  /* Bglanti koparsa tekrar baglanana kadar bekle */
  while (!client.connected()) {
    /* Rastgele client ID degeri olustur */
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    /* Baglanti basarili olursa asagida verilenleri yap */
    if (client.connect(clientId.c_str())) {
      /* TELESERA topic degerlerine rastgele degerleri gonder */
      client.publish("TELESERA/temp", "20");
      client.publish("TELESERA/hum", "78");
      client.publish("TELESERA/light", "56");
      client.publish("TELESERA/terr_hum", "20");
      client.publish("TELESERA/fan", "Açık");
      client.publish("TELESERA/systime", "10");
      /* TELESERA verilerine abone ol */
      client.subscribe("TELESERA/instant_watering_time");
      client.subscribe("TELESERA/watering_time");
      client.subscribe("TELESERA/waiting_time");
      client.subscribe("TELESERA/fan");
      client.subscribe("TELESERA/instant_watering");
    }
    else
    {
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  /* 115200 baud rate ile uart iletisimini baslat */
  Serial.begin(115200);
  /* Wifi kurulum fonksiyonunu cagir */
  setup_wifi();
  /* Belirtilen port numarasi ile server i ayarla */
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

/* Verileri uart ile almak ve gondermek icin gerekli degiskenleri olustur */
int counter = 0;
char gelenVeri[3];
int sayac = 0;
char mqtt_message[2];

void loop() {
  /* Client e baglanana kadar bekle */
  if (!client.connected()) {
    if (counter < 3)
    {
      counter++;
      reconnect();
    }
  }
  client.loop();

  if(water_flag)
  {
    if(millis() >= (waited_time + waiting_time))
    {
      digitalWrite(D3, LOW);
      digitalWrite(D4, LOW);
      delay(watering_time);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
      waited_time = millis();
    }
  }  

  /* STM tarafindan veri geldiginde asadika verilenleri yap */
  if (Serial.available() > 0) 
    { 
      /* STM tarafindan gelen verileri oku */
      gelenVeri[sayac] = Serial.read(); 
      sayac++;
      if(sayac == 3)
      {
        sayac = 0;
        /* Ne verisi geldigini anlamak icin ilk byte i kontrol et */
        if(gelenVeri[0] == 123)
        {
          /* Gelen verileri ascii 0 degeriyle toplayarak karakterlere donustur ve diziye kaydet */
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          /* Kaydedilen veriyi server a gonder */
          client.publish("TELESERA/terr_hum", mqtt_message);
        }
        /* Ne verisi geldigini anlamak icin ilk byte i kontrol et */
        else if(gelenVeri[0] == 124)
        {
          /* Gelen verileri ascii 0 degeriyle toplayarak karakterlere donustur ve diziye kaydet */
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          /* Kaydedilen veriyi server a gonder */
          client.publish("TELESERA/light", mqtt_message);
        }
        /* Ne verisi geldigini anlamak icin ilk byte i kontrol et */
        else if(gelenVeri[0] == 125)
        {
          /* Gelen verileri ascii 0 degeriyle toplayarak karakterlere donustur ve diziye kaydet */
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          /* Kaydedilen veriyi server a gonder */
          client.publish("TELESERA/hum", mqtt_message);
        }
        /* Ne verisi geldigini anlamak icin ilk byte i kontrol et */
        else if(gelenVeri[0] == 126)
        {
          /* Gelen verileri ascii 0 degeriyle toplayarak karakterlere donustur ve diziye kaydet */
          mqtt_message[0] = '0' + gelenVeri[1];
          mqtt_message[1] = '0' + gelenVeri[2];
          /* Kaydedilen veriyi server a gonder */
          client.publish("TELESERA/temp", mqtt_message);
        }
      }
   }
}

void publishInt(int val, char label1[11])
{
  char label[11] = "TELESERA/";
  char val1[3];
  char result[27];
  strcat(result, label);
  strcat(result, label1);
  snprintf(val1, sizeof(val1), "%d", val);

  client.publish(result, val1);
}
