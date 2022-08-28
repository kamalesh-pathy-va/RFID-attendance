#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN 21    //ESP32
#define RST_PIN 22   //ESP32
#define SIZE_BUFFER 18
#define MAX_SIZE_BLOCK 16

const char* ssid     = "ssid";
const char* password = "password";

//Google Sheet Script
String url = "scripturl";

//Dummy values
int dummy = 10;
String val = "hello";

//used in authentication
MFRC522::MIFARE_Key key;

//authentication return status code
MFRC522::StatusCode status;

// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin();
  delay(1000);

  // Init MFRC522
  Serial.println("Init MFRC522");
  mfrc522.PCD_Init();

  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" ");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.print(ssid);
  Serial.print(" @ ");
  Serial.println(WiFi.localIP());
  Serial.println("Waiting for 5 sec for the setup to stabilize...");
  delay(5000);
  Serial.println("System Ready");

}

void loop() {
    
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Uncomment the required function
  readingData();
//  writingData();

  //instructs the PICC when in the ACTIVE state to go to a "STOP" state
  mfrc522.PICC_HaltA();
  
  // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
  mfrc522.PCD_StopCrypto1();
  
  delay(1000);
}

//reads data from card/tag
void readingData()
{
  //prints the technical details of the card/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 
  
  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  //buffer for read data
  byte buffer[SIZE_BUFFER] = {0};
  byte newBuffer[7] = {0};
 
  //the block to operate
  byte block = 1;
  byte size = SIZE_BUFFER;  //authenticates the block to operate
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //read data from block
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else{
      Serial.println("Reading Done");
  }

  Serial.print(F("\nData from block ["));
  Serial.print(block);Serial.print(F("]: "));

 //prints read data
  for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
  {
      Serial.write(buffer[i]);
//      if (i != 0)
      newBuffer[i] = buffer[i];
  }
  Serial.println(" ");
  String bufferData = (char*)newBuffer;
  Serial.print("The string formate: ");
  Serial.print(bufferData);
  Serial.println(" ");


  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    Serial.println("String to be uploaded");
    Serial.println(bufferData);
    
    String theRequest = url+"tag=BusNO"+"&value="+bufferData;
    Serial.println(theRequest);
    
    http.begin(theRequest.c_str());
    
    int httpRes = http.GET();

    if(httpRes > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpRes);
      String payload = http.getString();
      Serial.println(payload);
    }
    else 
    {
      Serial.print("Error code: ");
      Serial.println(httpRes);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }

  
}


void writingData()
{
  //prints thecnical details from of the card/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 
  
  // waits 30 seconds dor data entry via Serial 
  Serial.setTimeout(30000L) ;     
  Serial.println(F("Enter the data to be written with the '#' character at the end \n[maximum of 6 characters]:"));

  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //buffer para armazenamento dos dados que iremos gravar
  //buffer for storing data to write
  byte buffer[MAX_SIZE_BLOCK] = "";
  byte block; //the block to operate
  byte dataSize; //size of data (bytes)

  //recover on buffer the data from Serial
  //all characters before chacactere '#'
  dataSize = Serial.readBytesUntil('#', (char*)buffer, MAX_SIZE_BLOCK);
  
  //void positions that are left in the buffer will be filled with whitespace
  for(byte i=dataSize; i < MAX_SIZE_BLOCK; i++)
  {
    buffer[i] = ' ';
  }
 
  block = 1; //the block to operate
  String str = (char*)buffer; //transforms the buffer data in String
  Serial.println(str);

   //authenticates the block to operate
   //Authenticate is a command to hability a secure communication
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  //else Serial.println(F("PCD_Authenticate() success: "));
 
  //Writes in the block
  status = mfrc522.MIFARE_Write(block, buffer, MAX_SIZE_BLOCK);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else{
    Serial.println(F("MIFARE_Write() success: "));
  }
 
}
