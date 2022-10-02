#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <SPIFFS.h>
#include <vector>
#include <map>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>

class Spiffs {
  private: File file;

  public: Spiffs(String file, const char* type) {
    this->file = SPIFFS.open(file, type);

    if (!file){
      Serial.println("Error opening file");
    } else {
      Serial.println("Success opening file");
    }
  }

  public: void print(String message) {
    if (this->file.println(message)){
      Serial.println("File was written");
    }else{
      Serial.println("File was not written");
    }
  }

  public: void close() {
    this->file.close();
  }

  public: std::vector<String> getData() {
    std::vector<String> dataList;
    
    while (this->file.available()) {
      dataList.push_back(this->file.readStringUntil('\n'));
    }

    return dataList;
  }

  public: void writeData(std::vector<String> dataList) {
    String message = "";
    
    for (String dataItem : dataList) {
      message += dataItem + "\n";
    }

    this->print(message);
  }

  public: void writeData(String message) {
    this->print(message);
  }
};

class Wifi {
  private: String ssid;
  private: String password;

  public: Wifi(String ssid, String password) {
    this->ssid = ssid;
    this->password = password;
  }

  public: String getSsid() {
    return this->ssid;
  }

  public: void setSsid(String ssid) {
    this->ssid = ssid;
  }

  public: String getPassword() {
    return this->password;
  }

  public: void setPassword(String password) {
    this->password = password;
  }

  public: static String wifiEncode(Wifi* wifi) {
    return wifi->getSsid() + "|" + wifi->getPassword() + "|";
  }

  public: static Wifi* wifiDecode(String wifiString) {
    std::vector<String> wifiData;
    String wifiDataItem = "";
    Serial.println(wifiString);
    for(int i = 0; i < wifiString.length(); i++) {
      if (wifiString[i] == '|') {
        wifiData.push_back(wifiDataItem); 
        wifiDataItem= "";
        continue;
      }

      wifiDataItem += wifiString[i];
    }
    
    return new Wifi(wifiData[0], wifiData[1]);
  }
};

class WifiList {
  private: std::vector<Wifi*> list;

  public: WifiList() {}

  public: WifiList(std::vector<String> wifiList) {
    for(auto wifiItem: wifiList) {
      this->list.push_back(Wifi::wifiDecode(wifiItem));
    }
  }

  public: std::vector<Wifi*> getList() {
    return this->list;
  }

  public: WifiList* addWifi(Wifi* wifi) {
    this->list.push_back(wifi);
    return this;
  }

  public: static std::vector<String> wifiListEncode(WifiList* wifiList) {
    std::vector<String> wifiEncodedData;

    for(auto wifiItem: wifiList->getList()) {
      wifiEncodedData.push_back(Wifi::wifiEncode(wifiItem));
    }

    return wifiEncodedData;
  }

  public: void connect(String api_key) {
    for(auto wifiItem: this->list) {
      WiFi.begin(wifiItem->getSsid().c_str(), wifiItem->getPassword().c_str());
      Serial.println("API key - " + api_key);
      Serial.println("SSID - " + wifiItem->getSsid());
      Serial.println("PASSWORD - " + wifiItem->getPassword());
       for (int j = 0; j < 10; j++) {
         if (WiFi.status() != WL_CONNECTED) {
           delay(500);
           Serial.print(".");
         } else {
           Serial.println("done");
           Serial.print("IP address: ");
           Serial.println(WiFi.localIP());
           Blynk.begin(api_key.c_str(), wifiItem->getSsid().c_str(), wifiItem->getPassword().c_str());
         }
       }
    }
  }
};

class Configuration {
  private: std::map<String, String> list;

  public: Configuration() {}

  public: Configuration(std::vector<String> configuratinEncodedData) {
    std::vector<String> configurationData;
    String configurationItem = "";

    for(auto configuratinEncodedItem: configuratinEncodedData) {
      for(int i = 0; i < configuratinEncodedItem.length(); i++) {
        if (configuratinEncodedItem[i] == '|') {
          configurationData.push_back(configurationItem); 
          configurationItem= "";
          continue;
        }
  
        configurationItem += configuratinEncodedItem[i];
      }

      this->list[configurationData[0]] = configurationData[1];
    }
  }

