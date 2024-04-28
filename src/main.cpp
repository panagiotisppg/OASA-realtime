// #include <Arduino.h>
// #include <WiFi.h>
// #include <DNSServer.h>
// #include <LittleFS.h>
// #include <ESPAsyncWebServer.h>
// #include "wsEventHandler.h"

// #define SSID "ESP32 SoftAP" // This is the SSID that ESP32 will broadcast
// // #define CAPTIVE_DOMAIN "http://domain-name-to-show" // This is the SSID that ESP32 will broadcast
// // Uncomment the following line to enable password in the wifi acces point
// // #define PASSWORD "12345678" // password should be atleast 8 characters to make it work
// #define DNS_PORT 53
// // Options to enable serial printing
// #define VERBOSE

// const IPAddress apIP(192, 168, 2, 1);
// const IPAddress gateway(255, 255, 255, 0);

// DNSServer dnsServer;
// AsyncWebServer server(80);
// AsyncWebSocket websocket("/ws");

// void redirectToIndex(AsyncWebServerRequest *request)
// {
// #ifdef CAPTIVE_DOMAIN
//   request->redirect(CAPTIVE_DOMAIN);
// #else
//   request->redirect("http://" + apIP.toString());
// #endif
// }

// void setup()
// {
//   pinMode(2, OUTPUT);
// #ifdef VERBOSE
//   Serial.begin(115200);
// #endif

//   WiFi.disconnect();   // added to start with the wifi off, avoid crashing
//   WiFi.mode(WIFI_OFF); // added to start with the wifi off, avoid crashing
//   WiFi.mode(WIFI_AP);
// #ifndef PASSWORD
//   WiFi.softAP(SSID);
// #else
//   WiFi.softAP(SSID, PASSWORD);
// #endif
//   WiFi.softAPConfig(apIP, apIP, gateway);
//   dnsServer.start(DNS_PORT, "*", apIP);

// #ifdef VERBOSE
//   Serial.println("\nWiFi AP is now running\nIP address: ");
//   Serial.println(WiFi.softAPIP());
// #endif

//   if (!LittleFS.begin())
//   {
// #ifdef VERBOSE
//     Serial.println("An Error has occurred while mounting LittleFS");
// #endif
//     return;
//   }

//   // bind websocket to async web server
//   websocket.onEvent(wsEventHandler);
//   server.addHandler(&websocket);
//   // setup statuc web server
//   server.serveStatic("/", LittleFS, "/www/")
//       .setDefaultFile("index.html");
//   // Captive portal to keep the client
//   server.onNotFound(redirectToIndex);
//   server.begin();

// #ifdef VERBOSE
//   Serial.println("Server Started");
// #endif
// }

// void loop()
// {
//   // serve DNS request for captive portal
//   dnsServer.processNextRequest();
//   vTaskDelay(1);
// }


#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "NOVA_2.4G_D310";     // Your WiFi SSID
const char* password = "bj7592kKnV";     // Your WiFi password
const char* apiUrl = "https://telematics.oasa.gr/api/?act=getStopArrivals&p1=090009"; // Original API URL
const char* routeUrlBase = "https://telematics.oasa.gr/api/?act=getRouteName&p1=";
const char* routesFromStop = "https://telematics.oasa.gr/api/?act=webRoutesForStop&p1=";
const char* stopCode = "090009";

LiquidCrystal_I2C lcd(0x27, 16, 2);

String jsonData; // is storing the JSON data of all the routes for the selected stop

struct RouteInfo {
  String lineID;
  String lineDescrEng;
};

RouteInfo parseJsonForRouteData(String jsonString, int routeCodeToFind) {
  int startIndex = 0;
  int endIndex;

  RouteInfo routeInfo;

  // Loop through each object in the JSON string
  while ((endIndex = jsonString.indexOf('{', startIndex)) != -1) {
    int routeCodeStart = jsonString.indexOf("\"RouteCode\":", endIndex) + 13;
    int routeCodeEnd = jsonString.indexOf(',', routeCodeStart);
    String routeCodeStr = jsonString.substring(routeCodeStart, routeCodeEnd);
    int routeCode = routeCodeStr.toInt();

    if (routeCode == routeCodeToFind) {
      //get the LineID
      int lineIDStart = jsonString.indexOf("\"LineID\":", endIndex) + 9;
      int lineIDEnd = jsonString.indexOf(',', lineIDStart);
      String lineID = jsonString.substring(lineIDStart+1, lineIDEnd-1);
      //get the route Name
      int lineDescrEngStart = jsonString.indexOf("\"LineDescrEng\":", endIndex) + 16;
      int lineDescrEngEnd = jsonString.indexOf('"', lineDescrEngStart + 1); // Find closing quote that's not escaped
      String lineDescrEng = jsonString.substring(lineDescrEngStart, lineDescrEngEnd);

      routeInfo.lineID = lineID;
      routeInfo.lineDescrEng = lineDescrEng;
      break;
    }

    startIndex = endIndex + 1; // Move to the next object after the closing brackets
  }

  return routeInfo;
}


