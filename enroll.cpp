#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define RXD1 4  // GPIO 4 as RX
#define TXD1 5  // GPIO 5 as TX
#define RXD2 16
#define TXD2 17
// Initialize Serial2 for SIM800L
HardwareSerial sim800l(1);

// Google Script Deployment URL
const char* serverName = "https://script.google.com/macros/s/AKfycbzQLnAIK39-5gb4cdamtLdgRMn93YslRZUH9RTq6ugwKC38Rgl-BvRY3uCd4dh3pHjPVg/exec";

// Insert Firebase project API Key
#define API_KEY "AIzaSyCgEAfzqgCaiuzsJrATYTAgy4ZVh6V3j1w"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://fingerprints-118e0-default-rtdb.firebaseio.com/" 

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

HardwareSerial mySerial(2); // Use UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define RESET_BUTTON 14

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

// Function declarations
void delete_fingerprint();
void enrollNewFingerprint();
void captureAttendance();
void playTriTone();
void resetESP();

int ID;

void setup() {
    Serial.begin(115200);
    // Request Wi-Fi credentials from user

    
    lcd.init();         // Initialize the LCD
    lcd.backlight();    // Turn on the LCD screen backlight

    Serial.println("Enter Wi-Fi SSID:");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER WIFI NAME");
    while (!Serial.available());
    String WIFI_SSID = Serial.readStringUntil('\n');
    WIFI_SSID.trim();
    
    Serial.println("Enter Wi-Fi Password:");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER PASSWORD");
    while (!Serial.available());
    String WIFI_PASSWORD = Serial.readStringUntil('\n');
    WIFI_PASSWORD.trim();

    attachInterrupt(digitalPinToInterrupt(RESET_BUTTON), loop, FALLING);
    pinMode(RESET_BUTTON, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);

    playTriTone();

    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CONNECTING......");
    digitalWrite(led1, LOW);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    digitalWrite(led1, HIGH);

    // Assign the API key (required)
    config.api_key = API_KEY;

    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;

    // Sign up
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("ok");
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2); // RX = 16, TX = 17

    // Initializing the fingerprint sensor
    if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
        lcd.setCursor(0, 0);
        lcd.print("Sensor Ready");
        delay(2000);
        lcd.clear();
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        lcd.setCursor(1, 0);
        lcd.print("Sensor error");
        while (1) { delay(1); }
    }
    sim800l.begin(9600, SERIAL_8N1, RXD1, TXD1);
    sim800l.println("AT");
    delay(1000);
    Serial.println("Initializing GSM module...");
    delay(1000);
    // sim800l.println("AT+CMGF=1");  // Set SMS to text mode
    // delay(1000);
}

// For deleting a fingerprint
uint8_t readnumber(void) {
    uint8_t num = 0;

    while (num == 0) {
        while (!Serial.available());
        num = Serial.parseInt();
    }
    return num;
}

// Enter the loop
void loop() { 
   if (digitalRead(RESET_BUTTON) == LOW) {
    resetESP();
    delay(50);
   }
  int mode; 
 
    Serial.println("Enter mode:\n0 - Enrollment\n1 - Attendance\n2 - Delete");
    while (!Serial.available());
    mode = Serial.parseInt();
    Serial.readStringUntil('\n');
    if (mode == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ENROLLMENT MODE");
        delay(2000);
        enrollNewFingerprint();
    } else if (mode == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ATTENDANCE MODE");
        delay(2000);
        captureAttendance();
    } else if (mode == 2) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DELETE MODE");
        delay(2000);
        delete_fingerprint();
    }
    delay(2000);
}  

