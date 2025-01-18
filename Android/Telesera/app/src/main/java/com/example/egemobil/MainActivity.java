package com.example.egemobil;

import androidx.appcompat.app.AppCompatActivity;

import android.app.PendingIntent;
import android.os.Bundle;
import android.os.Handler;
import android.preference.EditTextPreference;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.content.Intent;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity {
    /* MQTT protokolu ile server a baglanmak icin gerekli degisken ve nesneleri tanimla */
    private static MainActivity instance;
    private static final String BROKER_URL = "tcp://test.mosquitto.org:1883";
    private static final String CLIENT_ID = "TELESERA";
    private static final String TAG = "MyTag";

    private String oldMsg = "";
    private MqttAndroidClient client;
    public static String arrivedMessage;

    /* Buton, edit text ve text view degiskenlerini tanimla */
    private static ImageButton button1, button2, button3, button4;

    private static EditText etd1, etd2, etd3;

    public static TextView txtTemp, txtHum, txtLight, txtTerrHum, txtFan, txtRMSpeed, txtSysTime;
    public static  boolean isConnected = false;
    public static  boolean isFanTurnedOn = false;

    /* Uygulama baslatildiginda asagida verilenleri yap */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        /* XML dosyasindaki nesneleri kaydet */
        button1 = findViewById(R.id.imageButton4);
        button2 = findViewById(R.id.imageButton5);
        button3 = findViewById(R.id.imageButton6);
        button4 = findViewById(R.id.imageButton2);
        txtTemp = findViewById(R.id.textView2);
        txtHum = findViewById(R.id.textView3);
        txtLight = findViewById(R.id.textView4);
        txtTerrHum = findViewById(R.id.textView5);
        txtFan = findViewById(R.id.textView6);
        txtSysTime = findViewById(R.id.textView7);
        etd1 = findViewById(R.id.editTextNumberDecimal6);
        etd2 = findViewById(R.id.editTextNumberDecimal7);
        etd3 = findViewById(R.id.editTextNumberDecimal5);

        initConnectButton();

        /* MQTT client tanimlamasi yap */
        client = new MqttAndroidClient(this.getApplicationContext(), BROKER_URL, CLIENT_ID);

        /* Buton icin listener olustur */
        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        /* Baglanti yoksa buton goruntusunu degistir ve baglan */
                        if(!isConnected) {
                            isConnected = true;
                            button1.setImageResource(R.drawable.no_wifi);
                            conectX();
                        }
                        /* Baglanti varsa buton goruntusunu degistir ve baglantiyi kopar*/
                        else
                        {
                            button1.setImageResource(R.drawable.internet);
                            isConnected = false;
                            try {
                                client.disconnect();
                            } catch (MqttException e) {
                                throw new RuntimeException(e);
                            }
                        }
                    }
                });

            }
        });

        /* Buton icin listener olustur */
        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        /* Fan kapaliysa buton simgesini ve text view i degistir, server a bilgiyi gonder */
                        if(!isFanTurnedOn) {
                            button2.setImageResource(R.drawable.fan_off);
                            publish("TELESERA/fan", "1");
                            isFanTurnedOn = true;
                            txtFan.setText("Fan: Açık");
                        }
                        /* Fan aciksa buton simgesini ve text view i degistir, server a bilgiyi gonder */
                        else
                        {
                            button2.setImageResource(R.drawable.fan_on);
                            publish("TELESERA/fan", "0");
                            isFanTurnedOn = false;
                            txtFan.setText("Fan: Kapalı");
                        }
                    }
                });

            }
        });

        /* Buton icin listener olustur */
        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        /* Server a anlik sulama istegi bilgisini gonder */
                        publish("TELESERA/instant_watering", "1");
                    }
                }, 0);

            }
        });

        /* Buton icin listener olustur */
        button4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if(!etd1.getText().toString().isEmpty())
                        {
                            /* Edit text boş değilse server a anlik sulama zamani bilgisini gonder */
                            publish("TELESERA/instant_watering_time", etd1.getText().toString());
                        }
                        if(!etd2.getText().toString().isEmpty())
                        {
                            /* Edit text boş değilse server a sulama zamani bilgisini gonder */
                            publish("TELESERA/watering_time", etd2.getText().toString());
                        }
                        if(!etd3.getText().toString().isEmpty())
                        {
                            /* Edit text boş değilse server a bekleme zamani bilgisini gonder */
                            publish("TELESERA/waiting_time", etd3.getText().toString());
                        }
                    }
                }, 0);

            }
        });
    }

    public void conectX()
    {

        try {
            IMqttToken token = client.connect();
            token.setActionCallback(new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    /* Baglanti saglandiysa TELESERA topiclerine abone ol */
                    Log.d(TAG, "onSuccess!!!");
                    sub("TELESERA/temp");
                    sub("TELESERA/hum");
                    sub("TELESERA/light");
                    sub("TELESERA/terr_hum");
                    sub("TELESERA/fan");
                    sub("TELESERA/systime");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    /* Hata olmasi durumunda yazdir */
                    Log.d(TAG, "onFailure!!!");

                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }

    }

    private void initConnectButton()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if(!isConnected) {
                    /* Baglanti saglanmamissa buton goruntusunu degistir */
                    button1.setImageResource(R.drawable.internet);
                }
                else
                {
                    /* Baglanti saglandiysa buton goruntusunu degistir */
                    button1.setImageResource(R.drawable.no_wifi);
                }
            }
        });
    }

    private void sub(String topic)
    {
        try {
            /* Belirtilen topic e abone ol */
            client.subscribe(topic, 0);
            /* Callback fonksiyonunu ayarla */
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                                /* Baglanti koptuysa buton goruntusunu degistir */
                                button1.setImageResource(R.drawable.internet);
                                isConnected = false;
                        }
                    });
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    /* Gelen mesaji degiskene kaydet */
                    String msg = new String(message.getPayload());
                    Log.d(TAG, "topic: " + topic);
                    Log.d(TAG, "message: " + msg);

                    /* Gelen topic e gore text view degerlererini guncelle */
                    switch(topic) {
                        case "TELESERA/temp": txtTemp.setText("Sıcaklık: " + msg + "°C"); break;
                        case "TELESERA/hum": txtTerrHum.setText("Nem: %" + msg + ""); break;
                        case "TELESERA/light": txtLight.setText("Işık: %" + msg + ""); break;
                        case "TELESERA/terr_hum": txtHum.setText("Tprk Nemi: %" + msg + ""); break;
                        case "TELESERA/systime": txtSysTime.setText("Sys Zmanı: " + msg + "dk"); break;
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {

                }
            });
            /* Hata olmasi durumunda yakala */
        }catch (MqttException e){

        }
    }

    public void publish(String topic, String message)
    {
        try {
            /* Mqttmessage nesnesi olusturup olusturulan nesneyi yayınla */
            MqttMessage mqttMessage = new MqttMessage((message.getBytes()));
            client.publish(topic, mqttMessage);
            /* Hata olmasi durumunda yakala ve yazdir */
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public static MainActivity getInstance() {
        return instance;
    }

}