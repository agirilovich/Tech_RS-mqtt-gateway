#include "controlWiFi.h"
//Import credentials from external file out of git repo
#include <Credentials.h>
const char *ssid = ssid_name;
const char *password = ssid_password;

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  // you're connected now, so print out the data
  Serial.print(F("You're connected to the network, IP = "));
  Serial.println(WiFi.localIP());

  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  Serial.print("Signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void initializeWiFi(const char *device_name)
{
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  
  char fqdn[13];
  strcpy(fqdn, device_name);
  WiFi.setHostname(fqdn);
  WiFi.setAutoReconnect(true);

}

void establishWiFi()
{
  
  WiFi.disconnect(); // to clear the way. not persistent
  WiFi.enableAP(false); // to disable default automatic start of persistent AP at startup
  WiFi.setAutoConnect(true);

  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  delay(1000);

  // attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi shield init done");
}