void enrollNewFingerprint() {
    String fname, lname, sclass, pcontact, gender;

    if (Firebase.ready() && signupOK) {
        Serial.println("Enter ID");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter ID");
        while (!Serial.available());
        ID = Serial.parseInt();
        Serial.readStringUntil('\n');

        //save fingerprint before uploading details
        while (!getFingerprintEnroll(ID));

        Serial.println("Enter fname");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter First Name");
        while (!Serial.available());
        fname = Serial.readStringUntil('\n');
        fname.trim();

        if (Firebase.RTDB.setString(&fbdo, "/" + String(ID) + "/fname", String(fname))) {
            Serial.println("PASSED");
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        Serial.println("Enter lname");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Last Name");
        while (!Serial.available());
        lname = Serial.readStringUntil('\n');
        lname.trim();

        if (Firebase.RTDB.setString(&fbdo, "/" + String(ID) + "/lname", String(lname))) {
            Serial.println("PASSED");
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        Serial.println("Enter class");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Class");
        while (!Serial.available());
        sclass = Serial.readStringUntil('\n');
        sclass.trim();

        if (Firebase.RTDB.setString(&fbdo, "/" + String(ID) + "/class", String(sclass))) {
            Serial.println("PASSED");
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        Serial.println("Enter parent's contact");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Parent's");
        lcd.setCursor(0, 1);
        lcd.print("Contact");
        while (!Serial.available());
        pcontact = Serial.readStringUntil('\n');
        pcontact.trim();

        if (Firebase.RTDB.setString(&fbdo, "/" + String(ID) + "/Pcontact", String(pcontact))) {
            Serial.println("PASSED");
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        Serial.println("Enter gender");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Gender");
        while (!Serial.available());
        gender = Serial.readStringUntil('\n');
        gender.trim();

        if (Firebase.RTDB.setString(&fbdo, "/" + String(ID) + "/gender", String(gender))) {
            Serial.println("PASSED");
        } else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
        }  
    }
}

uint8_t getFingerprintEnroll(uint8_t ID) {
    int p = -1;
    Serial.print("Waiting for finger to enroll as #");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place finger");
    Serial.println(ID);

    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image taken");
                break;
            case FINGERPRINT_NOFINGER:
                Serial.println(".");
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

    Serial.println("Remove finger");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Remove finger");

    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }

    Serial.print("ID ");
    Serial.println(ID);
    p = -1;
    Serial.println("Place same finger again");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place finger");
    lcd.setCursor(0, 1);
    lcd.print("again");

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

    Serial.print("Creating model for #");
    Serial.println(ID);

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

    Serial.print("ID "); Serial.println(ID);
    p = finger.storeModel(ID);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Stored");
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
    return true;
}

void captureAttendance(){
  String FIRSTNAME;
  String LASTNAME;
  String SCLASS;
  String PCONTACT;
  String GENDER;
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Not Recodnised");
    Serial.println("Fingerprint not recognized");
    digitalWrite(led2, HIGH);
    delay(500);
    digitalWrite(led2, LOW);
    tone(buzzerPin, buzzerFrequency, buzzerDuration);
    return;
  } 
  ID=finger.fingerID;

  // Print ID of recognized fingerprint
    Serial.println("Fingerprint recognized! ID: ");
    playTriTone();
    Serial.println(ID);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ATTENDANCE");
    lcd.setCursor(0,1);
    lcd.print("RECORED FOR: ");
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
    if (Firebase.RTDB.getString(&fbdo, "/" + String(ID) + "/gender")) {
        if (fbdo.dataType() == "string") {
          GENDER = fbdo.stringData();
          Serial.println(GENDER);
        }
    }
    else {
      Serial.println(fbdo.errorReason());
      }    
    if (Firebase.RTDB.getString(&fbdo, "/" + String(ID) + "/Pcontact")) {
        if (fbdo.dataType() == "string") {
          PCONTACT = fbdo.stringData();
          Serial.println(PCONTACT);
        }
    }
    else {
      Serial.println(fbdo.errorReason());
      }           
  }

    String smsMessage = "Dear Parent, your child " + FIRSTNAME + " " + LASTNAME + " from class " + SCLASS + " has just checked in.";
    sendSMS(PCONTACT, smsMessage);


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
    postData += "&GENDER=";
    postData += GENDER;

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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter ID to");
  lcd.setCursor(0, 1);
  lcd.print("Delete");
  Serial.println("Please type in the ID you want to delete...");
  uint8_t id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }

  Serial.print("Deleting ID #");
  Serial.println(id);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Deleting ID :");
  lcd.print(id);

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

void resetESP() {
  ESP.restart();  // Reset the ESP32
}

void sendSMS(String PCONTACT, String message) {
   sim800l.println("AT+CMGF=1"); // Set SMS to text mode
   delay(1000);
   sim800l.println("AT+CMGS=\"" + PCONTACT + "\"");  // Send SMS command
   delay(1000);
   sim800l.println(message);  // The SMS content
   delay(1000);
   sim800l.println((char)26);  // End AT command with a ^Z, ASCII code 26
   delay(3000);
   Serial.println("SMS sent successfully!");
}
