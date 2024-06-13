#include <SoftwareSerial.h>

SoftwareSerial ESP8266(2, 3); // RX, TX

// Wi-Fi Variables 
unsigned char check_connection = 0;
unsigned char times_check = 0;
const char* ssid = "mahmut";
const char* password = "mahmutenes";

// Server Variables
const char* host = "192.168.163.157";
int port = 3000;
String server = "192.168.163.157";
String path = "/mail";

String jsonData;


// Test Variables
int amplitude = 200; // Örnek amplitude değeri
int phase = 500; // Örnek phase değeri
bool metal= true;

// TimeStamp
unsigned long previousMillis = 0;
const long interval = 1000;  // 1 saniye
int second = 0;
int minute = 0;
int hour = 0;


// Linked-List Definition
struct node {
    String data;
    node* next;
};

// Linked list class definition
class LinkedList {
private:
    node* head;

public:
    // Constructor
    LinkedList() : head(nullptr) {}

    // Destructor to free memory
    ~LinkedList() {
        while (head != nullptr) {
            node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    // Function to add a node to the end
    void append(const String& newData) {
      Serial.println("Append");
        node* newNode = new node();
        newNode->data = newData;
        newNode->next = nullptr;

        if (head == nullptr) {
            head = newNode;
        } else {
            node* temp = head;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    // Function to remove a node from the beginning and return its data
    String removeFirst() {
      Serial.println("Remove");
        if (head == nullptr) {
            Serial.println("The list is empty, no node to remove.");
            return "";
        }

        node* temp = head;
        head = head->next;
        String removedData = temp->data;
        delete temp;
        return removedData;
    }

    // Function to get the data of the first node without removing it
    String getFirst() const {
        if (head == nullptr) {
            Serial.println("The list is empty, no node to return.");
            return "";
        }

        return head->data;
    }

    // Function to count the number of nodes
    int count() const {
        Serial.println("Sayiliyor");
        int count = 0;
        node* temp = head;
        while (temp != nullptr) {
            count++;
            temp = temp->next;
        }
        return count;
    }
};


LinkedList list;

void setup() {
  Serial.begin(115200);
  ESP8266.begin(115200);  

  ESP8266.println("AT+RST");
  delay(2000);
  ESP8266.println("AT+CWMODE=1");  // Station mode
  delay(2000);
}

void loop() {
  if (check_connection == 0) {
    Serial.println("Connecting to Wifi...");
    while (check_connection == 0 && times_check < 5) {
      ESP8266.print("AT+CWJAP=\"");
      ESP8266.print(ssid);
      ESP8266.print("\",\"");
      ESP8266.print(password);
      ESP8266.println("\"");
      ESP8266.setTimeout(10000);
      if (ESP8266.find("OK")) {
        Serial.println("WIFI CONNECTED");
        delay(5000);
        check_connection = 1;
      } else {
        Serial.println("Failed to connect to WiFi");
        times_check++;
        delay(10000);
      }
    }
    if (check_connection == 0) {
      Serial.println("Failed to connect after 5 attempts. Restarting...");
      ESP8266.println("AT+RST");
      delay(15000);
      times_check = 0;
    }
  }

  if (check_connection == 1) {

//    String control = Serial.readString();
//    if(control == "true"){
//      metal = true;
//    }
//    else if(control == "false"){
//      metal = false;
//    }
    
    if (metal) {
      // led
      // ses
      // lcd 
      jsonData = "{\"to\":\"menesscevik@icloud.com\",\"message\":\"Metal tespit edildi. Zaman Bilgisi: \"}";

      list.append(jsonData);
    } else {
      jsonData = "";
    }
    if(list.count() > 0){
      Serial.println("Sending Post Request");
      sendPostRequest();
    }
  }
}

void sendPostRequest() {
   
  String cmd = "AT+CIPSTART=\"TCP\",\"" + server + "\"," + port;
  ESP8266.println(cmd);
  delay(2000);
  
  if (ESP8266.find("OK")) {
    Serial.println("Connection Ready");

    
    String data = list.getFirst();
    String httpRequest = "POST " + path + " HTTP/1.1\r\n";
    httpRequest += "Host: " + server + "\r\n";
    httpRequest += "User-Agent: Arduino/1.0\r\n";
    httpRequest += "Connection: close\r\n";
    httpRequest += "Content-Type: application/json\r\n";
    httpRequest += "Content-Length: " + String(data.length()) + "\r\n";
    httpRequest += "\r\n";
    httpRequest += data;

    cmd = "AT+CIPSEND=" + String(httpRequest.length());
    ESP8266.println(cmd);
    delay(2000);

    if (ESP8266.find(">")) {
      Serial.println("Sending data...");
      ESP8266.print(httpRequest);
      delay(2000);

      if (ESP8266.find("SEND OK")) {
        Serial.println("Data sent successfully");
        String removedData = list.removeFirst();
        String response = "";
        long timeout = millis() + 20000;  // Increase timeout to 20 seconds
        while (millis() < timeout) {
          while (ESP8266.available()) {
            char c = ESP8266.read();
            response += c;
          }
          if (response.indexOf("+IPD") != -1 && response.endsWith("\r\n")) {
            break;
          }
        }

        Serial.println("Response from server:");
        Serial.println(response);

      } else {
        Serial.println("Failed to send data");
      }
    } else {
      Serial.println("Failed to send data");
    }
  } else {
    Serial.println("Failed to establish connection");
  }

  check_connection = 0;
  times_check = 0;
}