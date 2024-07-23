#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
//int sensor=A0

HardwareSerial mySerial(2); // Using UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define BUTTON_ENROLL_PIN 14
#define BUTTON_ATTENDANCE_PIN 12
#define BAZ 15

//Function declarations
void enrollNewFingerprint();
void captureAttendance();
//void ISR();

int mode=1; //initialise to attendance mode

String firstName;
String lastName;

void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_ENROLL_PIN), loop, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_ATTENDANCE_PIN), loop, FALLING);

  lcd.init();         // initialize the lcd
  lcd.backlight();    // Turn on the LCD screen backlight

  pinMode(BUTTON_ENROLL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ATTENDANCE_PIN, INPUT_PULLUP);


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

  

void loop() {

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
  
  if (digitalRead(BUTTON_ATTENDANCE_PIN) == LOW) {
    mode = 1;  // Switch to Attendance Mode
    Serial.println("Mode: Attendance");
    delay(50); // Debounce delay

    lcd.setCursor(2,0);
    lcd.print("In Attend Mode");
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

void captureAttendance();

String firstName;

String lastName;

void setup() {
  


  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // Receive = 16, Transmit = 17

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
   
  } else {
    Serial.println("Did not find fingerprint sensor :(");
   
    while (1) { delay(1); }
  }
}

void loop() {
  Serial.println("Ready to enroll a fingerprint!");
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

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
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

  return true;
}

void captureAttendance(){
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
    return;
  }

  // Print ID of recognized fingerprint
  Serial.print("Fingerprint recognized! ID: ");
  Serial.println(finger.fingerID);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ATTENDANCE");
  lcd.setCursor(0,1);
  lcd.print("RECORED");
  delay(2000);
  digitalWrite(BAZ,HIGH);


}


