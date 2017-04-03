#include "DeviceHive.h"
#include <ArduinoJson.h>

DeviceHive::DeviceHive(String bu, HTTPClient* ht) {
  baseUrl = bu;
  http = ht;
}

bool DeviceHive::RegisterDevice(device* d) {
  Serial.println("Registering device " + d->Name  + " in deviceclass " + d->DeviceClass.Name);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = d->Name;
  root["status"] = d->Status;
  root["key"] = d->Key;

  JsonObject& deviceClass = root.createNestedObject("deviceClass");
  deviceClass["name"] = d->DeviceClass.Name;
  deviceClass["Version"] = d->DeviceClass.Version;

  JsonObject& data = root.createNestedObject("data");
  data["latitude"] = d->Position.latitude;
  data["longitude"]  = d->Position.longitude;
  data["timezone"] = "Europe/Copenhagen";
  data["firmware"] = d->FirmwareVersion;

  String payload;
  root.printTo(payload);
  Serial.println("URL: " + baseUrl + "device/" + d->Id);
  http->begin(baseUrl + "device/" + d->Id);

  http->addHeader("Auth-DeviceID", d->Id);
  http->addHeader("Auth-DeviceKey", d->Key);

  http->addHeader("Content-Type","application/json");
  int httpCode = http->sendRequest("PUT", payload);
  if(httpCode > 0) {
    if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_ACCEPTED) {
      Serial.println("Device registered");
      http->end();
      return true;
    } else {
      Serial.printf("[HTTP] PUT... code: %d\n", httpCode);
      String returnPayload = http->getString();
      //Serial.println(strcat("Return: ", returnPayload.c_str()));
    }
  } else {
    Serial.printf("Device registration failed, error: %s\n", http->errorToString(httpCode).c_str());
  }
  http->end();
  return false;

}

bool DeviceHive::sendEquipmentNotification(device* d, equipmentNotification notification) {
  Serial.println("Sending equipment notification for device " + d->Name);
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["notification"] = "equipment";

  JsonObject& parameters = root.createNestedObject("parameters");
  parameters["equipment"] = notification.equipment;
  parameters["value"] = double_with_n_digits(notification.value,6);

  String payload;
  root.printTo(payload);

  http->begin(baseUrl + "device/" + d->Id + "/notification");
  http->addHeader("Auth-DeviceID", d->Id);
  http->addHeader("Auth-DeviceKey", d->Key);
  http->addHeader("Content-Type","application/json");

  int httpCode = http->sendRequest("POST", payload);
  if(httpCode > 0) {
    Serial.printf("Code: %d", httpCode);
    if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_ACCEPTED) {
      Serial.println("Notification sent");
      http->end();
      return true;
    }
  } else {
    Serial.printf("Device notification failed, error: %s\n", http->errorToString(httpCode).c_str());
  }
  http->end();
  return false;
}

bool DeviceHive::sendBatchEquipmentNotification(device* d, equipmentNotification notifications[], int numberOfNotifications) {

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["notification"] = "batch";

  JsonObject& jsonParameters = root.createNestedObject("parameters");
  JsonArray& jsonNotifications = jsonParameters.createNestedArray("notifications");

  for(int i = 0; i < numberOfNotifications; ++i)
  {

    JsonObject& jsonNotification = jsonBuffer.createObject();
    jsonNotification["notification"] = "equipment";

    JsonObject& equipmentParameters = jsonNotification.createNestedObject("parameters");
    equipmentParameters["equipment"] = notifications[i].equipment;
    equipmentParameters["value"] = double_with_n_digits(notifications[i].value,6);
    jsonNotifications.add(jsonNotification);
  }

  String payload;
  root.printTo(payload);

  http->begin(baseUrl + "device/" + d->Id + "/notification");
  http->addHeader("Auth-DeviceID", d->Id);
  http->addHeader("Auth-DeviceKey", d->Key);
  http->addHeader("Content-Type","application/json");

  int httpCode = http->sendRequest("POST", payload);
  if(httpCode > 0) {
    Serial.printf("Code: %d", httpCode);
    if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_ACCEPTED) {
      Serial.println("Notification sent");
      http->end();
      return true;
    }
  } else {
    Serial.printf("Device batch notification failed, error: %s\n", http->errorToString(httpCode).c_str());
  }

  http->end();
  return false;
}
