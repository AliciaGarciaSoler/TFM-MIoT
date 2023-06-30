// *************************************************************
// ***********            LIBRARIES            *****************
#include <Adafruit_NeoPixel.h>
#include <SCD30.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MQTT.h>
#include <time.h>
#include "LiquidCrystal.h"
#include <Ticker.h>


// *************************************************************
//************      DeepSleep DEFINITION       *****************

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 27           // Time ESP32 will go to sleep (in seconds)
#define PIN_BITMASK 0x100008014
char topic[50] = "test";
char msg[50] = "test";
RTC_DATA_ATTR float co2;
RTC_DATA_ATTR float temperature;
RTC_DATA_ATTR float humidity;

// *************************************************************
//************      SCD30 PIN DEFINITION       *****************
#define I2C_SDA 21                 // Pin definition for I2C
#define I2C_SCL 22

//************      LED STRIP PIN DEFINITION       *************
#define PIXEL_PIN 16         // Pin definition
#define PIXEL_COUNT 8        // Number of LEDs

#define WHITE 255, 255, 255  // Color definition
#define RED 255, 0, 0
#define ORANGE 255, 165, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT,PIXEL_PIN,NEO_GRB+NEO_KHZ800);  // LED strip declaration

//************        LCD PIN DEFINITION       *****************

const int RS = 33, EN = 25, d4 = 26, d5 = 27, d6 = 14, d7 = 13;  // LCD pin
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);  


// *************************************************************
//************ For the MQTT communication **********************
const char* server = "192.168.0.9";         // IP of the mosquitto broker
const char* ssid = "Telecable202B";         // Wifi connection
const char* password = "57B208454E314540";  // WiFi password
const char* mqttUsername = "shed";          // Username for broker authentication
const char* mqttPassword = "3stud10e";      // Password for broker authentication
String clientId = "clientESP32_classroomA";
int QoS = 0;
WiFiClient myCliente;             // WiFi client declaration


PubSubClient client(myCliente);  // MQTT client declaration
                     

// *************************************************************
//************      FLAME PIN DEFINITION       *****************

#define flameSensorPin 15  // Flame sensor pin definition

// *************************************************************
//************      SOUND PIN DEFINITION       *****************

#define soundSensorPin 4   // Sound sensor pin definition


// *************************************************************
//************        PIR PIN DEFINITION       *****************

#define pirSensorPin 2     // PIR sensor pin definition

// *************************************************************
//************      TOUCH PIN DEFINITION       *****************

#define touchSensorPin 32  // Touch sensor pin definition


// ************************************************************
//                 INITIAL SETUP FUNCTIONS
// ************************************************************


void scd30Setup() {
  Wire.begin(I2C_SDA, I2C_SCL);     // Sets the communication with the sensor
  scd30.initialize();               // Initializes the sensor

  scd30.setTemperatureOffset(600);  // Set offset in hundreds of degree
}

void ledstripSetup() {
  strip.begin();  // Initializes the LED strip
  strip.show();   // Turn the LED strip on
}

void lcdSetup() {
  lcd.begin(16, 2);               // Set the LCD 16 collumns, 2 lines
  lcd.noDisplay();
}

void flameSetup() {
  pinMode(flameSensorPin, INPUT);  // Declares the sensor pin as input
}

void touchSetup() {
  pinMode(touchSensorPin, INPUT);  // Declares the sensor pin as input
}

void soundSetup() {
  pinMode(soundSensorPin, INPUT);  // Declares the sensor pin as input
}

void pirSetup() {
  pinMode(pirSensorPin, INPUT);    // Declares the sensor pin as input
}

// ************************************************************
//                WAKE UP REASON
// ************************************************************
void wakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {

    case ESP_SLEEP_WAKEUP_EXT1:
      {
        uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
        int GPIO_pin = (log(GPIO_reason)) / log(2);
        lcdSetup();
        delay(100);
        QoS = 1;
        
        updateLCD();
        switch (GPIO_pin) {
          case 32:
            touchSetup();
            touchDetector(topic, msg);
            break;
          case 4:
            soundSetup();
            soundDetector(topic, msg);
            break;
          case 15:
            flameSetup();
            flameDetector(topic, msg);
            break;
          case 2:
            pirSetup();
            pirDetector(topic, msg);
            break;
        }

        break;
      }
    case ESP_SLEEP_WAKEUP_TIMER:
      scd30Setup();  // Call on the function to set up the SCD30 connection
      delay(10);
      ledstripSetup();
      delay(10);
    
      lcdSetup();
      delay(100);
      scd30Process(topic, msg);  
      QoS = 0;
      break;

  }
}




