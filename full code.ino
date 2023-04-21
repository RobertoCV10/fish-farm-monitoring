//Sensor libraries
#include <NewPing.h> // hc-sr04
#include <DallasTemperature.h> //temparature
#include <OneWire.h> // dont know
#include <MQ135.h> //ammonium

//connection libraries
#include <WiFi.h> // Include WiFi library

//Config libraries
#include "config.h"

//Adafruit libraries
#include <AdafruitIO_WiFi.h> // Include Adafruit IO WiFi library
#include <AdafruitIO.h> // Include Adafruit IO library


    //    Variables   
//hc-sr04
#define TRIGGER_PIN  5 // Define the trigger pin
#define ECHO_PIN     18 // Define the echo pin
#define MAX_DISTANCE 200 // Define the maximum distance in cm
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Define the sonar object

//ammonium
MQ135 gasSensor(33);//analog pin

//dont know
#define LED_PIN 5
const int WATERTEMPERATURET = 4; 

//salinity meter
int analogPin = 0;
int dtime = 500;
int raw = 0;
int Vin = 5;
float Vout = 0;
float R1 = 1000;
float R2 = 0;
float buff = 0;
float avg = 0;
int samples = 5000;

float CalibrationTemp = 21.0; //Temperature Calibration
float CorrectionFactor = 1.02; //Correction factor acording to temperature
float dtemp = 0; //Initial value for temperature difference


    //    Wifi setup     
#define WIFI_SSID       "blablabla" // Your wifi SSID
#define WIFI_PASS       "pasword" // Your wifi password


    //    Adafruit IO Setup
#define IO_USERNAME     "user" // Your Adafruit IO username
#define IO_KEY          "change it" // Your Adafruit IO key



    //    Adafruit and Feeds setup connection  
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS); // Initialize Adafruit IO WiFi
AdafruitIO_Feed *distance = io.feed("distance"); // hc-sr04
AdafruitIO_Feed *WaterTemperature = io.feed("WaterTemperature");// temperature
AdafruitIO_Feed *Salinity = io.feed("Salinity");// salinity
AdafruitIO_Feed *ammonia = io.feed("ammonia");// ammonium


    //    oneWire setup
OneWire oneWire(WATERTEMPERATURET);//Setup a oneWire instance to comunicate with any OneWire Device
DallasTemperature sensors(&oneWire);//Pass our oneWire reference to dallas temperature sensor


void setup() {
  // Start serial communication 
  Serial.begin(115200);

  //hc-sr04
  pinMode(TRIGGER_PIN, OUTPUT); // Set trigger pin as output
  pinMode(ECHO_PIN, INPUT); // Set echo pin as input

  //dont now
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(LED_PIN, OUTPUT);//led pin

  //Extra setup
  sensors.begin();//dont know

  //conection wifi alert HIM
  WiFi.begin(WIFI_SSID, WIFI_PASS); // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) { // Wait for WiFi connection
    delay(500);
    Serial.println("Connecting to WiFi...");}
  Serial.println("WiFi connected");

  //conection adafruit alert HIM
  io.connect(); // Connect to Adafruit IO
  while(io.status() < AIO_CONNECTED) { // Wait for Adafruit IO connection
    delay(500);
    Serial.println("Connecting to Adafruit IO...");}
  Serial.println("Adafruit IO connected");

  //dont know
  WaterTemperature->onMessage(handleMessage);
  Salinity->onMessage(handleMessage);
  WaterTemperature->get();
  Salinity->get();

}
    //    Main loop
void loop() {

  //hc-sr04 measure
  unsigned int uS = sonar.ping(); // Send a ping and get the duration in microseconds
  float distance_cm = sonar.convert_cm(uS); // Convert ping duration to distance in cm
  

  //ammonium measure
  float ammoniaLevel = gasSensor.getPPM();//MQ137 sensor measurement in parts per million (ppm) of ammonium

  //temperature measure
  sensors.requestTemperatures();
  float WATERTEMPERATURET= sensors.getTempCByIndex(0);


  //dont know
    float tot = 0;
  for ( int i = 0; i < samples; i++){
    digitalWrite(25, HIGH);
    digitalWrite(26, LOW);
    delayMicroseconds(dtime);
    digitalWrite(25, LOW);
    digitalWrite(26, HIGH);
    delayMicroseconds(dtime);
    raw = analogRead(analogPin);

    if (raw){
      buff = raw * Vin;
      Vout = (buff)/1024.0;
      buff = (Vin/Vout)-1;
      R2 = R1 * buff;
      tot = tot + R2;
    }
  }


  //dont know
  avg = tot/samples;
  dtemp = WATERTEMPERATURET - CalibrationTemp;
  avg = avg * pow(CorrectionFactor, dtemp);


  //hc-sr04 feedback
  distance->save(distance_cm); // Save the distance to the Adafruit IO feed
  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
  Serial.println();

  //ammonium feedback
  Serial.print("Ammonia: ");
  Serial.print(ammoniaLevel);
  Serial.println(" ppm");
  ammonia->save(ammoniaLevel);
  Serial.println();

  //temperature feedback
  Serial.print(WATERTEMPERATURET);
  Serial.println("ºC");
  WaterTemperature->save(WATERTEMPERATURET);
  Serial.println();

  //salinity feedback
  Serial.print("Water Salinity: " + String(avg));
  Salinity->save(avg);


  //global delay
  delay(5000); // Wait for 5 second before taking another reading
}

//dont know
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  if(data->toPinLevel() == 1)
    Serial.println("HIGH");
  else
    Serial.println("LOW");

  // Write the current state to relay
  digitalWrite(LED_PIN, data->toPinLevel());
}