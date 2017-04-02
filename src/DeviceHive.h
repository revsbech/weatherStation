#include <Arduino.h>
#include <ESP8266HTTPClient.h>

struct geoPosition {
  float longitude;
  float latitude;
};

struct deviceClass {
  String Name;
  String Version;
};

struct device {
  String Id;
  String Name;
  deviceClass DeviceClass;
  String NetworkName;
  String FirmwareVersion;
  geoPosition Position;
  String Key = "";
  String Status = "Online";
};

struct equipmentNotification {
  String equipment;
  float value;
  
};

class DeviceHive 
{
public:
  DeviceHive(String baseUrl, HTTPClient* http);
  bool RegisterDevice(device* d);
  bool sendEquipmentNotification(device* d, equipmentNotification);
  bool sendBatchEquipmentNotification(device*, equipmentNotification[], int );
protected:
  HTTPClient* http;
  String baseUrl;
};

