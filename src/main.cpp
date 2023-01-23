#include<Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// buzzer and LED variables
  // notes in the melody:
  int buzzer = 6;
  int led = 5;
  int led_red = 2;
  int led_green = 4;

// Variable Declaration
  // iteration Variables (tries Vars)
  bool check = false;
  int numWriteTries = 3;
  int writeCntr  = 0;
  
  int numReadTries = 3;
  int readCntr = 0;
  
  int incDecTries = 3;
  int incDecCntr = 0;

  // read from Serial vars
  int s = 0;
  int k = 0;
  unsigned int low, high;
  char* value_2 = new char[2];
  char* value_4 = new char[4];
  char* value_8 = new char[8];
  int r = 0;
  int blksReadIndx = 0;
  char c;
  char message_to_server[512];
  // message variables
  int Header = 255;
  int dataLength = 0;
  int directionOfData = 0;
  int command = 0;
  int checkSumm = 0;
  int Footer = 205;
  struct time_struct
  {
    int seconds;
    int hours;
    int minutes;
    int days;
    int months;
    int years;
  };
  // readed message from server variables
  int availableBytes = 0;
  
  
  // parser variables
  int parserheader = 0;
  int parserdataLength = 0;
  int parserdirectionOfData = 0;
  int parsercommand = 0;
  time_struct parserdate = {0, 0, 0, 0, 0, 0};
  int parsercheckSum = 0;
  int parserfooter = 0;

  // data from messages
  byte blockIndx;
  byte blockIndx2;
  byte dataToWrite[16];
  byte numOfBlks = 0;
  byte keyType = 0;
  byte beepType = 0;
  byte ledID = 0;
  byte ledStatus = 0;
  byte stat = 0;  
  long valStrRead = 0;
  long valStrWrite = 0;
  long maxValStr = 0;
  long minValStr = 0;
  long rstValStr = 0;
  byte msgType = 0;
  int numBytes = 0;
  byte indexesOfBytes[16];
  byte bytesValues[16];
  

  // value block vars
  int valueIndex = 15; // the index which we will save the value amonge the dataValue Blocks
  byte valueBlockA    = 0; // the actual index of the block
  int trailerBlock = 23; // the tail of the sector of the value data
  int trailerBlockIdx = 0; // the tail of the sector of the value data
  int sector = 5;       // the index of the sector of the value data
  int32_t valueOfBlock = 0; // value to be set to the block
  int32_t maxValueOfBlock = 0; // max value to be set to the block
  int32_t minValueOfBlock = 0; // min value to be set to the block
  int32_t rstValueOfBlock = 0; // reset value to be set to the block
  int32_t value;            // variable to read the value of the block to
  byte buffer[18];          // char array to read the value of the block to
  byte size = sizeof(buffer); // size of buffer
  byte trailerBuffer[] = {    // this var is used to set the tail of the sector which contains value block
          255, 255, 255, 255, 255, 255,       // Keep default key A
          0, 0, 0,
          0,
          255, 255, 255, 255, 255, 255};      // Keep default key B
  byte d[4] = {0x00, 0x00, 0x00, 0x00};
          

  // access bits rights
  byte accessBit0 = 0;
  byte accessBit1 = 6;
  byte accessBit2 = 6;
  byte accessBit3 = 1;

  // read and write to multiple blocks vars
  typedef struct // this struct is used to save multiple blocks in series (saving the index of the block and the value of the block)
  {
    byte blkIndex;
    byte blockData[16];
  } BlockStruct;
  BlockStruct blocksArray[25];
  
  /* Set the block to which we want to write data */
  /* Be aware of Sector Trailer Blocks */
  int blockNum = 2;  
  /* Create an array of 16 Bytes and fill it with data */
  /* This is the actual data which is going to be written into the card */
  byte blockData [16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
  byte i = 0;
  byte j = 0;
  
  /* Create another array to read data from Block */
  /* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
  byte bufferLen = 18;
  byte readBlockData[18];
  
  // indexes of the datablocks 
  int sectorTails[16] = {3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63};
  int blocksForData[47] = {1,2,4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26,28,29,30,32,33,34,36,37,38,40,41,42,44,45,46,48,49,50,52,53,54,56,57,58,60,61,62};
  

  // key and status variables
  /* Create an instance of MIFARE_Key */
  MFRC522::MIFARE_Key key; 
  MFRC522::MIFARE_Key receivedKey;          
  
  MFRC522::StatusCode status;
  MFRC522::StatusCode statusRes;

  // cardType Struct
  typedef struct // this struct is used to save multiple blocks in series (saving the index of the block and the value of the block)
  {
    byte type;
    byte cardUid[10];
  } CardInfoStruct;
  CardInfoStruct cardInfo;

  byte checkRegister[18];
  
  
/*Using Hardware SPI of Arduino */
/*MOSI (11), MISO (12) and SCK (13) are fixed */
/*You can configure SS and RST Pins*/
#define SS_PIN 10  /* Slave Select Pin */
#define RST_PIN 7  /* Reset Pin */


/* Create an instance of MFRC522 */
MFRC522 mfrc522(SS_PIN, RST_PIN);

// function declaration
void stringToCharArray(char* hex, int len);
void turnLedBegin();
void turnLedSuccess();
void turnLedFail();
void turnOffAllLeds();
void turnOnLED();
void turnOffLED();
void turnOnLED_RED();
void turnOffLED_RED();
void turnOnLED_GREEN();
void turnOffLED_GREEN();
void makeBeep(byte beepType);
void statusOfOperation(int state);
MFRC522::StatusCode getCardInfo(byte keyType, MFRC522::MIFARE_Key key);
MFRC522::StatusCode getCardInfoCheckStatus(byte keyType, MFRC522::MIFARE_Key key);
void getCardInfoNoKey();
void readCardToSerial();

void parseMessage();
void gen_connection_message();
void genResponseSuccess(int commandErr, byte cardUID[]);
void genResponseERR(int commandErr, int errCode, byte cardUID[]);
void genReadBlockSuccess(int blkNum, byte readedData[], byte cardUID[]);
void genGetValueBlockSuccess(int blkNum, byte cardUID[]);
void genReadMultipleBlocksResponse(int numBlks, BlockStruct blocksArray[], byte cardUID[]);
void genReadCardInfoSuccess(CardInfoStruct cardInfoUID, byte cardUID[]);
void genReadValueAsStrSuccess(byte val[], byte cardUID[]);
void genSetBlkDecValueAsStrSuccess(byte val[], byte fee[], byte cardUID[]);
void genResponseIncDecSuccess(int commandErr, byte cardUID[], byte val[]);

MFRC522::StatusCode WriteDataToBlockModified(byte keyType, int blockNumber, byte blockDataaa[], MFRC522::MIFARE_Key key);
MFRC522::StatusCode ReadDataFromBlockModified(byte keyType, int blockNum, byte readBlockData[], MFRC522::MIFARE_Key key);
MFRC522::StatusCode formatValueBlockProceedure(byte keyType, byte blockAddr,byte tailerIndex, MFRC522::MIFARE_Key key);
MFRC522::StatusCode formatValueBlock(byte blockAddr);
MFRC522::StatusCode setTailerInfo(byte keyType, int32_t trailerBlock, MFRC522::MIFARE_Key key,  byte trailerBuffer[]);
MFRC522::StatusCode incrmentValueBlockWithSet(int keyType, int32_t trailerBlock, int32_t valueBlockA, int32_t val, int32_t maxVal, int32_t resetVal, MFRC522::MIFARE_Key key);
MFRC522::StatusCode decrementBlockValueWithSet(int keyType, int32_t trailerBlock, int32_t valueBlockB, int32_t valDecr, int32_t lowerSide, int32_t valReset, MFRC522::MIFARE_Key key);
MFRC522::StatusCode getValue(int keyType, int32_t trailerBlock, int32_t valueBlockB, MFRC522::MIFARE_Key key);
MFRC522::StatusCode setValue(int keyType, int32_t trailerBlock, int32_t valueBlockB, int32_t val, MFRC522::MIFARE_Key key);
MFRC522::StatusCode WriteMultipleDataBlocks(byte keyType, int numBlks, BlockStruct blocksArray[],  MFRC522::MIFARE_Key key);
MFRC522::StatusCode ReadMultipleDataBlocks(byte keyType, int numBlks,byte readBlockData[], BlockStruct blocksArray[],  MFRC522::MIFARE_Key key);
MFRC522::StatusCode WriteDataToBlockNoTries(byte keyType, int blockNumber, byte blockDataaa[], MFRC522::MIFARE_Key key);
MFRC522::StatusCode ReadDataFromBlockNoTries(byte keyType, int blockNum, byte readBlockData[], MFRC522::MIFARE_Key key);
MFRC522::StatusCode WriteMultipleDataBlocksRegister(byte keyType, int numBlks, BlockStruct blocksArray[],  MFRC522::MIFARE_Key key);

void setup() 
{
  Serial.begin(115200);
  //Serial.println(millis());
  pinMode(buzzer,OUTPUT);//initialize the buzzer pin as an output
  pinMode(led,OUTPUT); // 
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  digitalWrite(led,HIGH);
  digitalWrite(led_red,HIGH);
  digitalWrite(led_green,HIGH);
  /* Initialize serial communications with the PC */
  
  /* Initialize SPI bus */
  SPI.begin();
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  //Serial.println("Scan a MIFARE 1K Tag to write data...");
  sei();
  for (i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  for (i = 0; i < 6; i++)
  {
    receivedKey.keyByte[i] = 0xFF;
  }
  
  //Serial.println(millis());
}

void loop()
{
  delay(200);
}

void serialEvent() {
    r = 0;
    //memset(message_to_server,0,sizeof(message_to_server));
    while (Serial.available()) {
      c = Serial.read();
      message_to_server[r] = c;
      r++;
      //Serial.print(c);
      //Serial.println('*');
    }

    //return;
    mfrc522.PCD_Reset();
    //Serial.print("Reset is done\n");
    mfrc522.PCD_Init();
    
    //Serial.write(message_to_server,r);
    
    //Serial.write(parsercommand);
    parseMessage();
    parsercommand = ((int) message_to_server[11]) + ((int) message_to_server[10])*256;
    //Serial.write(parsercommand);
    if(parsercommand == 1)
    {
      gen_connection_message();
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
    }
    else if(parsercommand == 2)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      for(i=0;i<16;i++)
      {
        dataToWrite[i] = (byte) message_to_server[20 + i];
      }
      blockNum = blocksForData[blockIndx];
      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfo(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      statusRes = WriteDataToBlockModified(keyType, blockNum, dataToWrite, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 3)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      blockNum = blocksForData[blockIndx];
      turnLedBegin();
      makeBeep(0);
      statusRes = ReadDataFromBlockModified(keyType, blockNum, readBlockData, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      genReadBlockSuccess(blockNum, readBlockData, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 4)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      numOfBlks = (byte) message_to_server[20];
      //Serial.write(numOfBlks);
      blksReadIndx = 0;
      for(i = 0; i < numOfBlks; i++)
      {
        blocksArray[i].blkIndex = blocksForData[message_to_server[21 + i + blksReadIndx]];
        //Serial.write(blocksArray[i].blkIndex);
        
        for(j = 0; j < 16; j++)
        {
          blocksArray[i].blockData[j] = message_to_server[21 + i + j + 1 + blksReadIndx];
          //Serial.write(blocksArray[i].blockData[j]);
        }
        //Serial.write(0xaa);Serial.write(0xaa);
        blksReadIndx += 16;
      }
      turnLedBegin();
      makeBeep(0);
      statusRes = WriteMultipleDataBlocks(keyType, numOfBlks, blocksArray, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();      
    }
    else if(parsercommand == 5)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      trailerBlockIdx = (byte) message_to_server[14];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[15 + i];
      }
      blockNum = blocksForData[blockIndx];
      trailerBlock = sectorTails[trailerBlockIdx];
      turnLedBegin();
      makeBeep(0);
      statusRes = formatValueBlockProceedure(keyType, blockNum,trailerBlock, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 6)
    {
      keyType = (byte) message_to_server[12];      
      sector = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      trailerBlockIdx = (byte) message_to_server[20];
      accessBit0 = (byte) message_to_server[21];
      accessBit1 = (byte) message_to_server[22];
      accessBit2 = (byte) message_to_server[23];
      accessBit3 = (byte) message_to_server[24];
      for(i=0;i<16;i++)
      {
        trailerBuffer[i] = (byte) message_to_server[25 + i];
      }
      trailerBlock = sectorTails[trailerBlockIdx];
      
      turnLedBegin();
      makeBeep(0);
      statusRes = setTailerInfo(keyType, trailerBlock, receivedKey,  trailerBuffer);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
      
    }
    else if(parsercommand == 7)
    {
      keyType = (byte) message_to_server[12];
      sector = (byte) message_to_server[13];
      trailerBlockIdx = (byte) message_to_server[14];
      blockIndx = (byte) message_to_server[15];
      valueOfBlock = (int32_t) (message_to_server[16]*16777216) + (int32_t) (message_to_server[17]*65536) + (int32_t) (message_to_server[18]*256) + (int32_t) message_to_server[19];
      maxValueOfBlock = (int32_t) message_to_server[20]*16777216 + (int32_t) message_to_server[21]*65536 + (int32_t) message_to_server[22]*256 + (int32_t) message_to_server[23];
      rstValueOfBlock = (int32_t) message_to_server[24]*16777216 + (int32_t) message_to_server[25]*65536 + (int32_t) message_to_server[26]*256 + (int32_t) message_to_server[27];
      if(valueOfBlock < 0)
      {
        valueOfBlock += 65536;
      }
      if(maxValueOfBlock < 0)
      {
        maxValueOfBlock += 65536;
      }
      if(rstValueOfBlock < 0)
      {
        rstValueOfBlock += 65536;
      }
      //Serial.print("valueOfBlock: "); Serial.println(valueOfBlock);
      //Serial.print("maxValueOfBlock: "); Serial.println(maxValueOfBlock);
      //Serial.print("rstValueOfBlock: "); Serial.println(rstValueOfBlock);
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[28 + i];
      }
      trailerBlock = sectorTails[trailerBlockIdx];
      valueBlockA = blocksForData[blockIndx];
      turnLedBegin();
      makeBeep(0);
      statusRes = incrmentValueBlockWithSet(keyType, trailerBlock, valueBlockA, valueOfBlock, maxValueOfBlock, rstValueOfBlock, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
  }
    else if(parsercommand == 8)
      {
        keyType = (byte) message_to_server[12];
        sector = (byte) message_to_server[13];
        trailerBlockIdx = (byte) message_to_server[14];
        blockIndx = (byte) message_to_server[15];
        valueOfBlock = (byte) message_to_server[16]*16777215 + (byte) message_to_server[17]*65536 + (byte) message_to_server[18]*256 + (byte) message_to_server[19];
        minValueOfBlock = (byte) message_to_server[20]*16777215 + (byte) message_to_server[21]*65536 + (byte) message_to_server[22]*256 + (byte) message_to_server[23];
        rstValueOfBlock = (byte) message_to_server[24]*16777215 + (byte) message_to_server[25]*65536 + (byte) message_to_server[26]*256 + (byte) message_to_server[27];
        if(valueOfBlock < 0)
        {
          valueOfBlock += 65536;
        }
        if(minValueOfBlock < 0)
        {
          minValueOfBlock += 65536;
        }
        if(rstValueOfBlock < 0)
        {
          rstValueOfBlock += 65536;
        }
        for (i = 0; i < 6; i++)
        {
          receivedKey.keyByte[i] = (byte) message_to_server[28 + i];
        }
        trailerBlock = sectorTails[trailerBlockIdx];
        valueBlockA = blocksForData[blockIndx];
        turnLedBegin();
        makeBeep(0);
        
        statusRes = decrementBlockValueWithSet(keyType, trailerBlock, valueBlockA, valueOfBlock, minValueOfBlock, rstValueOfBlock, receivedKey);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        
        genResponseSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
    }
    else if(parsercommand == 9)
      {
        keyType = (byte) message_to_server[12];
        sector = (byte) message_to_server[13];
        trailerBlockIdx = (byte) message_to_server[14];
        blockIndx = (byte) message_to_server[15];
        for (i = 0; i < 6; i++)
        {
          receivedKey.keyByte[i] = (byte) message_to_server[16 + i];
        }
        trailerBlock = sectorTails[trailerBlockIdx];
        valueBlockA = blocksForData[blockIndx];
        turnLedBegin();
        makeBeep(0);
        
        statusRes = getValue(keyType, trailerBlock, valueBlockA, receivedKey);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        
        genGetValueBlockSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
    }
    else if(parsercommand == 10)
      {
        keyType = (byte) message_to_server[12];
        sector = (byte) message_to_server[13];
        trailerBlockIdx = (byte) message_to_server[14];
        blockIndx = (byte) message_to_server[15];
        valueOfBlock = (byte) message_to_server[16]*16777215 + (byte) message_to_server[17]*65536 + (byte) message_to_server[18]*256 + (byte) message_to_server[19];
        for (i = 0; i < 6; i++)
        {
          receivedKey.keyByte[i] = (byte) message_to_server[20 + i];
        }
        trailerBlock = sectorTails[trailerBlockIdx];
        valueBlockA = blocksForData[blockIndx];
        turnLedBegin();
        makeBeep(0);
        
        statusRes = setValue(keyType, trailerBlock, valueBlockA, valueOfBlock, receivedKey);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        
        genResponseSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
    }
    else if(parsercommand == 11)
      {
        beepType = (byte) message_to_server[12];
        
        makeBeep(beepType);
        
      
        genResponseSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
    }
    else if(parsercommand == 12)
      {
        ledID = (byte) message_to_server[12];
        ledStatus = (byte) message_to_server[13];
        
        if(ledID == 0 && ledStatus == 0)
        {
          turnOffLED();
        }
        else if(ledID == 0 && ledStatus == 1)
        {
          turnOnLED();
        }
        else if(ledID == 1 && ledStatus == 0)
        {
          turnOffLED_RED();
        }
        else if(ledID == 1 && ledStatus == 1)
        {
          turnOnLED_RED();
        }
        else if(ledID == 2 && ledStatus == 0)
        {
          turnOffLED_GREEN();
        }
        else if(ledID == 2 && ledStatus == 1)
        {
          turnOnLED_GREEN();
        }
        else if(ledID == 100 && ledStatus == 100)
        {
          turnOffAllLeds();
        }
      
        genResponseSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
    }
    else if(parsercommand == 13)
    {
      turnLedBegin();
        makeBeep(0);
        readCardToSerial();
        
        
        genResponseSuccess(parsercommand, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
    }
    else if(parsercommand == 14)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      numOfBlks = (byte) message_to_server[20];
      //Serial.write(numOfBlks);
      for(i = 0; i < numOfBlks; i++)
      {
        blocksArray[i].blkIndex = blocksForData[message_to_server[21 + i]];
        //Serial.write(21 + i + blksReadIndx);
        //Serial.write(blocksArray[i].blkIndex);
      }
      turnLedBegin();
      makeBeep(0);
      
      statusRes = ReadMultipleDataBlocks(keyType, numOfBlks,readBlockData, blocksArray,  receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      genReadMultipleBlocksResponse(numOfBlks, blocksArray, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 15)
    {
      turnLedBegin();
      makeBeep(0);
        
      getCardInfoNoKey();
        
      genReadCardInfoSuccess(cardInfo, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 16)
    {
      stat = (byte)message_to_server[12];
      statusOfOperation(stat);

      //genResponseSuccess(parsercommand);
      //stringToCharArray(message_to_server, 2*(dataLength + 3));
      //Serial.write(message_to_server, dataLength + 3);
    }
    else if(parsercommand == 17)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      blockNum = blocksForData[blockIndx];
      
      valStrRead = 0;
      d[0] = (byte) message_to_server[20];
      d[1] = (byte) message_to_server[21];
      d[2] = (byte) message_to_server[22];
      d[3] = (byte) message_to_server[23];
      valStrRead += (long)d[0] << 24;
      valStrRead += (long)d[1] << 16;
      valStrRead += (long)d[2] << 8;
      valStrRead += (long)d[3];
      maxValStr = 0;
      d[0] = (byte) message_to_server[24];
      d[1] = (byte) message_to_server[25];
      d[2] = (byte) message_to_server[26];
      d[3] = (byte) message_to_server[27];
      maxValStr += (long)d[0] << 24;
      maxValStr += (long)d[1] << 16;
      maxValStr += (long)d[2] << 8;
      maxValStr += (long)d[3];
      rstValStr = 0;
      d[0] = (byte) message_to_server[28];
      d[1] = (byte) message_to_server[29];
      d[2] = (byte) message_to_server[30];
      d[3] = (byte) message_to_server[31];
      rstValStr += (long)d[0] << 24;
      rstValStr += (long)d[1] << 16;
      rstValStr += (long)d[2] << 8;
      rstValStr += (long)d[3];
      
      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfoCheckStatus(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }

      if((int) checkRegister[0] == 0)
      {
        genResponseERR(parsercommand, 0xFA, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }

      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      statusRes = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }

      d[0] = readBlockData[0];
      d[1] = readBlockData[1];
      d[2] = readBlockData[2];
      d[3] = readBlockData[3];

      valStrWrite = 0;
      valStrWrite += (long)d[0]<< 24;
      valStrWrite += (long)d[1] << 16;
      valStrWrite += (long)d[2] << 8;
      valStrWrite += (long)d[3];

      valStrWrite += valStrRead;
      if(valStrWrite > maxValStr)
      {
        valStrWrite = rstValStr;
        //Serial.println(valStrWrite);
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        } 
        statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        
        genResponseIncDecSuccess(parsercommand, cardInfo.cardUid, readBlockData);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
      else
      {
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        //Serial.write(readBlockData[0]);
        //Serial.write(readBlockData[1]);
        //Serial.write(readBlockData[2]);
        //Serial.write(readBlockData[3]);
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        }
        statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
      
        genResponseIncDecSuccess(parsercommand, cardInfo.cardUid, readBlockData);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
      
    }
    else if(parsercommand == 18)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      blockNum = blocksForData[blockIndx];
      
      valStrRead = 0;
      d[0] = (byte) message_to_server[20];
      d[1] = (byte) message_to_server[21];
      d[2] = (byte) message_to_server[22];
      d[3] = (byte) message_to_server[23];
      valStrRead += (long)d[0]<< 24;
      valStrRead += (long)d[1] << 16;
      valStrRead += (long)d[2] << 8;
      valStrRead += (long)d[3];
      minValStr = 0;
      d[0] = (byte) message_to_server[24];
      d[1] = (byte) message_to_server[25];
      d[2] = (byte) message_to_server[26];
      d[3] = (byte) message_to_server[27];
      minValStr += (long)d[0]<< 24;
      minValStr += (long)d[1] << 16;
      minValStr += (long)d[2] << 8;
      minValStr += (long)d[3];
      rstValStr = 0;
      d[0] = (byte) message_to_server[28];
      d[1] = (byte) message_to_server[29];
      d[2] = (byte) message_to_server[30];
      d[3] = (byte) message_to_server[31];
      rstValStr += (long)d[0]<< 24;
      rstValStr += (long)d[1] << 16;
      rstValStr += (long)d[2] << 8;
      rstValStr += (long)d[3];

      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfoCheckStatus(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      if((int) checkRegister[0] == 0)
      {
        genResponseERR(parsercommand, 0xFA, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      statusRes = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      d[0] = (byte) readBlockData[0];
      d[1] = (byte) readBlockData[1];
      d[2] = (byte) readBlockData[2];
      d[3] = (byte) readBlockData[3];

      valStrWrite = 0;
      valStrWrite += (long)d[0] << 24;
      valStrWrite += (long)d[1] << 16;
      valStrWrite += (long)d[2] << 8;
      valStrWrite += (long)d[3];

      valStrWrite -= valStrRead;
      if(valStrWrite < minValStr)
      {
        valStrWrite = rstValStr;
        //Serial.println(valStrWrite);
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        } 
        statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
      
        genResponseIncDecSuccess(parsercommand, cardInfo.cardUid, readBlockData);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
      else
      {
        //Serial.println(valStrWrite);
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        //Serial.write(readBlockData[0]);
        //Serial.write(readBlockData[1]);
        //Serial.write(readBlockData[2]);
        //Serial.write(readBlockData[3]);
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        }
        statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
      
        genResponseIncDecSuccess(parsercommand, cardInfo.cardUid, readBlockData);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
      
    }
    else if(parsercommand == 19)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      blockNum = blocksForData[blockIndx];
      
      valStrRead = 0;
      d[0] = (byte) message_to_server[20];
      d[1] = (byte) message_to_server[21];
      d[2] = (byte) message_to_server[22];
      d[3] = (byte) message_to_server[23];
      valStrRead += (long)d[0]<< 24;
      valStrRead += (long)d[1] << 16;
      valStrRead += (long)d[2] << 8;
      valStrRead += (long)d[3];

      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfo(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      for(i=0;i<16;i++)
      {
        readBlockData[i] = 0x00;
      } 
      readBlockData[3] = valStrRead & 0xFF;
      readBlockData[2] = (valStrRead >> 8) & 0xFF;
      readBlockData[1] = (valStrRead >> 16) & 0xFF;
      readBlockData[0] = (valStrRead >> 24) & 0xFF;
      for(i=4;i<16;i++)
      {
        readBlockData[i] = 0x00;
      } 
      statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
    
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
      
    }
    else if(parsercommand == 20)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      blockNum = blocksForData[blockIndx];
      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfo(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      statusRes = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }

      genReadValueAsStrSuccess(readBlockData, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();

    }
    else if(parsercommand == 21)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      numBytes = (byte) message_to_server[20];
      j = 0;
      for(i=0;i<numBytes*2;i+=2)
      {
        indexesOfBytes[j] = (byte)message_to_server[21 + i];
        bytesValues[j] = (byte)message_to_server[21 + i + 1];
        j++;
      }
      blockNum = blocksForData[blockIndx];
      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfo(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      statusRes = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      for(i=0;i<numBytes;i++)
      {
        readBlockData[indexesOfBytes[i]] = bytesValues[i];
      }
      statusRes = mfrc522.MIFARE_Write(blockNum, readBlockData, 16);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
    
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 22)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      blockIndx2 = (byte) message_to_server[14];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[15 + i];
      }
      for(i=0;i<16;i++)
      {
        dataToWrite[i] = (byte) message_to_server[21 + i];
      }
      blockNum = blocksForData[blockIndx];
      valueBlockA = blocksForData[blockIndx2];
      valStrRead = 0;
      d[0] = (byte) message_to_server[37];
      d[1] = (byte) message_to_server[38];
      d[2] = (byte) message_to_server[39];
      d[3] = (byte) message_to_server[40];
      valStrRead += (long)d[0]<< 24;
      valStrRead += (long)d[1] << 16;
      valStrRead += (long)d[2] << 8;
      valStrRead += (long)d[3];
      minValStr = 0;
      d[0] = (byte) message_to_server[41];
      d[1] = (byte) message_to_server[42];
      d[2] = (byte) message_to_server[43];
      d[3] = (byte) message_to_server[44];
      minValStr += (long)d[0]<< 24;
      minValStr += (long)d[1] << 16;
      minValStr += (long)d[2] << 8;
      minValStr += (long)d[3];
      rstValStr = 0;
      d[0] = (byte) message_to_server[45];
      d[1] = (byte) message_to_server[46];
      d[2] = (byte) message_to_server[47];
      d[3] = (byte) message_to_server[48];
      rstValStr += (long)d[0]<< 24;
      rstValStr += (long)d[1] << 16;
      rstValStr += (long)d[2] << 8;
      rstValStr += (long)d[3];
      turnLedBegin();
      makeBeep(0);
      statusRes = getCardInfo(keyType, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      
      statusRes = mfrc522.MIFARE_Write(blockNum, dataToWrite, 16);
      if (status != MFRC522::STATUS_OK)
      {
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      if(keyType == 0)
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, valueBlockA, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      else
      {
        statusRes = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, valueBlockA, &receivedKey, &(mfrc522.uid));
        if (statusRes != MFRC522::STATUS_OK)
        {
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
        else{}
      }
      
      statusRes = mfrc522.MIFARE_Read(valueBlockA, readBlockData, &bufferLen);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      d[0] = (byte) readBlockData[0];
      d[1] = (byte) readBlockData[1];
      d[2] = (byte) readBlockData[2];
      d[3] = (byte) readBlockData[3];

      valStrWrite = 0;
      valStrWrite += (long)d[0] << 24;
      valStrWrite += (long)d[1] << 16;
      valStrWrite += (long)d[2] << 8;
      valStrWrite += (long)d[3];

      valStrWrite -= valStrRead;
      if(valStrWrite < minValStr)
      {
        valStrWrite = rstValStr;
        //Serial.println(valStrWrite);
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        } 
        statusRes = mfrc522.MIFARE_Write(valueBlockA, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
      
        genSetBlkDecValueAsStrSuccess(dataToWrite, readBlockData, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
      else
      {
        //Serial.println(valStrWrite);
        readBlockData[3] = valStrWrite & 0xFF;
        readBlockData[2] = (valStrWrite >> 8) & 0xFF;
        readBlockData[1] = (valStrWrite >> 16) & 0xFF;
        readBlockData[0] = (valStrWrite >> 24) & 0xFF;
        //Serial.write(readBlockData[0]);
        //Serial.write(readBlockData[1]);
        //Serial.write(readBlockData[2]);
        //Serial.write(readBlockData[3]);
        for(i=4;i<16;i++)
        {
          readBlockData[i] = 0x00;
        }
        statusRes = mfrc522.MIFARE_Write(valueBlockA, readBlockData, 16);
        if(statusRes != MFRC522::STATUS_OK){
          genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
          //Serial.println(mfrc522.GetStatusCodeName(statusRes));
          stringToCharArray(message_to_server, 2*(dataLength + 3));
          Serial.write(message_to_server, dataLength + 3);
          turnLedFail();
          makeBeep(2);
          turnOffAllLeds();
          return;
        }
      
        genSetBlkDecValueAsStrSuccess(dataToWrite, readBlockData, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedSuccess();
        makeBeep(1);
        turnOffAllLeds();
      }
    }
    else if(parsercommand == 23)
    {
      keyType = (byte) message_to_server[12];
      blockIndx = (byte) message_to_server[13];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[14 + i];
      }
      numOfBlks = (byte) message_to_server[20];
      //Serial.write(numOfBlks);
      blksReadIndx = 0;
      for(i = 0; i < numOfBlks; i++)
      {
        blocksArray[i].blkIndex = blocksForData[message_to_server[21 + i + blksReadIndx]];
        //Serial.write(blocksArray[i].blkIndex);
        
        for(j = 0; j < 16; j++)
        {
          blocksArray[i].blockData[j] = message_to_server[21 + i + j + 1 + blksReadIndx];
          //Serial.write(blocksArray[i].blockData[j]);
        }
        //Serial.write(0xaa);Serial.write(0xaa);
        blksReadIndx += 16;
      }
      turnLedBegin();
      makeBeep(0);
      statusRes = WriteMultipleDataBlocksRegister(keyType, numOfBlks, blocksArray, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        //Serial.println(mfrc522.GetStatusCodeName(statusRes));
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds();
    }
    else if(parsercommand == 24)
    {
      numOfBlks = 12;
      keyType = (byte) message_to_server[12];
      for (i = 0; i < 6; i++)
      {
        receivedKey.keyByte[i] = (byte) message_to_server[13 + i];
      }
      for(i = 0; i < numOfBlks; i++)
      {
        blocksArray[i].blkIndex = blocksForData[i];
        
        for(j = 0; j < 16; j++)
        {
          blocksArray[i].blockData[j] = 0x00;
        }
      }
      
      turnLedBegin();
      makeBeep(0);
      statusRes = WriteMultipleDataBlocks(keyType, numOfBlks, blocksArray, receivedKey);
      if(statusRes != MFRC522::STATUS_OK){
        genResponseERR(parsercommand, (byte) statusRes, cardInfo.cardUid);
        stringToCharArray(message_to_server, 2*(dataLength + 3));
        Serial.write(message_to_server, dataLength + 3);
        turnLedFail();
        makeBeep(2);
        turnOffAllLeds();
        return;
      }
      
      genResponseSuccess(parsercommand, cardInfo.cardUid);
      stringToCharArray(message_to_server, 2*(dataLength + 3));
      Serial.write(message_to_server, dataLength + 3);
      turnLedSuccess();
      makeBeep(1);
      turnOffAllLeds(); 
    }
}

void getCardInfoNoKey()
{
  cardInfo.type = 0x00;
  memset(cardInfo.cardUid,0,10);
  mfrc522.PCD_Reset();
  mfrc522.PCD_Init();
  while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
  }
  
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  for (i = 0; i < mfrc522.uid.size; i++)
  {
    cardInfo.cardUid[i] = mfrc522.uid.uidByte[i];
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  cardInfo.type = piccType;
}

MFRC522::StatusCode getCardInfo(byte keyType, MFRC522::MIFARE_Key key)
{
  status = MFRC522::STATUS_OK;
  cardInfo.type = 0x00;
  memset(cardInfo.cardUid,0,10);
  mfrc522.PCD_Reset();
  mfrc522.PCD_Init();
  while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
  }
  
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  for (i = 0; i < mfrc522.uid.size; i++)
  {
    cardInfo.cardUid[i] = mfrc522.uid.uidByte[i];
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  cardInfo.type = piccType;
  
  if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 2, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
  else
  {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 2, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
      return status;
    }
    else{}
  }
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(2, checkRegister, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }
  else
  {}

  return status;
}

MFRC522::StatusCode getCardInfoCheckStatus(byte keyType, MFRC522::MIFARE_Key key)
{
  status = MFRC522::STATUS_OK;
  cardInfo.type = 0x00;
  memset(cardInfo.cardUid,0,10);
  mfrc522.PCD_Reset();
  mfrc522.PCD_Init();
  while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    delay(100);
  }
  
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  for (i = 0; i < mfrc522.uid.size; i++)
  {
    cardInfo.cardUid[i] = mfrc522.uid.uidByte[i];
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  cardInfo.type = piccType;
  
  if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 13, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
  else
  {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 13, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
      return status;
    }
    else{}
  }
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(13, checkRegister, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    return status;
  }
  else
  {}

  return status;
}

MFRC522::StatusCode WriteMultipleDataBlocks(byte keyType, int numBlks, BlockStruct blocksArray[],  MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  getCardInfoNoKey();
  for(i = 0; i < numBlks; i++)
  {
    statusRes = WriteDataToBlockNoTries(keyType, blocksArray[i].blkIndex, blocksArray[i].blockData, key);
    if (status != MFRC522::STATUS_OK)
    {
      break;
    }
  }
  //readCardToSerial();
  return status;
}

MFRC522::StatusCode WriteMultipleDataBlocksRegister(byte keyType, int numBlks, BlockStruct blocksArray[],  MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
  check = false;
  for(i=0;i<16;i++)
  {
    if(blocksArray[0].blockData[i] != checkRegister[i])
    {
      check = true;
      break;
      
    }
  }
  if(check == false)
  {
    status = MFRC522::STATUS_INTERNAL_ERROR;
    return status;
  }

  for(i = 0; i < numBlks; i++)
  {
    statusRes = WriteDataToBlockNoTries(keyType, blocksArray[i].blkIndex, blocksArray[i].blockData, key);
    if (status != MFRC522::STATUS_OK)
    {
      break;
    }
  }
  return status;
}

MFRC522::StatusCode ReadMultipleDataBlocks(byte keyType, int numBlks,byte readBlockData[], BlockStruct blocksArray[],  MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
  
  for(i = 0; i < numBlks; i++)
  {
    
    statusRes = ReadDataFromBlockNoTries(keyType, blocksArray[i].blkIndex, readBlockData, key);
    for(int j=0;j<16;j++)
    {
      blocksArray[i].blockData[j] = readBlockData[j];
      //Serial.write(blocksArray[i].blockData[j]);
    }
    //Serial.write(0x0d);Serial.write(0x0a);
    if (status != MFRC522::STATUS_OK)
    {
      break;
    }
  }
  //readCardToSerial();
  return status;
}

MFRC522::StatusCode WriteDataToBlockModified(byte keyType, int blockNumber, byte blockDataaa[], MFRC522::MIFARE_Key key)
{
  mfrc522.PCD_Reset();
  mfrc522.PCD_Init();
  writeCntr  = 0;
  status = MFRC522::STATUS_OK;
  while(writeCntr < numWriteTries)
  {
    while ( ! mfrc522.PICC_IsNewCardPresent());
  
    /* Select one of the cards */
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      continue;
    }
    if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        writeCntr++;
        continue;
      }
      else{}
    }
    else
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNumber, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        writeCntr++;
        continue;
      }
      else{}
    }
    

    /* Write data to the block */    
    status = mfrc522.MIFARE_Write(blockNumber, blockDataaa, 16);
    if (status != MFRC522::STATUS_OK)
    {
      writeCntr++;
    }
    else{break;}

    mfrc522.PCD_Reset();
    //Serial.print("Reset is done\n");
    mfrc522.PCD_Init();
  }
  return status;
}

MFRC522::StatusCode WriteDataToBlockNoTries(byte keyType, int blockNumber, byte blockDataaa[], MFRC522::MIFARE_Key key)
{
  if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
    else
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNumber, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
    

    /* Write data to the block */    
    status = mfrc522.MIFARE_Write(blockNumber, blockDataaa, 16);
    if (status != MFRC522::STATUS_OK)
    {
      return status;
    }
    else{}

    //mfrc522.PCD_Reset();
    //Serial.print("Reset is done\n");
    //mfrc522.PCD_Init();
  return status;
}

MFRC522::StatusCode ReadDataFromBlockNoTries(byte keyType, int blockNum, byte readBlockData[], MFRC522::MIFARE_Key key)
{
  if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
    else
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        return status;
      }
      else{}
    }
    /* Reading data from the Block */
    status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
    if (status != MFRC522::STATUS_OK)
    {
      return status;
    }
    else
    {}

  return status;
}

MFRC522::StatusCode ReadDataFromBlockModified(byte keyType, int blockNum, byte readBlockData[], MFRC522::MIFARE_Key key) 
{
  /* Authenticating the desired data block for Read access using Key A */
  mfrc522.PCD_Reset();
  //Serial.print("Reset is done\n");
  mfrc522.PCD_Init();
  status = MFRC522::STATUS_OK;
  readCntr = 0;
  while(readCntr < numReadTries)
  {
    while ( ! mfrc522.PICC_IsNewCardPresent());
  
    /* Select one of the cards */
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      continue;
    }
    if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        writeCntr++;
        continue;
      }
      else{}
    }
    else
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockNum, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        writeCntr++;
        continue;
      }
      else{}
    }
    /* Reading data from the Block */
    status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
    if (status != MFRC522::STATUS_OK)
    {
      readCntr++;
    }
    else
    {break;}

    mfrc522.PCD_Reset();
    //Serial.print("Reset is done\n");
    mfrc522.PCD_Init();
  }

  return status;
}

