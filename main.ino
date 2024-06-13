#include <OLED_I2C.h>
#include "pitches.h"
#include <SoftwareSerial.h>

SoftwareSerial ESP8266(2, 3); // RX, TX

#define BUTTON_PIN 7  // Butonun bağlı olduğu pin

int redLed = 9;
int greenLed = 8;
int signalpin = 5;
int voicePin = 6;
bool metal = false;

OLED myOLED(SDA, SCL, 8);


extern uint8_t SmallFont[];
extern uint8_t no_metal_logo [];
extern uint8_t yes_metal_logo [];


//***************WiFi Bağlantı Noktaları********************
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
//***************WiFi Bağlantı Noktaları********************


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


//***************SES KODU********************
// Melody 1 (metal detected)
int melody1[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations1[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Melody 2 (metal is not detected)
int melody2[] = {
  NOTE_E4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4, 0
};
int noteDurations2[] = {
  4, 4, 4, 4, 4, 4, 4, 4
};

void playMelody(int melody[], int noteDurations[], int length) {
  for (int thisNote = 0; thisNote < length; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(voicePin, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(voicePin);
  }
}
//*************************************************


void setup() {
  Serial.begin(115200);
  ESP8266.begin(115200);  
  ESP8266.println("AT+RST");
  ESP8266.println("AT+CWMODE=1");  // Station mode

  pinMode(signalpin, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Buton pini için pull-up direnci ayarla
  myOLED.begin();
  myOLED.setFont(SmallFont); 
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
}

void loop() {

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);

  loading();
  myOLED.clrScr();
  
  int signalValue= digitalRead(signalpin);
  if (signalValue == LOW){
    metal = true;
  }
  else {
    metal = false;
  }

  if(signalValue==LOW){
    logo_yes_metal();
  }
  else{
    logo_no_metal();
  } 

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

    if (metal) {
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
void loading() {
  myOLED.clrScr();
  myOLED.drawRoundRect(20, 20, 100, 40);

  if (digitalRead(BUTTON_PIN) == HIGH) {
    myOLED.print("YUKLENIYOR", 28, 45); // Türkçe yükleme mesajı
  } else {
    myOLED.print("LOADING", 40, 45); // İngilizce yükleme mesajı
  }

  for (int i = 21; i <= 99; i++) {
    myOLED.drawLine(i, 20, i, 40);
    myOLED.update();
    delay(10);
  }
}

void logo_no_metal() {
  myOLED.drawBitmap(20, 0, no_metal_logo, 96, 64);
  myOLED.update();
  for (int i = 128; i >= 2; i--) {
    if (digitalRead(BUTTON_PIN) == HIGH) {
      digitalWrite(redLed, HIGH);
      myOLED.print(" Metal Bulunmamistir  ", i, 57);
      if (i == 127) {
          playMelody(melody2, noteDurations2, 8);
      }  
       // Türkçe mesaj
    } else {
      digitalWrite(redLed, HIGH);
      myOLED.print("Metal is not detected ", i, 57); // İngilizce mesaj
      if (i == 127) {
          playMelody(melody2, noteDurations2, 8);

      }  
    }
    delay(5);
    myOLED.update();
  }
  myOLED.clrScr();
}

void logo_yes_metal() {
  myOLED.drawBitmap(20, 0, yes_metal_logo, 96, 64);
  myOLED.update();
  for (int i = 128; i >= 15; i--) {
    if (digitalRead(BUTTON_PIN) == HIGH) { // Türkçe mesaj
      digitalWrite(greenLed, HIGH);
      myOLED.print("Metal Bulunmustur  ", i, 57);
      if (i == 127) {
          playMelody(melody1, noteDurations1, 8);
      }
      //EMAİL KODU GELECEK(TÜRKÇE)
    } else {  // İngilizce mesaj
      digitalWrite(greenLed, HIGH);
      myOLED.print("Metal is detected  ", i, 57);
      if (i == 127) {
          playMelody(melody1, noteDurations1, 8);
      }
      //EMAİL KODU GELECEK(İNGİLİZCE)
    }
    delay(5);
    myOLED.update();
  }
  myOLED.clrScr();
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
