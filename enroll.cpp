#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
<<<<<<< HEAD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "VAWZEN"
#define WIFI_PASSWORD "Maya552002.L"

// Google Script Deployment URL
const char* serverName = "https://script.google.com/macros/s/AKfycbyrRYU9A_ZOHxYP4TTFYwxgCZDdB-YzGE5CNkdSo3ezCFPifBjo3Uxa30WAgC-GJk4OXQ/exec";

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
//int sensor=A0 
=======
>>>>>>> parent of b6889b5 (button logic)

// Firebase project API Key and URL
#define API_KEY "AIzaSyCgEAfzqgCaiuzsJrATYTAgy4ZVh6V3j1w"
#define DATABASE_URL "https://fingerprints-118e0-default-rtdb.firebaseio.com/" 

//Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

HardwareSerial mySerial(2); // Use UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
<<<<<<< HEAD
#define BUTTON_ENROLL_PIN 14
#define BUTTON_ATTENDANCE_PIN 12
#define BUTTON_DELETE_PIN 13  
#define BAZ 15
const int buzzerPin = 25;
const int buzzerFrequency = 1000;
const int buzzerDuration = 500;
const int led1 = 26;
const int led2 = 27;

#define Tone1 880
#define Tone2 1047
#define Tone3 1319

#define toneDuration 150
#define pause 50
/Function declarations
void delete_fingerprint();
void enrollNewFingerprint();
void captureAttendance();
//void ISR();

int mode=1; //initialise to attendance mode

String firstName;
String lastName;

void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_ENROLL_PIN), loop, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_ATTENDANCE_PIN), loop, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_DELETE_PIN), loop, FALLING);

  lcd.init();         // initialize the lcd
  lcd.backlight();    // Turn on the LCD screen backlight

  pinMode(BUTTON_ENROLL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ATTENDANCE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DELETE_PIN, INPUT_PULLUP);


  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX = 16, TX = 17

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0,0);
    lcd.print("Sensor Ready");
    delay(2000);
    lcd.clear();

  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.setCursor(1,0);
    lcd.print("Sensor error");
    while (1) { delay(1); }
  }
}
//for deleteing a fingerprint
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  //ENROLL
  if (digitalRead(BUTTON_ENROLL_PIN) == LOW) {
    mode = 0;  // Switch to Enrollment Mode
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE 0");//THIS NOT SEEN
    delay(2000);
    Serial.println("Mode: Enrollment");
    delay(50); // Debounce delay

    lcd.setCursor(0,0);
    lcd.print("In Enroll Mode");
    delay(2000);
    lcd.clear();
  }
  //ATTENDANCE
  if (digitalRead(BUTTON_ATTENDANCE_PIN) == LOW) {
    mode = 1;  // Switch to Attendance Mode
    Serial.println("Mode: Attendance");
    delay(50); // Debounce delay

    lcd.setCursor(2,0);
    lcd.print("In Attend Mode");
    delay(2000);
    lcd.clear();
  }
  //DELETE
  if (digitalRead(BUTTON_DELETE_PIN) == LOW) {
    mode = 2;  // Switch to Attendance Mode
    Serial.println("Mode: Attendance");
    delay(50); // Debounce delay

    lcd.setCursor(0,0);
    lcd.print("In Delete Mode");
    delay(2000);
    lcd.clear();
  }
  // Depending on mode, execute corresponding functionality
  if (mode == 0) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE 0");
    delay(2000);
    enrollNewFingerprint();
  } else if (mode == 1) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE 1");
    delay(2000);
    captureAttendance();
  }else if (mode == 2) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE 2");
    delay(2000);
    delete_fingerprint();
  }
  delay(200);
}  