  public: Configuration* addConfiguration(String key, String val) {
    this->list[key] = val;
    return this;
  }

  public: String getConfiguration(String key) {
    return this->list[key];
  }

  public: std::map<String, String> getList() {
    return this->list;
  }

  public: static std::vector<String> configuartionEncode(Configuration* configuration) {
    std::vector<String> configurationEncodedData;

    for(auto& it : configuration->getList()) {
        configurationEncodedData.push_back(it.first + "|" + it.second + "|");
    }

    return configurationEncodedData;
  }
};

class ServerManager {
  public: static void updateStatus(String host, String api_key, String deviceStatus) {
    WiFiClient client;

    Serial.println("Connect to " + host);

    if (client.connect(host.c_str(), 80)) {
       Serial.println("send request");
      Serial.print("GET /device/status/");
      Serial.print(api_key);
      Serial.print("/");
      Serial.print(deviceStatus);
      
      Serial.println(" HTTP/1.1");
      Serial.print("Host:");
      Serial.println(host);
      Serial.println("Connection: close");
      Serial.println();
      Serial.println();
      
      client.print("GET /device/status/");
      client.print(api_key);
      client.print("/");
      client.print(deviceStatus);
      
      client.println(" HTTP/1.1");
      client.print("Host:");
      client.println(host);
      client.println("Connection: close");
      client.println();
      client.println();
    }
  }

  public: static void updateLocation(String host, String api_key, double latitude, double longitude) {
    WiFiClient client;

    Serial.println("Connect to " + host);

    if (client.connect(host.c_str(), 80)) {
      Serial.println("send request");
      Serial.print("GET /device/location/");
      Serial.print(api_key);
      Serial.print("/");
      Serial.print(latitude, 6);
      Serial.print("/");
      Serial.print(longitude, 6);
      
      Serial.println(" HTTP/1.1");
      Serial.print("Host:");
      Serial.println(host);
      Serial.println("Connection: close");
      Serial.println();
      Serial.println();
  
      client.print("GET /device/location/");
      client.print(api_key);
      client.print("/");
      client.print(latitude, 6);
      client.print("/");
      client.print(longitude, 6);
      
      client.println(" HTTP/1.1");
      client.print("Host:");
      client.println(host);
      client.println("Connection: close");
      client.println();
      client.println();
    }
  }
};

WifiList* wifiList;
Configuration* configuration;

class GPS {
  public: static TinyGPSPlus gps;
  public: static const int RXPin = 16;
  public: static const int TXPin = 17;
  public: static const uint32_t GPSBaud = 9600;
  
  public: static bool checkGPS(){
    if (gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      return true;
    }
  
    return false;
  }

  public: static void displayInfo() {
    WiFiClient client;
    if (GPS::checkGPS()) return;
    
    if (GPS::gps.location.isValid()) {
      ServerManager::updateLocation(
        "esp.vfbsac.by", 
        configuration->getConfiguration("api_key"), 
        GPS::gps.location.lat(), 
        GPS::gps.location.lng());
    }
    
    delay(10000);
  }
};

SoftwareSerial gps_module(GPS::RXPin, GPS::TXPin);
 
void setup()
{
  Serial.begin(9600);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Spiffs* wifiFile = new Spiffs("/wifi.txt", FILE_READ);
  Spiffs* configFile = new Spiffs("/config.txt", FILE_READ);
  wifiList = new WifiList(wifiFile->getData());
  configuration = new Configuration(configFile->getData());  
  wifiFile->close();
  configFile->close();
  
  gps_module.begin(GPS::GPSBaud);
}

void loop()
{
  if (gps_module.available() > 0) 
  {
    //displays information every time a new sentence is correctly encoded.
    if (GPS::gps.encode(gps_module.read()))
    displayInfo();
  }
}

void displayInfo() {
  if (GPS::checkGPS()) return;

  if(Serial.available() > 0) {                  
      char cmd = char(Serial.read());
      
      if (cmd == 'u') {
        // update Data
      } else if (GPS::gps.location.isValid()) {
        GPS::displayInfo();
        delay(10000);       
      }
  }
}