String fetchJsonData(const char* baseUrl, const char* stopCode) {
  String url = String(baseUrl) + stopCode;
  
  HTTPClient http;

  if (http.begin(url)) {
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();
      return payload;
    } else {
      Serial.printf("HTTP request failed with error code: %d\n", httpCode);
    }
  } else {
    Serial.println("Failed to connect to API");
  }

  return "";
}





void setup() {
  Wire.begin(5, 6);
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);

  Serial.begin(115200);
  delay(100);

  // Connect to Wi-Fi
  Serial.println();
  lcd.print("Connecting to Wifi...");
  Serial.print("Connecting to WiFi.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi connected]");

  // Print the IP address :)
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Fetch the JSON data for the routes of the stop
  jsonData = fetchJsonData(routesFromStop, stopCode);
}


void parseAndPrintJson(String jsonString) { 
  //Get and print all the data 
  //Beautifully (subjective) to the terminal
  //And ugly in the LCD (working on it)
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  int printed_buses = 0;
  lcd.clear();
  

  for (JsonObject obj : arr) {
    const char* route_code = obj["route_code"];
    const char* routeName = "? - ?";
    const char* veh_code = obj["veh_code"];
    const char* btime2 = obj["btime2"];
    const char* real_bus = "?";
    const char* tabs_after_route = "\t|\t\t";

    // Make request to fetch route name and id
    String routeUrl = String(routeUrlBase) + route_code;
    RouteInfo routeIDName = parseJsonForRouteData(jsonData, atoi(route_code));

    if (routeIDName.lineID.length() > 0) {
      real_bus = (routeIDName.lineID).c_str();
      routeName = (routeIDName.lineDescrEng).c_str();
    } else {
      Serial.println("Route code not found in the provided JSON data.");
    }

    Serial.print("\nBus: ");
    Serial.print(real_bus);

    //Stuff from rduino code but not many buses pass rn so cant check to fine tune
    if (printed_buses < 2) {
      lcd.setCursor(0, printed_buses);
  
      lcd.print(real_bus);
      if (strlen(real_bus) == 1){
        lcd.print("  ");
      }
      if (strlen(real_bus) == 2){
        lcd.print(" ");
      }
      lcd.print("    ");
      if (strlen(btime2) == 1){
        lcd.print(" ");
      }
      lcd.print(btime2);
      lcd.print("'");
      printed_buses++;
    }

    //more info for debug 
    Serial.print("  \t|\t Time to Arrival: ");
    Serial.print(btime2);
    Serial.print(" minutes");
    Serial.print("\t|\tRoute: '");
    Serial.print(routeName);
    Serial.print("'");
    Serial.print(tabs_after_route);
    Serial.print("  Vehicle Code: ");
    Serial.print(veh_code);
    Serial.print("  \t|\tRoute Code: ");
    Serial.print(route_code);
  }
  if (printed_buses == 0){
    lcd.setCursor(0, 0);
    lcd.print("No bus passes");
    lcd.setCursor(0, 1);
    lcd.print("from your stop");
  }
  Serial.println("\nNext request in 15 seconds.");
  printed_buses = 0;
}




void loop() {
  // Check if connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Make the HTTP request
    Serial.println("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\nMaking the request...");
    if (http.begin(apiUrl)) {
      int httpCode = http.GET();
      
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println("Routes available from your stop:");
          parseAndPrintJson(payload);
        } else {
          Serial.printf("HTTP request failed with error code: %d [Failed to get the list of buses]", httpCode);
        }
      } else {
        Serial.println("HTTP request failed. [Failed to get the list of buses]");
      }

      // End the request
      http.end();
    } else {
      Serial.println("Failed to connect to API.");
    }

    // Wait for 15 seconds before making the next request
    delay(15000);
  } else {
    Serial.println("WiFi not connected. Reconnecting (5 seconds)...");
    WiFi.reconnect();
    delay(5000);
  }
}