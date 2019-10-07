//Librerias usadas
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//#include "DHT.h"
#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <dht.h>

#define DHT11_PIN 15
#define OLED_SDA 19
#define OLED_SCL 18
#define ONE_WIRE_BUS 4
HardwareSerial hwSerial(1);
DFRobotDFPlayerMini dfPlayer;


Adafruit_SH1106 display(OLED_SDA, OLED_SCL);
OneWire oneWire(ONE_WIRE_BUS);
// Pasar la referencia a OneWire del
// Sensor de temperatura Dallas
DallasTemperature sensors(&oneWire);
int tempPiel=0;
int playpause = 0;
dht DHT;
const int GSR=34;
int sensorValue=0;
int gsr_average=0;

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00  };

//----------------------------coneccion con el wifi
const char* ssid = "WiFi-UAO";
const char* password =  "";
//----------------------------coneccion con el servidor
const String ip ="http://11.11.27.131:8080/";
//----------------------------variables globales
//esta es el tamaño de datos que va enviar el json
const size_t capacidad = JSON_OBJECT_SIZE(2) + 21*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(10);
int Conteo_conexiones =0;//los intentos de conexion a la red wifi

// Definir cliente NTP para obtener tiempo
WiFiUDP ntpUDP;//Network Time Protocol Protocolo de datagramas de usuario
NTPClient timeClient(ntpUDP);//Network Time Protocol Client
// Variables para guardar fecha y hora
String formato;//formato de datos
String fecha;//
String hora;//


void setup() {
  
  hwSerial.begin(9600, SERIAL_8N1, 16, 17);  // speed, type, TX, RX           
  Serial.begin(115200);
  while(!Serial);
  Serial.println("started");


  dfPlayer.begin(hwSerial);  //Use softwareSerial to communicate with mp3
  dfPlayer.setTimeOut(300); //Set serial communication time out 500ms
  dfPlayer.volume(30);  //Set volume value (0~30).
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  sensors.begin();

  //Usamos el metodo buscar_wifi para buscar el wifi
  buscar_wifi();

  //usamos el conteo de conecciones
  if(Conteo_conexiones<50){ //si se conecta en menos de 50 intentos me da la comprobacion y el ip del dispositivo
    Serial.println(""); Serial.println("WiFi conectado"); Serial.println(WiFi.localIP());    
  }else{//si no se coneta buscara de nuevo hasta lograrlo
      Serial.println("");Serial.println("Error de conexion");buscar_wifi(); }
  //Inicialice un NTPClient para obtener tiempo     
  timeClient.begin(); timeClient.setTimeOffset(-18000);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setTextColor(WHITE);


}

void loop() {
  if(WiFi.status()== WL_CONNECTED){//comprobamos que seguimos conectados ala red wifi
// Leemos la temperatura o la humedad toma alrededor de 250 milisegundos
  //resibimos un string que es array de chars que tambien es un objeto json a traves del metodo devolver_json
 String info = devolver_json(
    sensors.getTempCByIndex(0), //body temperature value
    gsr_average, //body resistenace value
    70, //body BPM value
    95.4, //body sp02 value
    DHT.temperature, //ambient temperature value
    DHT.humidity, //ambient humidity value
    5,//device acceleration ax
    0,//device acceleration ay
    22,//device acceleration az
    5.2,//device gyro gx
    0.0,//device gyro gy
    15.2,//device gyro gz
    5,//device magnetometer mx
    0,//device magnetometer my
    22,//device magnetometer mz
    234.5,//device pressure value
    950.3,//device altitude value
    45);//device temperature value  
  //le enviamos los datos al servidor 
  enviar_dato_cliente(info);
  }else{
    //Serial.println("Error in WiFi connection");  
    //Serial.println("Reeconectando:");  
    buscar_wifi();
    }
  
  Oled();
  Audio();
  tempHumedad();
  rGalvanic();
}

