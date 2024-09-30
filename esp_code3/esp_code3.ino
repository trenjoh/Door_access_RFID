#include <WiFi.h>
#include <HTTPClient.h> // Use HTTPClient instead of HttpClient
#include <ArduinoJson.h>
// Declare the pins used for Serial communication on ESP32
#define txd1 17
#define rxd1 16

// Credentials for WiFi network
const char* ssid = "CDED";
const char* password = "CDED2024.";
// const char* ssid = "dekut";
// const char* password = "dekut@ict2023?";

const char* getResponsecode = "https://clocking-system-uguj.onrender.com/";
const char* serverName = "https://clocking-system-uguj.onrender.com/api/clocking/";
const char* sendRegNo = "https://clocking-system-uguj.onrender.com/api-docs/#/default/post_api_clocking_register"; 

String httpRequestData; 
const char* cardIdValue;
const char* cardIdAction;
String response;
String RegNo;
int PostCode;
int httpResponseCode;
bool isPresent;
String cardIdEnrollment;
void setup() {
  Serial.begin(115200); // To allow use of Serial monitor
  Serial2.begin(115200, SERIAL_8N1); // Use of multiple Serial, for communication between the two microcontrollers
  WiFi.disconnect();  // Disconnects wifi from previous network
  WiFi.begin(ssid, password); // To connect ESP32 to the WiFi Network
  WiFi.mode(WIFI_STA);
  Serial.println("Connecting to wifi");

  // Buffer, waiting for wifi to connect
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to a wifi network");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Fetches then shows the IP address of the wifi network
}

void loop() {
  if (Serial2.available()) { // Ensures data can be sent from the other microcontroller
    httpRequestData = Serial2.readString(); // Reads the data (cardId) from the Pro Mini uc
    delay(1000);
    isPresent = false;
    StaticJsonDocument<192> docobj; // Creates a json object in memory
    DeserializationError error = deserializeJson(docobj, httpRequestData); // Fetches the data from the memory, if successful
    cardIdValue = docobj["cardId"]; // Stores the fetched data in the var as a String - array of char
    cardIdAction = docobj["action"];
    cardIdEnrollment = docobj["attemptedClocking"].as<String>(); //"{\"cardId\": \"" +scannedTag+ "\",\"action\": \"" +action+ "\",\"attemptedClocking\": \"" +isEnrolled+ "\"}";
    Serial.print("Checking UID : ");
    Serial.println(cardIdValue);
    Serial.print("Checking User action : ");
    Serial.println(cardIdAction);
    //Serial.print("Enrolled : [ 0 for yes]  :  " );
    Serial.println(cardIdEnrollment);
    checkstudent();
  }
}

void checkstudent() {
  Serial.println("Get Auth");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http1;
    char getStudent[128];
    strcpy(getStudent, serverName);
    strcat(getStudent, cardIdValue);

    http1.begin(getStudent); 
    httpResponseCode = http1.GET();
    Serial.print("Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String response = http1.getString(); 

      DynamicJsonDocument doc(256); 
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      isPresent = doc["message"].as<bool>(); 
      Serial.println(isPresent);
      Serial.println(isPresent);
      if (isPresent && cardIdEnrollment == "0") { 
        Serial2.println("1");
        Serial.println("ACCESS GRANTED");
      } else {
        Serial2.println("2");
        Serial.println("ACCESS DENIED");
      }
      sendClockRequest(cardIdValue, cardIdAction);
    } else {
      if (cardIdEnrollment == "0"){
      Serial2.println("2");
      Serial.println("ACCESS DENIED");
      }
      sendClockRequest(cardIdValue, cardIdAction);
    }
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void sendClockRequest(String cardId, String action) {
     
    String toggleClock = "https://clocking-system-uguj.onrender.com/api/clocking/";
    toggleClock += cardId;
    toggleClock += "/toggle-clock";

    Serial.println(toggleClock);

    HTTPClient hClient;
    hClient.begin(serverName);

    int httpResponseCode = hClient.POST(toggleClock);
    if (httpResponseCode > 0) {
      Serial.println("--Clocked--");
    }
    else {
      Serial.println("--failed clock--");
    }

    hClient.end();
}
