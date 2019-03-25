#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include &lt;WiFi.h&gt;

// conexao WiFi
const char* ssid = "snoopy";
const char* password = "xupeta01";
const uint16_t port = 5000;
const char * host = "192.168.1.83";
WiFiClient client;

// Pin definetion of WIFI LoRa 32
// HelTec AutoMation 2017 support@heltec.cn 
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

SSD1306  display(0x3c, SDA, SCL, RST_LED);
String rssi = "RSSI --";
String packSize = "--";
String packet ;

unsigned int counter = 0;

void loraData(){
  // send to wifi server
  if (!client.connect(host, port)) {
   Serial.println("Connection to host failed");
   delay(1000);
    return;
  }
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

void setup() {
  // conexao wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("conectando ao wifi...");
  }
  Serial.print("WiFi conectado com IP: ");
  Serial.println(WiFi.localIP());
  
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
  delay(1500);
  display.clear();

  //inicializa o log no computador para debug
  Serial.begin(115200);
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI00);
  
  if (!LoRa.begin(BAND,PABOOST)) {
    display.drawString(0, 0, "Falha incialização lora!");
    display.display();
    while (1);
  }
  Serial.print("LoRa iniciou com sucesso!");
  Serial.print("Esperando dados... ");
  display.drawString(0, 0, "LoRa iniciou com sucesso!");
  display.drawString(0, 10, "Esperando dados...");
  display.display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);  }
  delay(10);
}
