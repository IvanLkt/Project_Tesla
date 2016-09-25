#include <dht11.h>
#include <RFID.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Nextion.h> 


//Реле
int relePin = 5;
boolean Rele = HIGH;


//Датчик температуры-влажности
dht11 DHT;               // Объявление переменной класса dht11
#define DHTPin 4         // Датчик DHT11 подключен к цифровому пину номер 4
float Temperature = 0.0;


/* RFID:
* MOSI: Pin 11 / ICSP-4
* MISO: Pin 12 / ICSP-1
* SCK: Pin 13 / ISCP-3
* SS: Pin 10
* RST: Pin 9
*/
#define SSPin 10
#define RSTPin 9
RFID rfid(SSPin, RSTPin);
unsigned char reading_card[5]; //for reading card
unsigned char master[5] = {123,178,168,133,228}; // allowed card
unsigned char mipt[5] = {103,249,86,180,124}; // allowed card
unsigned char i;
boolean StatusCard = LOW;

SoftwareSerial nextion(2, 3);// Nextion TX to pin 2 and RX to pin 3 of Arduino
Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps

boolean StatusStart = LOW;

void setup()
{
  Serial.begin(9600);
  SPI.begin(); 
  rfid.init();
  pinMode(relePin, OUTPUT);
  myNextion.init();
  analogReference(INTERNAL);
  digitalWrite(relePin, HIGH);
}

void rfidCheck()
{
  if (rfid.isCard()) 
    {
        if (rfid.readCardSerial()) 
        {
                /* Reading card */
                Serial.println(" ");
                Serial.println("Card found");
                Serial.println("Cardnumber:"); 
                for (i = 0; i < 5; i++)
                {     
                  Serial.print(rfid.serNum[i]);
                  Serial.print(" ");
                  reading_card[i] = rfid.serNum[i];
                }
                Serial.println();
                //verification
                for (i = 0; i < 5; i++)
                {
                  if (reading_card[i]!=mipt[i])
                  {
                    break;
                  }
                }
                if (i == 5)
                {
                  rfid_allow();
                }
                else
                {
                  rfid_denied();
                }
         } 
    }
  else
    {
      myNextion.init();
    }
    rfid.halt();  
}


void rfid_allow()
{
  Serial.println("Allow");
  myNextion.setComponentText("t2", "Access is allow");
  myNextion.setComponentText("b1", "OK");
  StatusCard = HIGH;
  
}


void rfid_denied()
{
  Serial.println("Denied");
  myNextion.init();
  StatusCard = LOW;
}


int Dht()
{  
  //Датчик температуры-влажности
  int chk;   
  // Мониторинг ошибок
  chk = DHT.read(DHTPin);    // Чтение данных
  switch (chk){
  case DHTLIB_OK:  
    break;
  case DHTLIB_ERROR_CHECKSUM:
    //Serial.println("Checksum error, \t");
    break;
  case DHTLIB_ERROR_TIMEOUT:
    //Serial.println("Time out error, \t");
    break;
  default:
    //Serial.println("Unknown error, \t");
    break;
  } 
  // Выводим показания влажности и температуры
  //Serial.print("Humidity = ");
  //Serial.print(DHT.humidity, 1);
  //Serial.print(", Temp = ");
  //Serial.println(DHT.temperature,1);


  Temperature = DHT.temperature;
}


void loop()
{ 
  Dht();
  //Serial.println(Temperature);
  delay(50);
   
  String message = myNextion.listen(); //check for message
  if(message != "")
  { // if a message is received...
    Serial.println(message); //...print it out
  }
  if(message == "65 0 2 1 ffff ffff ffff") // открывается страница аутентификации
  {
     delay(3000);
     rfidCheck();
     delay(50);
  }
    
     if (message == "65 1 3 1 ffff ffff ffff")
     {
        delay(2000);
        myNextion.setComponentText("b0", "GO");
     }
  
  if (message == "65 2 2 1 ffff ffff ffff")
  {
    myNextion.setComponentText("t0", String(Temperature));
    StatusStart = HIGH;
    
  }

  if (StatusStart && StatusCard)
  {
    digitalWrite(relePin, LOW);
    myNextion.setComponentText("t0", String(Temperature));
    delay(100);    
    Serial.println("OK");
  }

  if (message == "65 3 1 1 ffff ffff ffff")
  {
    digitalWrite(relePin, HIGH); 
    StatusCard = LOW;
    StatusStart = LOW;
    myNextion.setComponentText("t0", "TESLA");
  }
}
