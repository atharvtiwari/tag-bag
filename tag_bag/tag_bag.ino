#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D8
#define RST_PIN D0
#define TOTAL_CARDS 3

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[TOTAL_CARDS][4];

int forcePin = D2;
int forceReading;
int tempPin = A0;
int tempReading;
int count = 0;

void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
int checkCardExists(byte *current);

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200); // Serial Monitor for FSR readings
  SPI.begin();          // Init SPI bus
  rfid.PCD_Init();      // Init MFRC522
  Serial.println();
  Serial.print(F("RFID :"));
  rfid.PCD_DumpVersionToSerial();

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println();
  Serial.println(F("This code scan the MIFARE Classic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  
  pinMode(2, OUTPUT); // LED
  pinMode(5, OUTPUT); // Fan
}

void loop()
{
  // put your main code here, to run repeatedly:

  digitalWrite(2, LOW); // Turn LED off
  digitalWrite(5, LOW); // Turn Fan off

  // Reading value from FSR
  forceReading = digitalRead(forcePin);
  tempReading = analogRead(tempPin);

  if (forceReading)
  {
    // Logging value from FSR in Serial Monitor
    Serial.print("Force: ");
    Serial.println(forceReading);
    digitalWrite(2, HIGH); // Turn LED on
    delay(100);
  }
  if (tempReading > 300)
  {
    Serial.print("Temperature: ");
    Serial.println(tempReading / 3.1);
    digitalWrite(5, HIGH); // Turn Fan on
    delay(100);
  }
  else
  {
    digitalWrite(2, LOW); // Turn LED off
    digitalWrite(5, LOW); // Turn Fan off
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  int card_number = checkCardExists(rfid.uid.uidByte);

  if (card_number == -1 && count == TOTAL_CARDS)
  {
    Serial.println(F("Can't register any more books"));
  }
  else if (card_number == -1)
  { 
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
        nuidPICC[count][i] = rfid.uid.uidByte[i];
    }
    count++;
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("Book: "));
    Serial.print(count);
    Serial.println();
  }
  else 
  {
    Serial.println(F("Card read previously."));
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("Book: "));
    Serial.print(card_number + 1);
    Serial.println();
  }
  
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  delay(1000);
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

int checkCardExists(byte *current)
{
  for (int i = 0; i < TOTAL_CARDS; i++)
  {
    if (current[0] == nuidPICC[i][0] &&
        current[1] == nuidPICC[i][1] &&
        current[2] == nuidPICC[i][2] &&
        current[3] == nuidPICC[i][3])
    {
      return i;
    }
  }
  return -1;
}
