#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D8
#define RST_PIN D0
#define TOTAL_CARDS 3

// Instance of the class
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Init 2D array that will store new NUID's
byte nuidPICC[TOTAL_CARDS][4];

int FCE_PIN = D2;
int TMP_PIN = A0;
int forceReading;
int tempReading;
int count = 0;

// Function declarations
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
int checkCardExists(byte *current);
void printCard(byte *current_card, int size, int num);

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200); // Serial Monitor for FSR readings
  SPI.begin();          // Init SPI bus
  rfid.PCD_Init();      // Init MFRC522
  Serial.println();
  Serial.print("RFID :");
  rfid.PCD_DumpVersionToSerial();

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println();
  Serial.println("This code scan the MIFARE Classic NUID.");
  Serial.print("Using the following key:");
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  
  pinMode(2, OUTPUT); // LED
  pinMode(5, OUTPUT); // Fan
}

void loop()
{
  // put your main code here, to run repeatedly:

  digitalWrite(2, LOW); // Turn LED off
  digitalWrite(5, LOW); // Turn Fan off

  // Reading values from FSR and Temperature Sensor
  forceReading = digitalRead(FCE_PIN);
  tempReading = analogRead(TMP_PIN);

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
    // Logging value from Temp in Serial Monitor
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

  Serial.print("PICC type: ");
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println("Your tag is not of type MIFARE Classic.");
    return;
  }

  int card_number = checkCardExists(rfid.uid.uidByte);

  if (card_number == -1 && count == TOTAL_CARDS)
  {
    Serial.println("Can't register any more books.");
  }
  else if (card_number == -1)
  { 
    Serial.println("A new book has been detected.");

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
        nuidPICC[count][i] = rfid.uid.uidByte[i];
    }
    count++;
    printCard(rfid.uid.uidByte, rfid.uid.size, count);
  }
  else 
  {
    Serial.println("Card read previously.");
    printCard(rfid.uid.uidByte, rfid.uid.size, card_number + 1)
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

/**
 *  Helper function to check if the card swiped already exists.
 */
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

/**
 * Helper function to print the current card being read
 */
void printCard(byte *current_card, int size, int num)
{
  Serial.println("The NUID tag is:");
  Serial.print("In hex: ");
  printHex(current_card, size);
  Serial.println();
  Serial.print("In dec: ");
  printDec(current_card, size);
  Serial.println();
  Serial.print("Book: ");
  Serial.print(num);
  Serial.println();
}
