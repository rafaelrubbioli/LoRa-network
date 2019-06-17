#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include <WiFi.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define SDA     4    // GPIO4  -- SX127x's SDA
#define SCL     15   // GPIO15 -- SX127X's SCL
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define RST_LED 16   // GPIO16 -- OLED reset pin
#define LED     25   // GPIO25 -- LED Light pin
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define PABOOST true
#define V2   1
#ifdef V2 //WIFI Kit series V1 not support Vext control
  #define Vext  21
#endif

// Conexao WiFi
const char* ssid = "Alexa";
const char* password = "";
const uint16_t port = 5000;
const char * host = "192.168.0.103";
WiFiClient client;

// Display
SSD1306  display(0x3c, SDA, SCL, RST_LED);

// LoRa
String rssi = "RSSI --";
String packSize = "--";
String packet;

// Protocol
unsigned int counter = 0;

// display string on board
void displayScreen(String tobedisplayed){
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	display.drawString(0 , 15 ,tobedisplayed);
	display.display();
}
  
// Parse lora packet
void loraData(){
	Serial.print("Recebi " + packSize + "bytes\n"+ packet + " " + counter + "\n");
	counter++;

	WiFiClient client;
	// send to wifi server
	if (!client.connect(host, port)) {
		Serial.println("Connection to host failed");
		delay(10);
		return;
	}
	Serial.println("Repassando o pacote ao servidor...");
	Serial.println();
	client.print(packet);

	// display on board
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	display.drawString(0 , 15 ,"Recebi - " + packSize + " bytes");
	display.drawStringMaxWidth(0 , 26 , 128, packet);
	display.drawString(0, 37, "Pacote recebido n: " + String(counter));
	display.drawString(0, 0, rssi);  
	display.display();
}

void cbk(int packetSize) {
	packet ="";
	packSize = String(packetSize,DEC);
	for (int i = 0; i < packetSize; i++) { 
		packet += (char) LoRa.read();
	}
	rssi = "RSSI " + String(LoRa.packetRssi(), DEC);
	loraData();
}

void setup() {
	pinMode(Vext,OUTPUT);
	digitalWrite(Vext, LOW);    // set GPIO16 low to reset OLED
	delay(50); 
	display.init();
	display.flipScreenVertically();  
	display.setFont(ArialMT_Plain_10);
	delay(1500);

	displayScreen("Inicializando");

	//inicializa o log no computador para debug
	Serial.begin(115200);

	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI00);

	// conexao wifi
	Serial.println("INICIALIZANDO");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(5000);
		displayScreen("Conectando ao WiFi...");
		Serial.println("Conectando ao WiFi...");
	}
	Serial.print("WiFi conectado com IP: ");
	Serial.println(WiFi.localIP());

	//NÃO ESTA FUNCIONANDO 
	while (!client.connect(host, port)) {
		Serial.println("Conexão falhou");
		displayScreen("Conexão falhou!");
		delay(100);
	}

	if (!LoRa.begin(BAND,PABOOST)) {
		displayScreen("Falha inicialização loRa!");
		while (1);
	}

	Serial.print("LoRa iniciou com sucesso!");
	Serial.print("Esperando dados... ");
	displayScreen("LoRa iniciou com sucesso!");
	delay(2000);
	displayScreen("Esperando dados...");
	delay(1000);
	//LoRa.onReceive(cbk);
	LoRa.receive();
}

void loop() {
	int packetSize = LoRa.parsePacket();
	if (packetSize) { 
		cbk(packetSize);  
	}

	delay(10);
	client.print(packet);
	if packet.startsWith("JOIN") {
		response = client.recieve();
	}
}
