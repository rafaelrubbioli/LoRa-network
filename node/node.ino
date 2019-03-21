#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 

#define SDA     4    // GPIO4  -- SX127x's SDA
#define SCK     5    // GPIO5  -- SX127x's SCK
#define SCL     15   // GPIO15 -- SX127X's SCL
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define RST_LED 16   // GPIO16 -- OLED reset pin
#define LED     25   // GPIO25 -- LED Light pin
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define BAND    915E6
#define PABOOST true

#define V2   1

#ifdef V2 //WIFI Kit series V1 not support Vext control
  #define Vext  21
#endif

// sensor interno de temperatura
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
// fim sensor de temperatura 

// sensor hall de campo magnetico
//    int measurement = 0;
//    measurement = hallRead();
//    Serial.print("Hall sensor measurement: ");
//    Serial.println(measurement); 
// fim sensor hall

unsigned int counter = 0;
bool hasJoined = false;
unsigned int id = 0;

SSD1306  display(0x3c, SDA, SCL, RST_LED);
String rssi = "RSSI --";
String packSize = "--";
String packet ;

void setup()
{
  pinMode(Vext,OUTPUT);
  pinMode(LED,OUTPUT);
  
  digitalWrite(Vext, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  delay(1500);
  display.clear();
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI00);
  
  if (!LoRa.begin(BAND,PABOOST))
  {
    display.drawString(0, 0, "Falha incialização lora!");
    display.display();
    while (1);
  }
  display.drawString(0, 0, "LoRa iniciou com sucesso!");
  display.display();
  delay(1000);
}

void loraData(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0 , 15 ,"Recebi - " + packSize + " bytes");
  display.drawStringMaxWidth(0 , 26 , 128, packet);
  display.drawString(0, 37, "Pacote recebido n: " + String(counter));
  display.drawString(0, 0, rssi);  
  display.display();

  Serial.print("Recebi " + packSize + "bytes\n"+ packet + " " + counter + "\n");
  counter++;
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  loraData();
}

void join() {
  LoRa.beginPacket();
  LoRa.print("JOIN REQUEST");
  LoRa.endPacket();

  LoRa.receive();
  int packetSize = LoRa.parsePacket();
  if (packetsize){
    cbk(packetSize);
  }
  
}

void loop()
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  if !hasJoined {
    join()
  }
  
  display.drawString(0, 0, "Enviando pacote: ");
  display.drawString(90, 0, String(counter));
  display.display();

  //mede a temperatura
  uint8_t medida = (temprature_sens_read() - 32)/1.8; 
  // send packet
  LoRa.beginPacket();
  LoRa.print("Pacote - ");
  LoRa.print(counter);
  LoRa.print(", medida: ");
  LoRa.print(medida);
  LoRa.endPacket();

  counter++;
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