void readCardToSerial()
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return;
  }
  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}
 /*
 *
 * Ensure that a given block is formatted as a Value Block.
 */
MFRC522::StatusCode formatValueBlockProceedure(byte keyType, byte blockAddr,byte trailerBlock, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
  
  if(keyType == 0)
  {
    // Authenticate using key A
    //Serial.println(F("Authenticating again using key B..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }
  else
  {
    // Authenticate using key B
    //Serial.println(F("Authenticating again using key B..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }

  status = formatValueBlock(blockAddr);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
   return status;
}

MFRC522::StatusCode formatValueBlock(byte blockAddr) {
    byte buffer[18];
    byte size = sizeof(buffer);
    MFRC522::StatusCode status = MFRC522::STATUS_OK;

    //Serial.print(F("Reading block ")); Serial.println(blockAddr);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("MIFARE_Read() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }

    if (    (buffer[0] == (byte)~buffer[4])
        &&  (buffer[1] == (byte)~buffer[5])
        &&  (buffer[2] == (byte)~buffer[6])
        &&  (buffer[3] == (byte)~buffer[7])

        &&  (buffer[0] == buffer[8])
        &&  (buffer[1] == buffer[9])
        &&  (buffer[2] == buffer[10])
        &&  (buffer[3] == buffer[11])

        &&  (buffer[12] == (byte)~buffer[13])
        &&  (buffer[12] ==        buffer[14])
        &&  (buffer[12] == (byte)~buffer[15])) {
        //Serial.println(F("Block has correct Value Block format."));
    }
    else {
        //Serial.println(F("Formatting as Value Block..."));
        byte valueBlock[] = {
            0, 0, 0, 0,
            255, 255, 255, 255,
            0, 0, 0, 0,
            blockAddr, ~blockAddr, blockAddr, ~blockAddr };
        status = mfrc522.MIFARE_Write(blockAddr, valueBlock, 16);
        if (status != MFRC522::STATUS_OK) {
            //Serial.print(F("MIFARE_Write() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
        }
    }
    return status;
}

MFRC522::StatusCode setTailerInfo(byte keyType, int32_t trailerBlock, MFRC522::MIFARE_Key key,  byte trailerBuffer[])
{
    MFRC522::StatusCode status = MFRC522::STATUS_OK;
    status = getCardInfo(keyType, key);
    if(status != MFRC522::STATUS_OK){
      return status;
    }
    // Authenticate using key A
    //Serial.println(F("Authenticating using key A..."));
    if(keyType == 0)
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
    }
    else
    {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
    }
    

    // Show the whole sector as it currently is
    //Serial.println(F("Current data in sector:"));
    //mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    //Serial.println();
    mfrc522.MIFARE_SetAccessBits(&trailerBuffer[6], accessBit0, accessBit1, accessBit2, accessBit3);

    // Read the sector trailer as it is currently stored on the PICC
    //Serial.println(F("Reading sector trailer..."));
    status = mfrc522.MIFARE_Read(trailerBlock, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("MIFARE_Read() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
    // Check if it matches the desired access pattern already;
    // because if it does, we don't need to write it again...
    if (    buffer[6] != trailerBuffer[6]
        ||  buffer[7] != trailerBuffer[7]
        ||  buffer[8] != trailerBuffer[8]) {
        // They don't match (yet), so write it to the PICC
        //Serial.println(F("Writing new sector trailer..."));
        status = mfrc522.MIFARE_Write(trailerBlock, trailerBuffer, 16);
        if (status != MFRC522::STATUS_OK) {
            //Serial.print(F("MIFARE_Write() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
            return status;
        }
    }
    return status;
}

MFRC522::StatusCode incrmentValueBlockWithSet(int keyType, int32_t trailerBlock, int32_t valueBlockA, int32_t val, int32_t maxVal, int32_t resetVal, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
    
  if(keyType == 0)
  {
    //Authenticate using key A
    //Serial.println(F("Authenticating again using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }
  else
  {
    //Authenticate using key B
    //Serial.println(F("Authenticating again using key B..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }
  

  // A value block has a 32 bit signed value stored three times
  // and an 8 bit address stored 4 times. Make sure that valueBlockA
  // and valueBlockB have that format (note that it will only format
  // the block when it doesn't comply to the expected format already).
  status = formatValueBlock(valueBlockA);
  if (status != MFRC522::STATUS_OK) {
      //Serial.print(F("PCD_Authenticate() failed: "));
      //Serial.println(mfrc522.GetStatusCodeName(status));
      return status;
  }

    // Add 1 to the value of valueBlockA and store the result in valueBlockA.
    //Serial.print("Adding to value of block "); Serial.print(valueBlockA); Serial.print(" value = ");Serial.println(val);
    status = mfrc522.MIFARE_Increment(valueBlockA, val);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("MIFARE_Increment() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
    status = mfrc522.MIFARE_Transfer(valueBlockA);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("MIFARE_Transfer() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
    // Show the new value of valueBlockB
    status = mfrc522.MIFARE_GetValue(valueBlockA, &value);
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("mifare_GetValue() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
    //Serial.print(F("New value of value block ")); Serial.print(valueBlockA);
    //Serial.print(F(" = ")); Serial.println(value);
    //Serial.print(F("Max Val of block: ")); Serial.print(valueBlockA);
    //Serial.print(F(" = ")); Serial.println(maxVal);
    // Check some boundary...
    if (value > maxVal) {
        //Serial.println(F("Below -100, so resetting it to 255 = 0xFF just for fun..."));
        status = mfrc522.MIFARE_SetValue(valueBlockA, resetVal);
        if (status != MFRC522::STATUS_OK) {
            //0Serial.print(F("mifare_SetValue() failed: "));
            //Serial.println(mfrc522.GetStatusCodeName(status));
            return status;
        }
    }
    return status;
}

MFRC522::StatusCode decrementBlockValueWithSet(int keyType, int32_t trailerBlock, int32_t valueBlockB, int32_t valDecr, int32_t lowerSide, int32_t valReset, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }

  if(keyType == 0)
  {
    //Serial.println(F("Authenticating again using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }
  else
  {
    //Serial.println(F("Authenticating again using key B..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        //Serial.print(F("PCD_Authenticate() failed: "));
        //Serial.println(mfrc522.GetStatusCodeName(status));
        return status;
    }
  }
  
  status = formatValueBlock(valueBlockB);
  if (status != MFRC522::STATUS_OK) {
      //Serial.print(F("PCD_Authenticate() failed: "));
      //Serial.println(mfrc522.GetStatusCodeName(status));
      return status;
  }
//Serial.print("Subtracting 10 from value of block "); Serial.println(valueBlockB);
  status = mfrc522.MIFARE_Decrement(valueBlockB, valDecr);
  if (status != MFRC522::STATUS_OK) {
      //Serial.print(F("MIFARE_Decrement() failed: "));
      //Serial.println(mfrc522.GetStatusCodeName(status));
      return status;
  }
  status = mfrc522.MIFARE_Transfer(valueBlockB);
  if (status != MFRC522::STATUS_OK) {
      //Serial.print(F("MIFARE_Transfer() failed: "));
      //Serial.println(mfrc522.GetStatusCodeName(status));
      return status;
  }
  // Show the new value of valueBlockB
  status = mfrc522.MIFARE_GetValue(valueBlockB, &value);
  if (status != MFRC522::STATUS_OK) {
      //Serial.print(F("mifare_GetValue() failed: "));
      //Serial.println(mfrc522.GetStatusCodeName(status));
      return status;
  }
  //Serial.print(F("New value of value block ")); Serial.print(valueBlockB);
  //Serial.print(F(" = ")); Serial.println(value);
  // Check some boundary...
  if (value <= lowerSide) {
      //Serial.println(F("Below -100, so resetting it to 255 = 0xFF just for fun..."));
      status = mfrc522.MIFARE_SetValue(valueBlockB, valReset);
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("mifare_SetValue() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  }
  return status;
}

MFRC522::StatusCode getValue(int keyType, int32_t trailerBlock, int32_t valueBlockB, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
  if(keyType == 0)
  {
  //Serial.println(F("Authenticating again using key A..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  }
  else
  {
    //Serial.println(F("Authenticating again using key B..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  }
  

  status = mfrc522.MIFARE_GetValue(valueBlockB, &value);
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("mifare_GetValue() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  return status;
}

MFRC522::StatusCode setValue(int keyType, int32_t trailerBlock, int32_t valueBlockB, int32_t val, MFRC522::MIFARE_Key key)
{
  MFRC522::StatusCode status = MFRC522::STATUS_OK;
  status = getCardInfo(keyType, key);
  if(status != MFRC522::STATUS_OK){
    return status;
  }
  if(keyType == 0)
  {
     MFRC522::StatusCode status = MFRC522::STATUS_OK;
    //Serial.println(F("Authenticating again using key A..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  }
  else
  {
     MFRC522::StatusCode status = MFRC522::STATUS_OK;
  //Serial.println(F("Authenticating again using key B..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("PCD_Authenticate() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  }
 

  status = mfrc522.MIFARE_SetValue(valueBlockB, val);
      if (status != MFRC522::STATUS_OK) {
          //Serial.print(F("mifare_GetValue() failed: "));
          //Serial.println(mfrc522.GetStatusCodeName(status));
          return status;
      }
  return status;
}

void makeToneStart()
{
   for(i=0;i<1;i++)
   {
    digitalWrite(buzzer,HIGH);
    delay(200);//wait for 1ms
    digitalWrite(buzzer,LOW);
    delay(200);//wait for 1ms
    }
}

void makeToneOK()
{
   for(i=0;i<10;i++)
   {
    digitalWrite(buzzer,HIGH);
    delay(50);//wait for 1ms
    digitalWrite(buzzer,LOW);
    delay(50);//wait for 1ms
    }
}


void makeToneError()
{
   for(i=0;i<80;i++)
   {
    digitalWrite(buzzer,HIGH);
    delay(2);//wait for 1ms
    digitalWrite(buzzer,LOW);
    delay(2);//wait for 1ms
    }
}

void makeBeep(byte beepType)
{
  if(beepType == 0)
  {
    makeToneStart();
  }
  else if(beepType == 1)
  {
    makeToneOK();
  }
  else
  {
    makeToneError();
  }
}

void turnOnLED()
{
  digitalWrite(led,LOW);
}

void turnOffLED()
{
  digitalWrite(led,HIGH);
}

void turnOnLED_RED()
{
  digitalWrite(led_red,LOW);
}

void turnOffLED_RED()
{
  digitalWrite(led_red,HIGH);
}

void turnOnLED_GREEN()
{
  digitalWrite(led_green,LOW);
}

void turnOffLED_GREEN()
{
  digitalWrite(led_green,HIGH);
}

void turnLedBegin()
{
  turnOffLED_RED();
  turnOffLED_GREEN();
  turnOnLED();
}

void turnLedSuccess()
{
  turnOffLED();
  turnOffLED_RED();
  turnOnLED_GREEN();
}

void turnLedFail()
{
  turnOffLED();
  turnOffLED_GREEN();
  turnOnLED_RED();
}

void turnOffAllLeds()
{
  delay(1000);
  turnOffLED();
  turnOffLED_GREEN();
  turnOffLED_RED();
}

void statusOfOperation(int state)
{
  if(state == 0)
  {
    turnLedBegin();
    makeBeep(0);
  }
  else if(state == 1)
  {
    turnLedSuccess();
    makeBeep(1);
    turnOffAllLeds();
  }
  else
  {
    turnLedFail();
    makeBeep(2);
    turnOffAllLeds();
  }
}


void to_String_X2(unsigned int v)
{
  v &= 0xff;
  memset(value_2, 0, sizeof(value_2));
  sprintf(value_2, "%02X", v);
}

void to_String_X4(unsigned int v)
{
  v &= 0xffff;
  memset(value_4, 0, sizeof(value_4));
  sprintf(value_4, "%04X", v);
}

void to_String_X8(unsigned long v)
{
  memset(value_8, 0, sizeof(value_8));
  memset(value_4, 0, sizeof(value_4));
  
  v &= 4294967295UL;
  low = (unsigned int)(v % 65536UL);
  high = (unsigned int)(v / 65536UL);
  
  sprintf(value_4, "%04X", high);
  strcat(value_8, value_4);
  
  memset(value_4, 0, sizeof(value_4));
  sprintf(value_4, "%04X", low);
  strcat(value_8, value_4);
}

char convertCharToHex(char ch)
{
  char returnType;
  switch(ch)
  {
    case '0':
    returnType = 0;
    break;
    case  '1' :
    returnType = 1;
    break;
    case  '2':
    returnType = 2;
    break;
    case  '3':
    returnType = 3;
    break;
    case  '4' :
    returnType = 4;
    break;
    case  '5':
    returnType = 5;
    break;
    case  '6':
    returnType = 6;
    break;
    case  '7':
    returnType = 7;
    break;
    case  '8':
    returnType = 8;
    break;
    case  '9':
    returnType = 9;
    break;
    case  'A':
    returnType = 10;
    break;
    case  'B':
    returnType = 11;
    break;
    case  'C':
    returnType = 12;
    break;
    case  'D':
    returnType = 13;
    break;
    case  'E':
    returnType = 14;
    break;
    case  'F' :
    returnType = 15;
    break;
    default:
    returnType = 0;
    break;
  }
  return returnType;
}

void stringToCharArray(char* hex, int len)
{
    s = 0;
    for (k = 0; k < len; k += 2) {
        hex[s] = (byte) (convertCharToHex(hex[k]) << 4 | convertCharToHex(hex[k+1]));
        s++;
    }
     hex[s] = 0;
}

void refresh_data()
{
  parserheader = 0;
  parserdataLength = 0;
  parserdirectionOfData = 0;
  //memset(parserdataMessage, 0, sizeof(parserdataMessage));
  parsercommand = 0;
  parserdate = {0, 0, 0, 0, 0, 0};
  parsercheckSum = 0;
  parserfooter = 0;
}

void parseMessage()
{
  refresh_data();
  parserheader = ((byte) message_to_server[0]);
  parserdataLength = (byte) message_to_server[2] + 256 * (byte) message_to_server[1];
  parserdirectionOfData = (byte) message_to_server[3];
  parserdate.years = (byte) message_to_server[4];
  parserdate.months = (byte) message_to_server[5];
  parserdate.days = (byte) message_to_server[6];
  parserdate.hours = (byte) message_to_server[7];
  parserdate.minutes = (byte) message_to_server[8];
  parserdate.seconds = (byte) message_to_server[9];
  parsercommand = (byte) message_to_server[11] + ((byte) message_to_server[10])*256; //((int) message_to_server[11] + (1 - (((int) message_to_server[11]) >= 0))*256) + (((int) message_to_server[10] + (1 - (((int) message_to_server[10]) >= 0))*256))*256;
  //Serial.write(parsercommand);
  //strncat(parserdataMessage,message_to_server + 12,parserdataLength);
  parsercheckSum = (((byte) message_to_server[12 + dataLength + 1]) + ((byte) message_to_server[12 + dataLength])*256); 
  parserfooter   = (byte) message_to_server[12 + dataLength + 2];
}

void gen_connection_message()
{
  dataLength = 18;
  directionOfData = 0;
  command = 1;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // time seconds
  to_String_X4(command);
  strcat(message_to_server, value_4);    // command 0x01 connection
  to_String_X2((int) 'S');
  strcat(message_to_server,value_2);  // dataType "SCTR01"
  to_String_X2((int) 'C');
  strcat(message_to_server,value_2);
  to_String_X2((int) 'R');
  strcat(message_to_server,value_2);
  to_String_X2((int) 'F');
  strcat(message_to_server,value_2);
  to_String_X2((int) 'M');
  strcat(message_to_server,value_2);
  to_String_X2((int) 'C');
  strcat(message_to_server,value_2);
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genResponseSuccess(int commandErr, byte cardUID[])
{
  dataLength = 24;
  directionOfData = 0;
  command = 200;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // time seconds
  to_String_X4(command);
  strcat(message_to_server, value_4);    // command 0x01 connection
  to_String_X4(commandErr);
  strcat(message_to_server, value_4);    // command 0x01 connection
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genResponseERR(int commandErr, int errCode, byte cardUID[])
{
  dataLength = 25;
  directionOfData = 0;
  command = 201;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // error command
  to_String_X4(command);
  strcat(message_to_server,value_4);
  to_String_X4(commandErr);
  strcat(message_to_server, value_4);    // command 0x01 connection
  to_String_X2(errCode);
  strcat(message_to_server,value_2);
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genReadBlockSuccess(int blkNum, byte readedData[], byte cardUID[])
{
  dataLength = 39;
  directionOfData = 0;
  command = 3;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  to_String_X2(blkNum);
  strcat(message_to_server,value_2);
  for(i=0;i<16;i++)
  {
    to_String_X2(readedData[i]);
    strcat(message_to_server,value_2);
  }
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genGetValueBlockSuccess(int blkNum, byte cardUID[])
{
  dataLength = 37;
  directionOfData = 0;
  command = 9;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  to_String_X2(blkNum);
  strcat(message_to_server,value_2);
  to_String_X8(value);
  strcat(message_to_server,value_8);
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genReadMultipleBlocksResponse(int numBlks, BlockStruct blocksArray[], byte cardUID[])
{
  dataLength = 23 + 17 * numBlks; 
  directionOfData = 0;
  command = 14;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  to_String_X2(numBlks);
  strcat(message_to_server,value_2);
  for(i=0;i<numBlks;i++)
  {
    to_String_X2(blocksArray[i].blkIndex);
    strcat(message_to_server,value_2);
    for(j=0;j<16;j++)
    {
      to_String_X2(blocksArray[i].blockData[j]);
      strcat(message_to_server,value_2);
    }
  }
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genReadCardInfoSuccess(CardInfoStruct cardInfoUID, byte cardUID[])
{
  dataLength = 33;
  directionOfData = 0;
  command = 15;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  to_String_X2(cardInfoUID.type);
  strcat(message_to_server, value_2);
  for(i=0;i<10;i++)
  {
    to_String_X2(cardInfoUID.cardUid[i]);
    strcat(message_to_server, value_2);
  }
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genResponseIncDecSuccess(int commandErr, byte cardUID[], byte val[])
{
  dataLength = 28;
  directionOfData = 0;
  command = 200;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // time seconds
  to_String_X4(command);
  strcat(message_to_server, value_4);    // command 0x01 connection
  to_String_X4(commandErr);
  strcat(message_to_server, value_4);    // command 0x01 connection
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X2(val[0]);
  strcat(message_to_server,value_2);
  to_String_X2(val[1]);
  strcat(message_to_server,value_2);
  to_String_X2(val[2]);
  strcat(message_to_server,value_2);
  to_String_X2(val[3]);
  strcat(message_to_server,value_2);
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genReadValueAsStrSuccess(byte val[], byte cardUID[])
{
  dataLength = 26;
  directionOfData = 0;
  command = 20;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  to_String_X2(val[0]);
  strcat(message_to_server, value_2);
  to_String_X2(val[1]);
  strcat(message_to_server, value_2);
  to_String_X2(val[2]);
  strcat(message_to_server, value_2);
  to_String_X2(val[3]);
  strcat(message_to_server, value_2);
  for(i=0;i<10;i++)
  {
    to_String_X2(cardUID[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}

void genSetBlkDecValueAsStrSuccess(byte val[], byte fee[], byte cardUID[])
{
  dataLength = 42;
  directionOfData = 0;
  command = 22;
  checkSumm = Header + dataLength + directionOfData + command + parserdate.years + parserdate.months + parserdate.days;
  
  memset(message_to_server, 0, sizeof(message_to_server));
  to_String_X2(255);
  strcat(message_to_server, value_2);    // Header
  to_String_X4(dataLength);
  strcat(message_to_server, value_4);    // DataLength & data = "SCTR01
  to_String_X2(directionOfData);
  strcat(message_to_server, value_2);    // Direction 0x00 (From Arduino To PC)
  to_String_X2(parserdate.years);
  strcat(message_to_server, value_2);    // time years
  to_String_X2(parserdate.months);
  strcat(message_to_server, value_2);    // time months
  to_String_X2(parserdate.days);
  strcat(message_to_server, value_2);    // time days
  to_String_X2(parserdate.hours);
  strcat(message_to_server, value_2);    // time hours
  to_String_X2(parserdate.minutes);
  strcat(message_to_server, value_2);    // time minuts
  to_String_X2(parserdate.seconds);
  strcat(message_to_server, value_2);    // command 0x01 connection
  to_String_X4(command);
  strcat(message_to_server, value_4);    // commandErr
  for(i=0;i<16;i++)
  {
    to_String_X2((int)val[i]);
    strcat(message_to_server,value_2);
  }
  to_String_X2((int)fee[0]);
  strcat(message_to_server,value_2);
  to_String_X2((int)fee[1]);
  strcat(message_to_server,value_2);
  to_String_X2((int)fee[2]);
  strcat(message_to_server,value_2);
  to_String_X2((int)fee[3]);
  strcat(message_to_server,value_2);
  for(i=0;i<10;i++)
  {
    to_String_X2((int)cardUID[2]);
    strcat(message_to_server,value_2);
  }
  to_String_X4(checkSumm);
  strcat(message_to_server,value_4);
  to_String_X2(Footer);
  strcat(message_to_server,value_2);
}