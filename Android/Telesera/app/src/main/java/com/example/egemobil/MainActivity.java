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

    private static MainActivity instance;
    private static final String BROKER_URL = "tcp://test.mosquitto.org:1883";
    private static final String CLIENT_ID = "TELESERA";
    private static final String TAG = "MyTag";

    private String oldMsg = "";
    private MqttAndroidClient client;
    public static String arrivedMessage;

    private static ImageButton button1, button2, button3, button4;

    private static EditText etd1, etd2, etd3;

    public static TextView txtSpeed, txtAngle, txtAcclr, txtBrake, txtLMSpeed, txtRMSpeed, txtLMTemp,
            txtRMTemp, txtBatLvl, txtBatVol, txtBatTemp, txtBatSmk, txtDistnc, txtOTemp, txtPing, txtSysTime;
    public static  boolean isConnected = false;
    public static  boolean isFanTurnedOn = false;
    private Timer timer;
    public static String topicToSend = "";
    public static String messageToSend = "";
    public static int toastMessageKey = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        button1 = findViewById(R.id.imageButton4);
        button2 = findViewById(R.id.imageButton5);
        button3 = findViewById(R.id.imageButton6);
        button4 = findViewById(R.id.imageButton2);
        txtSpeed = findViewById(R.id.textView2);
        txtAngle = findViewById(R.id.textView3);
        txtAcclr = findViewById(R.id.textView4);
        txtBrake = findViewById(R.id.textView5);
        txtLMSpeed = findViewById(R.id.textView6);
        txtRMSpeed = findViewById(R.id.textView7);
        etd1 = findViewById(R.id.editTextNumberDecimal6);
        etd2 = findViewById(R.id.editTextNumberDecimal7);
        etd3 = findViewById(R.id.editTextNumberDecimal5);

        initConnectButton();

        client = new MqttAndroidClient(this.getApplicationContext(), BROKER_URL, CLIENT_ID);
        //conectX();

        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {

                if(messageToSend != "")
                {
                    //publish(topicToSend, messageToSend);
                    //topicToSend = "";
                    //messageToSend = "";
                }

                if(toastMessageKey == 1)
                {
                    //Toast.makeText(getApplicationContext(),"The key has already been added.", Toast.LENGTH_SHORT).show();
                }

            }
        }, 0, 500);

        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if(!isConnected) {
                            isConnected = true;
                            button1.setImageResource(R.drawable.no_wifi);
                            conectX();
                        }
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

        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if(!isFanTurnedOn) {
                            button2.setImageResource(R.drawable.fan_off);
                            publish("TELESERA/fan", "1");
                            isFanTurnedOn = true;
                        }
                        else
                        {
                            button2.setImageResource(R.drawable.fan_on);
                            publish("TELESERA/fan", "0");
                            isFanTurnedOn = false;
                        }
                    }
                });

            }
        });

        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        publish("TELESERA/instant_watering", "1");
                    }
                }, 0);

            }
        });

        button4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if(!etd1.getText().toString().isEmpty())
                        {
                            publish("TELESERA/instant_watering_time", etd1.getText().toString());
                        }
                        if(!etd2.getText().toString().isEmpty())
                        {
                            publish("TELESERA/watering_time", etd2.getText().toString());
                        }
                        if(!etd3.getText().toString().isEmpty())
                        {
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
                    // We are connected
                    Log.d(TAG, "onSuccess!!!");
                    sub("TELESERA/temp");
                    sub("TELESERA/hum");
                    sub("TELESERA/light");
                    sub("TELESERA/terr_hum");
                    sub("TELESERA/fan");
                    sub("TELESERA/systime");
                    //sub("TELESERA/lmtemp");
                    //sub("TELESERA/rmtemp");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    // Something went wrong e.g. connection timeout or firewall problems
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
                    button1.setImageResource(R.drawable.internet);
                }
                else
                {
                    button1.setImageResource(R.drawable.no_wifi);
                }
            }
        });
    }

    private void sub(String topic)
    {
        try {
            client.subscribe(topic, 0);
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                                button1.setImageResource(R.drawable.internet);
                                isConnected = false;
                        }
                    });
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    String msg = new String(message.getPayload());
                    Log.d(TAG, "topic: " + topic);
                    Log.d(TAG, "message: " + msg);

                    switch(topic) {
                        case "TELESERA/temp": txtSpeed.setText("Sıcaklık: " + msg + "°C"); break;
                        case "TELESERA/hum": txtAngle.setText("Nem: %" + msg + ""); break;
                        case "TELESERA/light": txtAcclr.setText("Işık: %" + msg + ""); break;
                        case "TELESERA/brake": txtBrake.setText("Tprk Nemi: %" + msg + ""); break;
                        case "TELESERA/terr_hum": txtLMSpeed.setText("Fan :" + msg + ""); break;
                        case "TELESERA/fan": txtRMSpeed.setText("Sys Zmanı: " + msg + "dk"); break;
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {

                }
            });
        }catch (MqttException e){

        }
    }

    public void publish(String topic, String message)
    {
        try {
            MqttMessage mqttMessage = new MqttMessage((message.getBytes()));
            client.publish(topic, mqttMessage);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public static MainActivity getInstance() {
        return instance;
    }

}