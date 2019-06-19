#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
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
const char* ssid = "wifi";
const char* password = "senha";
const uint16_t port = 5000;
const char * host = "150.164.10.115";
WiFiClient client;

// LoRa
String rssi = "RSSI --";
String packSize = "--";
String packet;

// Protocol
unsigned int counter = 0;
  
// Parse lora packet
void loraData(){
	Serial.println("Recebi " + packSize + "bytes\n"+ packet + " " + counter + "\n");
	counter++;

	WiFiClient client;
	// send to wifi server
	if (!client.connect(host, port)) {
		Serial.println("Connection to host failed");
		delay(10);
		return;
	}
	Serial.println();
  	Serial.println("Repassando o pacote ao servidor...");
	client.print(packet);
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
	delay(1500);

	//inicializa o log no computador para debug
	Serial.begin(115200);
	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI00);

	// conexao wifi
	Serial.println("INICIALIZANDO");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(5000);
		Serial.println("Conectando ao WiFi...");
	}
	Serial.print("WiFi conectado com IP: ");
	Serial.println(WiFi.localIP());

	while (!client.connect(host, port)) {
		Serial.println("Conexão falhou");
		delay(100);
	}

	if (!LoRa.begin(BAND)) {
		Serial.println("Falha inicialização loRa!");
		while (1);
	}

	Serial.print("LoRa iniciou com sucesso!");
	Serial.print("Esperando dados... ");
	delay(3000);
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
	if (packet == "JOIN|0|0|0") {
		String response = "";
    		response = client.read();
		Serial.println(response);
		LoRa.print(response);
	}
  
}