void buscar_wifi(){
  //mandamos el nombre de la red wifi y la contraseña
  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while(WiFi.status() !=WL_CONNECTED and Conteo_conexiones <50  ){ Conteo_conexiones++; delay(500); Serial.print("#"); }
  //cuando se conecte me dira la aprobacion y la ip
  if(WiFi.status() ==WL_CONNECTED){Conteo_conexiones==0;Serial.println("WiFi conectado"); Serial.println(WiFi.localIP());}}//fin buscar wifi
  //***************************************************************************************************************************************************************************
  String devolver_json(float v1, int v2, int v3, float v4, float v5, float v6, int v7, int v8, int v9, float v10, float v11,
  float v12, int v13, int v14, int v15, float v16, float v17,float v18){
  
  DynamicJsonDocument doc(capacidad);

  //le mandamos la fecha y la hora del metodo tiempo_actual
  doc["time"] = tiempo_actual();

  JsonObject body = doc.createNestedObject("body");

  JsonObject body_temperature = body.createNestedObject("temperature");
  body_temperature["value"] = v1;
  body_temperature["type"] = "float";
  body_temperature["unit"] = "°C";

  JsonObject body_resistance = body.createNestedObject("resistance");
  body_resistance["value"] = v2;
  body_resistance["type"] = "int";
  body_resistance["unit"] = "Ohm";

  JsonObject body_BPM = body.createNestedObject("BPM");
  body_BPM["value"] = v3;
  body_BPM["type"] = "int";
  body_BPM["unit"] = "Dimensionless";

  JsonObject body_SpO2 = body.createNestedObject("SpO2");
  body_SpO2["value"] = v4;
  body_SpO2["type"] = "float";
  body_SpO2["unit"] = "%";

  JsonObject ambient = doc.createNestedObject("ambient");

  JsonObject ambient_temperature = ambient.createNestedObject("temperature");
  ambient_temperature["value"] = v5;
  ambient_temperature["type"] = "float";
  ambient_temperature["unit"] = "°C";

  JsonObject ambient_humidity = ambient.createNestedObject("humidity");
  ambient_humidity["value"] = v6;
  ambient_humidity["type"] = "float";
  ambient_humidity["unit"] = "%";

  JsonObject device = doc.createNestedObject("device");
  device["UUID"] = "550e8400-e29b-41d4-a716-446655440000";

  JsonObject device_acceleration = device.createNestedObject("acceleration");

  JsonObject device_acceleration_ax = device_acceleration.createNestedObject("ax");
  device_acceleration_ax["value"] = v7;
  device_acceleration_ax["type"] = "int";
  device_acceleration_ax["unit"] = "mg";

  JsonObject device_acceleration_ay = device_acceleration.createNestedObject("ay");
  device_acceleration_ay["value"] = v8;
  device_acceleration_ay["type"] = "int";
  device_acceleration_ay["unit"] = "mg";

  JsonObject device_acceleration_az = device_acceleration.createNestedObject("az");
  device_acceleration_az["value"] = v9;
  device_acceleration_az["type"] = "int";
  device_acceleration_az["unit"] = "mg";

  JsonObject device_gyro = device.createNestedObject("gyro");

  JsonObject device_gyro_gx = device_gyro.createNestedObject("gx");
  device_gyro_gx["value"] = v10;
  device_gyro_gx["type"] = "float";
  device_gyro_gx["unit"] = "deg/s";

  JsonObject device_gyro_gy = device_gyro.createNestedObject("gy");
  device_gyro_gy["value"] = v11;
  device_gyro_gy["type"] = "float";
  device_gyro_gy["unit"] = "deg/s";

  JsonObject device_gyro_gz = device_gyro.createNestedObject("gz");
  device_gyro_gz["value"] = v12;
  device_gyro_gz["type"] = "float";
  device_gyro_gz["unit"] = "deg/s";

  JsonObject device_magnetometer = device.createNestedObject("magnetometer");

  JsonObject device_magnetometer_mx = device_magnetometer.createNestedObject("mx");
  device_magnetometer_mx["value"] = v13;
  device_magnetometer_mx["type"] = "int";
  device_magnetometer_mx["unit"] = "mG";

  JsonObject device_magnetometer_my = device_magnetometer.createNestedObject("my");
  device_magnetometer_my["value"] = v14;
  device_magnetometer_my["type"] = "int";
  device_magnetometer_my["unit"] = "mG";

  JsonObject device_magnetometer_mz = device_magnetometer.createNestedObject("mz");
  device_magnetometer_mz["value"] = v15;
  device_magnetometer_mz["type"] = "int";
  device_magnetometer_mz["unit"] = "mG";

  JsonObject device_pressure = device.createNestedObject("pressure");
  device_pressure["value"] = v16;
  device_pressure["type"] = "float";
  device_pressure["unit"] = "mb";

  JsonObject device_altitude = device.createNestedObject("altitude");
  device_altitude["value"] = v17;
  device_altitude["type"] = "float";
  device_altitude["unit"] = "m";

  JsonObject device_temperature = device.createNestedObject("temperature");
  device_temperature["value"] = v18;
  device_temperature["type"] = "float";
  device_temperature["unit"] = "°C";

  //char info[1200];
  String info;
  serializeJson(doc, info);
  //Serial.println(info);
  return info;
 
  }
