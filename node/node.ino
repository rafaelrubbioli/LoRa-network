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
String ID = "";
bool hasJoined = false;
unsigned int id = 0;

String rssi = "RSSI --";
String packSize = "--";
String packet ;

void setup(){
	pinMode(Vext,OUTPUT);
	pinMode(LED,OUTPUT);

	digitalWrite(Vext, LOW);    // set GPIO16 low to reset OLED
	delay(50); 

	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI00);

	if (!LoRa.begin(BAND)){
		Serial.println("Falha incialização lora!");
		while (1);
	}
  Serial.println("LoRa iniciou com sucesso!");
	delay(1000);
}

void loraData(){
	Serial.println("Recebi " + packSize + "bytes\n"+ packet + " " + counter + "\n");
	counter++;
}

void cbk(int packetSize) {
	packet ="";
	packSize = String(packetSize,DEC);
	for (int i = 0; i < packetSize; i++) { 
		packet += (char) LoRa.read(); 
	}
	rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
	loraData();
}

void join() {
	LoRa.beginPacket();
	LoRa.print("JOIN|0|0|0");
	LoRa.endPacket();
  Serial.println("Enviando pacote JOIN");
	LoRa.receive();
	int packetSize = LoRa.parsePacket();
	if (packetSize){
		cbk(packetSize);
	  hasJoined = true;
    ID = String(packet);
	}
}

void loop(){
	if (hasJoined !=  true) {
		join();
		digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(1000);                       // wait 
		digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
		delay(1000);
	} 
	
	else {
    Serial.println("Enviando pacote:" + String(counter));
		//mede a temperatura
		uint8_t medida = (temprature_sens_read() - 32)/1.8; 
		// send packet
		LoRa.beginPacket();
		LoRa.print("MEASUREMENT|");
		LoRa.print(String(ID));
		LoRa.print("|");
		LoRa.print(counter);
		LoRa.print("|TEMP:");
		LoRa.print(medida);
		LoRa.endPacket();

		counter++;
		digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(900000);                       // wait for a second
		digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
		delay(900000); 					// wait for a second
	}                      
}
