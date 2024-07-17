#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(2); // Using UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

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