void enrollNewFingerprint(){

  lcd.setCursor(0,0);
  lcd.print("In Enroll Mode");
  delay(2000);
  lcd.clear();

  Serial.println("Ready to enroll a fingerprint!");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter id");
  lcd.setCursor(0,1);
  lcd.print("and name");
  //delay(5000);
  //lcd.clear();





  Serial.println("Enter name and ID"); //# (from 1 to 127) you want to save this finger as...");
  while (!Serial.available());
  int id = Serial.parseInt();
  if (id == 0) { // ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  Serial.println("Please enter the first name:");
  while (!Serial.available());
  firstName = Serial.readStringUntil('\n');
  firstName.trim();

  Serial.println("Please enter the last name:");
  while (!Serial.available());
  lastName = Serial.readStringUntil('\n');
  lastName.trim();

  Serial.print("Enrolling fingerprint for ");
  Serial.print(firstName);
  Serial.print(" ");
  Serial.println(lastName);

  while (!getFingerprintEnroll(id));
}

uint8_t getFingerprintEnroll(uint8_t id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  lcd.setCursor(1,0);
  lcd.clear();
  lcd.print("Place finger");
  Serial.println(id);


  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  lcd.clear();
  Serial.println("Remove finger");
  lcd.setCursor(2,0);
  lcd.print("Remove finger");
  //delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Place f again");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Saved!!");
    delay(2000);

    Serial.print("Fingerprint enrolled for ");
    Serial.print(firstName);
    Serial.print(" ");
    Serial.println(lastName);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  mode=1;

  return true;
}

  




void captureAttendance(){
  String FIRSTNAME;
  String LASTNAME;
  String SCLASS;
  // Attempt to read a fingerprint
  Serial.println("Place your finger on the sensor...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Place finger");

  while (finger.getImage() != FINGERPRINT_OK);
  
  // Convert image to template
  if (finger.image2Tz() != FINGERPRINT_OK) {
    Serial.println("Error converting image");
    return;
  }

  // Search for fingerprint in the database
  if (finger.fingerSearch() != FINGERPRINT_OK) {
    Serial.println("Fingerprint not recognized");
    digitalWrite(led2, HIGH);
    delay(500);
    digitalWrite(led2, LOW);
    tone(buzzerPin, buzzerFrequency, buzzerDuration);
    lcd.setCursor(0,0);
    lcd.print("Not Recognized");
    return;
  }
  ID=finger.fingerID;
  
  // Print ID of recognized fingerprint
  Serial.print("Fingerprint recognized! ID: ");
  Serial.println(ID);
  digitalWrite(led2, LOW);
  playTriTone();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ATTENDANCE");
  lcd.setCursor(0,1);
  lcd.print("RECORED");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("For ID: ");
  lcd.print(finger.fingerID);
  delay(5000);
  if (Firebase.ready() && signupOK){
    if (Firebase.RTDB.getString(&fbdo, "/" + String(ID) + "/fname")) {
        if (fbdo.dataType() == "string") {
          FIRSTNAME = fbdo.stringData();
          Serial.println(FIRSTNAME);
        }
    }
    else {
      Serial.println(fbdo.errorReason());
      }
    if (Firebase.RTDB.getString(&fbdo, "/" + String(ID) + "/lname")) {
        if (fbdo.dataType() == "string") {
          LASTNAME = fbdo.stringData();
          Serial.println(LASTNAME);
        }
    }
    else {
      Serial.println(fbdo.errorReason());
      }

    if (Firebase.RTDB.getString(&fbdo, "/" + String(ID) + "/class")) {
        if (fbdo.dataType() == "string") {
          SCLASS = fbdo.stringData();
          Serial.println(SCLASS);
        }
    }
    else {
      Serial.println(fbdo.errorReason());
      }    
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    String postData = "ID=";
    postData += ID;
    postData += "&FIRSTNAME=";
    postData += FIRSTNAME;
    postData += "&LASTNAME=";
    postData += LASTNAME;
    postData += "&CLASS=";
    postData += SCLASS;

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);
    
    if (httpResponseCode == 302) { // Check for redirect response
    String newLocation = http.header("Location");
    Serial.println("Redirected to: " + newLocation);
  } else if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    }

    http.end();
  }


}
void playTriTone() {
  tone(buzzerPin, Tone1, toneDuration);
  delay(toneDuration + pause);
  tone(buzzerPin, Tone2, toneDuration);
  delay(toneDuration + pause);
  tone(buzzerPin, Tone3, toneDuration);
  delay(toneDuration + pause);
}

void delete_fingerprint(){
  Serial.println("DELETE MODE");
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  uint8_t id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }

  Serial.print("Deleting ID #");
  Serial.println(id);

  deleteFingerprint(id);
} 

  uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;

}

