//
// DeviceHive Weather station
//

#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "BH1750.h"

#include "DeviceHive.h"
#include <ArduinoJson.h>

// Pins

// Defines
#define DHTTYPE DHT22   // DHT 22  (AM2302)

void blinkOk();

// Basic init
BH1750 lightMeter;
ESP8266WiFiMulti WiFiMulti;

DHT dht(D1, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino. Temp senson on pin D2
DHT dht2(D2, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino. Temp senson on pin D2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, D5, NEO_GRB + NEO_KHZ800);

// Configuration variables
bool do_deep_sleep = true; // IF set, do deep sleep instead of normal delay
int delayval = 30000; // delay between measurements in milliseconds
char *wifi_name = "1 er sejere end 4";
char *wifi_password = "elisabeth";
char *hiveEndpoint = "http://hive.grundfosbacmon.com/";
char *hiveDeviceClass = "RevsbechWeatherStation";
char *hiveDeviceClassVersion = "1.0.0";
char *hiveDeviceId = "greenhouse01";
char *hiveDeviceName = "Greenhouse weather station";

// Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
float hum2;
float temp2;
uint16_t lux; //Store lux
bool isRegistered = false;

device* thisDevice;
HTTPClient http;
DeviceHive dh(hiveEndpoint, &http);

void setup() {
  thisDevice = new device;
  thisDevice->Id = hiveDeviceId;
  thisDevice->Name = hiveDeviceName;
  thisDevice->DeviceClass = {hiveDeviceClass, hiveDeviceClassVersion};
  thisDevice->Position = {55.8684720, 12.3429770};
  thisDevice->FirmwareVersion = "beta";
  thisDevice->Key = "123";

  Serial.begin(9600);

  lightMeter.begin(D3, D4);
  pixels.begin(); // This initializes the NeoPixel library.
  dht.begin(); // Initialize dht library for sensor input
  dht2.begin();

  // Set pixel to blue before connecting
  pixels.setPixelColor(0, pixels.Color(150,0,0));
  pixels.show();

  delay(1000); // Do initial wait, to make sure it works
  WiFiMulti.addAP(wifi_name, wifi_password);
  Serial.printf("[SETUP] WAIT for wifi ");
  for(uint8_t t = 12; t > 0; t--) {
      Serial.print(".");
      Serial.flush();
      if((WiFiMulti.run() == WL_CONNECTED)) {
        Serial.printf("Connected!!!\n");
        break;
      } else {
        delay(1000);
      }
  }

}

void loop() {
  // @todo Move this to the setp function
  if((WiFiMulti.run() == WL_CONNECTED)) {
      if (isRegistered == false) {
        Serial.printf(" - Registering device in devicehive");
        isRegistered = dh.RegisterDevice(thisDevice);
        if (isRegistered == true) {
          blinkOk();
        }
      }
  } else {
    Serial.println("Not connected to wifi");
  }


  if (isRegistered) {
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    hum2 = dht2.readHumidity();
    temp2 = dht2.readTemperature();
    lux = lightMeter.readLightLevel();
    //Print temp and humidity values to serial monitor
    Serial.println("###### Sensor 1 ######");
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");

    Serial.println("###### Sensor 2 ######");
    Serial.print("Humidity: ");
    Serial.print(hum2);
    Serial.print(" %, Temp: ");
    Serial.print(temp2);
    Serial.println(" Celsius");

    Serial.println("###### Lightsensor ######");
    Serial.print(" Lux: ");
    Serial.println(lux);

    pixels.setPixelColor(0, pixels.Color(75,75,0));
    pixels.show();

    // Sending as individual notifications
    //dh.sendEquipmentNotification(thisDevice, {"temperature", temp});
    //dh.sendEquipmentNotification(thisDevice, {"humidity", hum});

    // Sending af batch notifications
    equipmentNotification notifications[5] = {{"temperature", temp},{"humidity", hum},{"temperature2", temp2},{"humidity2", hum2},{"luminosity", lux}};
    dh.sendBatchEquipmentNotification(thisDevice, notifications, 5);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.show();
  }

  if (do_deep_sleep)  {
    Serial.println("* Good night, going to sleep...");
    ESP.deepSleep(delayval * 1000); //deepSleep expects nano seconds, delayval is in milliseconds
  } else {
    Serial.println("* I'm just going to wait here for a while. No sleeping");
    delay(delayval); // Delay for a period of time (in milliseconds).
  }

}

void blinkOk() {
  for(uint8_t t = 3; t > 0; t--) {
      // Set pixel to green for succesfully conntected //@todo Blink three times for ok, and then turn off.
      pixels.setPixelColor(0, pixels.Color(0,30,0));
      pixels.show();
      delay(200);
      pixels.setPixelColor(0, pixels.Color(0,0,0));
      pixels.show();
      delay(200);
  }
}