//***************************************************************************************************************************************************************************
String tiempo_actual(){
  while(!timeClient.update()) { timeClient.forceUpdate(); }
  // Necesitamos extraer fecha y hora
  formato = timeClient.getFormattedDate();
  //aqui obtenemos la fecha
  int splitT = formato.indexOf("T");
  fecha = formato.substring(0, splitT);
  //aqui obtenemos la hora
  hora = formato.substring(splitT+1, formato.length()-1);
  String fecha_hora = fecha+" "+ hora ;
  return fecha_hora;}//fin de tiempo_actual
  //***************************************************************************************************************************************************************************
  void enviar_dato_cliente(String info){
  HTTPClient http;   
  //Especificar destino para solicitud HTTP
  http.begin(ip);  
  //Especificar encabezado de tipo de contenido
  http.addHeader("Content-Type", "application/json");
  //Enviar la solicitud POST real
  int httpResponseCode = http.POST(info);    
  if(httpResponseCode>0){//comprobamos la solicitud fue aceptada
 //Obtenga la respuesta a la solicitud
  String response = http.getString();  
  Serial.println("");
  Serial.println("Codigo de verificacion de post");
  Serial.println(httpResponseCode);   //Imprimir código de retorno si es posivito llego bien
  //Serial.println(response);           //Imprimir solicitud de respuesta es lo que devuelvo para verificar que si llego como era
  }else{ 
  Serial.print("Error al enviar POST: "); Serial.println(httpResponseCode);// si el -1 no conecto al servidor
    }http.end();  //liberamos los recursos
  }//fin de enviar_dato_cliente

void tempHumedad(){
  
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("\t");
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("\t");
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);

}

void Oled(){
    
    sensors.requestTemperatures();  // Enviar el comando para obtener la temperatura
    Serial.println(sensors.getTempCByIndex(0));
    int valorPiel = sensors.getTempCByIndex(0);
      
    //display.clearDisplay();   
    display.clearDisplay();                                //Clear the display
    display.drawBitmap(90, 0, logo3_bmp, 30, 30, WHITE);    //Draw the second picture (bigger heart)
    // display temperature
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Temperature: ");
    display.setTextSize(2);
    display.setCursor(0,10);
    display.print(valorPiel);
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");

    // display humidity
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Humidity: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(DHT.humidity);
    display.print(" %");
    display.display(); 
    
    if(valorPiel >= 38 || valorPiel == -127) {  
      display.invertDisplay(true);   
    }else{  
      display.invertDisplay(false);    
    }
    display.display(); 
     
  }

void Audio(){
  
      sensors.requestTemperatures();
      int valorPiel = sensors.getTempCByIndex(0);
      if(valorPiel >= 40){
      if(playpause == 1 || playpause == 0)
      {
      
        Serial.print("\t");
        Serial.print("\t");
        Serial.println("Next");
        dfPlayer.play(2);
        playpause = 0;
      
      }      
    }
    else if(valorPiel >= 38)
  
    {
      if(playpause ==0)
      {
      
        Serial.print("\t");
        Serial.println("Play");
        dfPlayer.play();
        playpause = 1;
      }
    }
    else if(valorPiel == 36)
    {
      if(playpause == 1 || playpause == 2)
      {
        playpause = 0;
        Serial.print("\t");
        Serial.println("Stop");
        //mp3_stop ();
      
      }
    }
    
}

void rGalvanic(){
  long sum=0;
  for(int i=0;i<10;i++)           //Average the 10 measurements to remove the glitch
      {
      sensorValue=analogRead(GSR);
      sum += sensorValue;
      
      }
   gsr_average = sum/10;
   Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print("\t");
   Serial.println(gsr_average);
}


  
