#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Wire.h>    // For I2C communication with BH1750 sensor
#include <BH1750.h>  // For BH1750 light intensity sensor
#include <BlynkSimpleEsp32.h>
// WiFi credentials
#define SOUND_SPEED 0.034
long duration;
float distanceCm;
char auth[] = "chFqrepM5Vs9mqUB-ns2HyDzqWUH-fE9";
char ssid[] = "WiFi-name";
char pass[] = "WiFi-password";
// Pins declaration
const int trigPin = 27;
const int echoPin = 14; 
const int ledPin1 = 16;
const int soundSensorPin = 35;
// Define the threshold values for motion, sound, and light
const int soundThreshold = 150;  // Adjust according to ambient noise
const int lightThreshold = 5;    // Adjust according to ambient light
const int PWMFreq = 5000;
const int PWMChannel = 0;
const int PWMResolution = 8;
BH1750 lightSensor;
// Variables to store sensor readings
int motionValue = 0;
int soundValue;
float lightValue = 0.0;
// Variable to track the state of light from Blynk app
static bool masterControl = true;
static bool lightOn = false;
void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  // pinMode(pirPin, INPUT);
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  ledcAttachPin(ledPin1, PWMChannel);
  Wire.begin();
  lightSensor.begin();
  Serial.print("Initial Light State: ");
  Serial.println(lightOn);
  pinMode(soundSensorPin, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}
void loop() {
  Blynk.run();
  // Read sensor values
  // motionValue = digitalRead(pirPin);
  soundValue = digitalRead(soundSensorPin);
  lightValue = lightSensor.readLightLevel();

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  Serial.println(duration);
  distanceCm = duration * SOUND_SPEED/2;

  // Serial.print("Motion: ");
  // Serial.print(motionValue);
  Serial.print("Distance (in cm): ");
  Serial.println(distanceCm);
  Serial.print(" | Sound: ");
  Serial.print(!soundValue);
  Serial.print(" | Light: ");
  Serial.println(lightValue);
  
  if (masterControl)
    return;
  if (lightValue < lightThreshold) {
    if (distanceCm < 13 || soundValue < 1) {
      ledcWrite(PWMChannel, 255);
      delay(2000);
    } else {
      ledcWrite(PWMChannel, 10);
    }
  } else {
    ledcWrite(PWMChannel, 0);
  }
  delay(30);
   double soundLevel = map(soundValue, 0, 1023, 0, 100);  // Map the analog value to a range of 0 to 100
   Blynk.virtualWrite(V2, soundLevel);
   Blynk.virtualWrite(V3, lightValue);
   if (motionValue == HIGH)
  {
    Blynk.virtualWrite(V1, "Motion detected!");
  }
  else
  {
    Blynk.virtualWrite(V1, "No motion detected!");
  }
}
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}
BLYNK_WRITE(V0)  // Button widget on V0
{
  int value = param.asInt();  // Get value from the button
  BLYNK_LOG("Got value in V0 %d ",value);
  lightOn = (value == HIGH);
  Serial.print("Light Status :");
  Serial.println(lightOn);
  if (!masterControl)
    return;
  if (lightOn) {
    ledcWrite(PWMChannel, 255);
  } else {
    ledcWrite(PWMChannel, 0);
  }
}
BLYNK_WRITE(V4)  // Button widget on V4
{
  int value = param.asInt();  // Get value from the button
  BLYNK_LOG("Got value in V4 %d ",value);
  masterControl = (value == HIGH);
  Serial.print("Master Control Status :");
  Serial.println(masterControl);
  if (!masterControl)
    return;
  if (lightOn) {
    ledcWrite(PWMChannel, 255);
  } else {
    ledcWrite(PWMChannel, 0);
  }
}
