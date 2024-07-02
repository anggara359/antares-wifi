#include <SPI.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h> // library untuk I2C

#define RXD0 3
#define TXD0 1

#define WIFISSID "anggara"
#define PASSWORD "12345678"

// MQTT Broker
const char *mqtt_broker = "91.121.93.94";
const char *topic_pub = "iwamos/pub/anggara";
//const char *topic_sub = "robotikid/sub";
//const char *mqtt_username = "RobotikID";
//const char *mqtt_password = "1234567";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

String dataIn;
boolean parsing = false;

unsigned long lastUploadTime = 0;
const unsigned long uploadInterval = 5000; // Interval upload dalam milidetik (contoh: 60 detik)

void setup() {
  Serial.begin(9600, SERIAL_8N1, RXD0, TXD0);
  dataIn = "";
  
  WiFi.begin(WIFISSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Menghubungkan ke mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  
  // publish
  client.publish(topic_pub, "Connected to MQTT broker");

}

void parsingData() {
  int commaIndex = dataIn.indexOf(',');
  if (commaIndex != -1) {
    String idPelanggan = dataIn.substring(0, commaIndex);
    String penggunaanStr = dataIn.substring(commaIndex + 1);
    String penggunaan = penggunaanStr.substring(penggunaanStr.indexOf(":") + 1);
    if (idPelanggan.equals("1") || idPelanggan.equals("2") || idPelanggan.equals("3") || idPelanggan.equals("4")) {
      // Publish data ke MQTT
      String message = idPelanggan + "," + penggunaan;

//    unsigned long currentMillis = millis(); // Ambil waktu saat ini

//  if (currentMillis - lastUploadTime >= uploadInterval) {
      client.publish(topic_pub, message.c_str());
      Serial.println("Data published to MQTT");
//    lastUploadTime = currentMillis; 
//  }
     
    }
  }
}

void receiveData() {
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    if (inChar == '*') {
      parsing = true;
      dataIn = "";
    } else if (inChar == '\n') {
      parsing = false;
    } else if (parsing) {
      dataIn += inChar;
    }
  }
}

void loop() {

  receiveData();
  if (!parsing && dataIn.length() > 0) {
    
    parsingData();
   
    dataIn = "";
  }
  
  // Periksa apakah sudah waktunya untuk mengunggah data ke MQTT
 
    
 
    // Loop MQTT client
    if (!client.connected()) {
      if (client.connect("ESP32Client")) {
        Serial.println("Reconnected to MQTT broker");
      } else {
        Serial.print("Failed to reconnect to MQTT broker, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
    client.loop();
  
}