// ************************************************************
//                    SETUP
// ************************************************************
void setup() {


  wakeupReason();                            // Studies the reason why the board has woken up
  
  
  setupWifi();                               // Sets up wifi connection
  delay(1500);                               
  client.set_server(server, 1883);           // Establish MQTT connection

  if (!client.connected()) {                 // Set reconnection loop 
    reconnect();
  }
  client.loop();                             // Sets the client loop
                 
  client.publish(MQTT::Publish(topic, msg).set_qos(QoS));             


  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);          
  esp_sleep_enable_ext1_wakeup(PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);    

  client.disconnect();
  delay(100);                                
  esp_deep_sleep_start();                   // Set the board to sleep                   
}
// ************************************************************
//                    WiFi SETUP
// ************************************************************
void setupWifi() {

  WiFi.begin(ssid, password);              // Sets up the connection

  while (WiFi.status() != WL_CONNECTED) {  // Attempting to connect to the WiFi
    delay(500);                            // Waits 5 seconds before retrying
    
  }
}

void reconnect() {

  while (!client.connected()) {                         // Loop to connect
    if (client.connect(MQTT::Connect(clientId.c_str())
			 .set_auth("shed", "3stud10e"))) {            // Trying to connect
     
    } else {
      delay(5000);      // Wait 5s before retrying
    }
  }
}
// ************************************************************
//               MESSAGE PROCESS FUNCTIONS
// ************************************************************

void updateLCD() {

  lcd.display();
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print(String(temperature, 1) + (char)223 + "C  " + String(humidity, 1) + "%HR");   // Display the content on LCD
  lcd.setCursor(0, 1);
  lcd.print(String(co2, 1) + " ppm");
  lcd.noCursor();                                                                        // Disable cursor so nothing else can be written on the LCD

}
void scd30Process(char* topic, char* msg) {
  float temp = 20;
  float result[3] = { 0 };

  if (scd30.isAvailable()) {                          // If the scd30 is available

    scd30.getCarbonDioxideConcentration(result);      // Get SCD30 values
    co2 = result[0];
    temperature = result[1];
    humidity = result[2];

    updateLCD();                                      // Update the LCD

    char co2str[8];
    dtostrf(result[0], 1, 2, co2str);                 // Parse the co2 value to string


    char tempstr[8];
    dtostrf(result[1], 1, 2, tempstr);                // Parse the temperature value to string


    char humstr[8];
    dtostrf(result[2], 1, 2, humstr);                 // Parse the fumidity value to string

    char separator[] = "/";
    strcpy(msg, co2str);                              // Prepare payload with the data
    strcat(msg, separator);
    strcat(msg, tempstr);
    strcat(msg, separator);
    strcat(msg, humstr);
    strcpy(topic, "estudio3/classroomA/data/SCD30");  // Prepare topic

    // ******************* LED STRIP SET UP ***************************

    strip.clear();                                    // Turn off all LEDs
    temp = result[1];

    if (temp > 30.0) {                                // Set the colors based on temperature conditions
      for (int i = 0; i < 8; i++) {
        strip.setPixelColor(i, RED);                  // Temperature > 30: All LEDs light up red
      }
    } else if (temp > 27.0) {
      for (int i = 0; i < 7; i++) {
        strip.setPixelColor(i, RED);                  // 27 < Temperature <= 30: 7/8 LEDs light up red
      }
    } else if (temp > 25.0) {
      for (int i = 0; i < 6; i++) {
        strip.setPixelColor(i, ORANGE);               // 25 < Temperature <= 27: 6/8 LEDs light up orange
      }
    } else if (temp > 17.0) {
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(i, GREEN);                // 17 < Temperature <= 25: 5/8 LEDs light up green
      }
    } else if (temp > 15.0) {
      for (int i = 0; i < 4; i++) {
        strip.setPixelColor(i, GREEN);                // 15 < Temperature <= 17: 4/8 LEDs light up green
      }
    } else if (temp > 13.0) {
      for (int i = 0; i < 3; i++) {
        strip.setPixelColor(i, BLUE);                 // 13 < Temperature <= 15: 3/8 LEDs light up blue
      }
    } else if (temp < 13.0) {
      strip.setPixelColor(1, BLUE);                   // Temperature <= 13: Only the second LED lights up blue
    }

    strip.setPixelColor(0, WHITE);                    // First LED always turns on white
    strip.show();                                     // Update NeoPixel strip

    
   
  }
}

void flameDetector(char* topic, char* msg) {

  strcpy(msg, "1");                                   // Prepare message to be sent
  strcpy(topic, "estudio3/classroomA/alert/flame");   // Prepare topic
}

void soundDetector(char* topic, char* msg) {

  strcpy(msg, "1");                                   // Prepare message to be sent
  strcpy(topic, "estudio3/classroomA/alert/noise");   // Prepare topic
}

void pirDetector(char* topic, char* msg) {

  strcpy(msg, "1");                                   // Prepare message to be sent
  strcpy(topic, "estudio3/classroomA/alert/pir");     // Prepare topic
}
void touchDetector(char* topic, char* msg) {

  strcpy(msg, "1");                                   // Prepare message sent
  strcpy(topic, "estudio3/classroomA/alert/touch");   // Prepare topic
}

void loop() {
  // Never gets here
}
