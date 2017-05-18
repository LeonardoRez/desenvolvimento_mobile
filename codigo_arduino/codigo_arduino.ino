#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

//credenciais para comunicação com broker
#define PORTA_MQTT 17093
#define USUARIO_MQTT "cdhwvxkk"
#define SENHA_MQTT "uoHN9SWskIFf"



#define TOPICO_LIGA_SISTEMA_IRRIGACAO "/sistema/irrigar"

#define TOPICO_MEDIR_TEMP "/sensor/temp/status"
#define TOPICO_ENVIAR_TEMP "/sensor/temp/refresh"

#define TOPICO_MEDIR_UMIAR "/sensor/umiar/status"
#define TOPICO_ENVIAR_UMIAR "/sensor/umiar/refresh"

#define TOPICO_MEDIR_UMISOLO "/sensor/umisolo/status"
#define TOPICO_ENVIAR_UMISOLO "/sensor/umisolo/refresh"

#define pino_umidade_solo A0
#define pino_temp_umidade_ar A1 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11


// Update these with values suitable for your network.
byte mac[]    = { 0xf8, 0xb1, 0x56, 0xfc, 0x4b, 0xbe };
char SERVER_ADDRESS[] = "m13.cloudmqtt.com";
char MENSAGEM_BROKER[100];

IPAddress ip(10, 0, 0, 98);
IPAddress server(34,200,51,91);
// Assinatura do metodo callback
void callback(char* topic, byte* payload, unsigned int length);

// Metodo para manipular mensagens recebidas do broker
void callback(char* topic, byte* payload, unsigned int length) 
{
  int i = 0;
  for(i=0; i<length; i++)
  {
    MENSAGEM_BROKER[i] = payload[i];
  }
  MENSAGEM_BROKER[i] = '\0';
  String topicoMensagem = String(topic);

  // Executa acao apropriada ao topico recebido
  if(topicoMensagem == TOPICO_LIGA_SISTEMA_IRRIGACAO)
  {
    irriga();
  }else if(topicoMensagem == TOPICO_MEDIR_TEMP)
  {
    medirTemp();
  }else if(topicoMensagem == TOPICO_MEDIR_UMIAR)
  {
    medirUmiAr();
  }else if(topicoMensagem == TOPICO_MEDIR_UMISOLO)
  {
    medirUmiSolo();
  }
  
}

EthernetClient ethClient;
PubSubClient client(SERVER_ADDRESS, PORTA_MQTT, callback, ethClient);

// Metodo para conexao com o broker
void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("HortaIngeligente", USUARIO_MQTT, SENHA_MQTT))
    {
      Serial.println("connected");
      
      client.subscribe(TOPICO_LIGA_SISTEMA_IRRIGACAO);
      client.subscribe(TOPICO_MEDIR_TEMP);
      client.subscribe(TOPICO_MEDIR_UMIAR);
      client.subscribe(TOPICO_MEDIR_UMISOLO);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}
int umidade_solo;
int porta_rele1 = 7;
DHT dht(pino_temp_umidade_ar, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  pinMode( pino_umidade_solo, INPUT);
  pinMode(porta_rele1, OUTPUT); 
  dht.begin();//inicializa o leitor de temperatura e umidade do ar
  
    // Inicializa conexao Ethernet
  
  Serial.println("Conectando na rede...");  
  //*
  if (Ethernet.begin(mac) == 0)
  { 
    Serial.println("Failed to configure Ethernet using DHCP");
    //for (;;);
    setup();
  }
  
  printIPAddress();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}

void irriga(){
   Serial.println("irrigando");
   digitalWrite(porta_rele1, HIGH);
   delay(40000);
   digitalWrite(porta_rele1, LOW);
}
void medirTemp(){
  
  Serial.println("medindo temp");
  float t = dht.readTemperature();
  char str_temp[6];

  /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
  dtostrf(t, 4, 2, str_temp);
  
  // testa se retorno e valido, caso contrario algo esta errado.
    if (isnan(t)) 
    {
      sprintf(MENSAGEM_BROKER, "Falha");
      client.publish(TOPICO_ENVIAR_TEMP, MENSAGEM_BROKER);
    } 
    else
    {
      Serial.print("temp: ");
      Serial.println(t);
      sprintf(MENSAGEM_BROKER, str_temp);    
      client.publish(TOPICO_ENVIAR_TEMP, MENSAGEM_BROKER);
    }
}
void medirUmiAr(){
  Serial.println("Medindo umidade do ar");
  float h = dht.readHumidity();
  // testa se retorno e valido, caso contrario algo esta errado.
    if (isnan(h)) 
    {
      sprintf(MENSAGEM_BROKER, "Falha");
      client.publish(TOPICO_ENVIAR_UMIAR, MENSAGEM_BROKER);
    } 
    else
    {
      char str_temp[6];

      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(h, 4, 2, str_temp);
      
      Serial.print("umidade ar: ");
      Serial.println(h);
      sprintf(MENSAGEM_BROKER, str_temp);
      client.publish(TOPICO_ENVIAR_UMIAR, MENSAGEM_BROKER);
    }
  
}
void medirUmiSolo(){
    Serial.println("medindo umidade do solo");
    umidade_solo = analogRead( pino_umidade_solo); 
    //Solo umido
    if (umidade_solo > 0 && umidade_solo < 400)
    {
      sprintf(MENSAGEM_BROKER, "Solo umido");
      client.publish(TOPICO_ENVIAR_UMISOLO, MENSAGEM_BROKER);
    }
    
    //Solo com umidade moderada
    if (umidade_solo > 400 && umidade_solo < 800)
    {
      sprintf(MENSAGEM_BROKER, "Umidade moderada");
      client.publish(TOPICO_ENVIAR_UMISOLO, MENSAGEM_BROKER);
    }
    
    //Solo seco
    if (umidade_solo > 800 && umidade_solo < 1024)
    {
      sprintf(MENSAGEM_BROKER, "Solo seco");
      client.publish(TOPICO_ENVIAR_UMISOLO, MENSAGEM_BROKER);
    }
    
      Serial.print("umidade solo: ");
      Serial.println(umidade_solo);
}
