#include <OneWire.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <DallasTemperature.h>
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <dht.h>
//#include "MPU9250.h"
#include "LedControlMS.h"

#define NumMatrix 1
#define DHT11_PIN 15
#define ONE_WIRE_BUS 4
#define REPORTING_PERIOD_MS 250
HardwareSerial hwSerial(1);
DFRobotDFPlayerMini dfPlayer;
LedControl lc=LedControl(12,11,10, NumMatrix);
int tempPiel=0;

PulseOximeter pox;
uint32_t tsLastReport = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
dht DHT;
const int GSR=34;
int sensorValue=0;
int gsr_average=0;

int playpause = 0;

//MPU9250 mpu = MPU9250();
// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print("\t");
  Serial.print("\t");
Serial.println("Beat!");
}

void setup()
{
 
 
 
 Serial.begin(115200);
 Serial.print("Initializing pulse oximeter..");

 if (!pox.begin()) {
  Serial.println("FAILED");
  for(;;);
 } else {
  Serial.println("SUCCESS");
  }
  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  hwSerial.begin(9600, SERIAL_8N1, 16, 17);  // speed, type, TX, RX
 dfPlayer.begin(hwSerial);  //Use softwareSerial to communicate with mp3
 dfPlayer.setTimeOut(500); //Set serial communication time out 500ms
 dfPlayer.volume(30);  //Set volume value (0~30).
 dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
 dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
 sensors.begin(); // Establece valor por defecto del chip 9 bit. Si tiene problemas pruebe aumentar
// uint8_t temp = mpu.begin();

}



void loop()
{

  // Make sure to call update as fast as possible
  pox.update();
  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
  Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print("\t");
  Serial.print("\t");  
  Serial.print("Heart rate:");
  Serial.print(pox.getHeartRate());
  Serial.print("bpm / SpO2:");
  Serial.print(pox.getSpO2());
  Serial.println("%");
  //Serial.print("Solicitando temperaturas...");
  sensors.requestTemperatures();  // Enviar el comando para obtener la temperatura
  //Serial.println("Hecho");
  //Serial.print("La temperatura en el dispositivo 1 es: ");
  Serial.println(sensors.getTempCByIndex(0));
  rGalvanic();
  tempHumedad();

  int valorPiel = sensors.getTempCByIndex(0);

  if(valorPiel >= 33){
    if(playpause == 1)
    {
      
      Serial.print("\t");
      Serial.print("\t");
      Serial.println("Next");
      dfPlayer.next();
      lc.writeString(0," Men por favor ve al doctor ");
      playpause = 0;
      
    }
         
  }
  else if(valorPiel >= 32)
  
  {
    if(playpause ==0)
    {
      
      Serial.print("\t");
      Serial.println("Play");
      dfPlayer.play(2);
      playpause = 1;
      lc.writeString(0," Tienes fiebre men ");
      
    }
  }
  else if(valorPiel == 31)
  {
    if(playpause == 1 || playpause == 2)
    {
      playpause = 0;
      Serial.print("\t");
      Serial.println("Stop");
      //mp3_stop ();
      
    }
  }
 
  tsLastReport = millis();
  }
  
 
// MPU9250sensor();
 
  //if(mpu.z_g >= 3 || mpu.z_g <= -3 || mpu.y_g >= 3 || mpu.y_g <= -3
  //||mpu.x_g >= 3 || mpu.x_g <= -3)
  //{
   // if(playpause ==0 || playpause ==1 )
    //{
     // Serial.print("\t");
     // Serial.println("Play");
      
      //dfPlayer.play(3);
      //lc.writeString(0," Fatality ");
      //playpause = 0;
    //}

//}


}

void tempHumedad(){
  
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("\t");
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("\t");
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  

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

//void MPU9250sensor(){
  
  //Accel
  //mpu.set_accel_range(RANGE_4G);
 // mpu.get_accel();
  
  //Serial.print("X: ");  Serial.print(mpu.x);
  //Serial.print(" Y: "); Serial.print(mpu.y);
  //Serial.print(" Z: "); Serial.print(mpu.z);
  
  //mpu.get_accel_g();
  //Serial.print("\t");Serial.print("\t");Serial.print("\t");Serial.print("\t");
  //Serial.print("\t");Serial.print("\t");
  //Serial.print(" X_g: "); Serial.print(mpu.x_g,2); 
 // Serial.print(" Y_g: "); Serial.print(mpu.y_g,2); 
 // Serial.print(" Z_g: "); Serial.print(mpu.z_g,2);  Serial.println(" G");

  //Gyro
 // mpu.set_gyro_range(RANGE_GYRO_250);
  //mpu.get_gyro();
  //Serial.print("GX: ");  Serial.print(mpu.gx); 
  //Serial.print(" GY: "); Serial.print(mpu.gy); 
  //Serial.print(" GZ: "); Serial.print(mpu.gz);

 // mpu.get_gyro_d();
  //Serial.print(" GX_g: "); Serial.print(mpu.gx_d,2); 
  //Serial.print(" GY_g: "); Serial.print(mpu.gy_d,2); 
  //Serial.print(" GZ_g: "); Serial.print(mpu.gz_d,2); Serial.println(" º/s");

  //Mag
  //mpu.set_mag_scale(SCALE_14_BITS);
  //mpu.set_mag_speed(MAG_8_Hz);
  //if(!mpu.get_mag()){
    //Serial.print("MX: ");  Serial.print(mpu.mx); 
    //Serial.print(" MY: "); Serial.print(mpu.my); 
    //Serial.print(" MZ: "); Serial.print(mpu.mz);

    //mpu.get_mag_t();
    //Serial.print(" MX_t: "); Serial.print(mpu.mx_t,2); 
    //Serial.print(" MY_t: "); Serial.print(mpu.my_t,2); 
    //Serial.print(" MZ_t: "); Serial.print(mpu.mz_t,2); Serial.println(" uT");
  //}
  //else{
    // |X|+|Y|+|Z| must be < 4912μT to sensor measure correctly 
    //Serial.println("Overflow no magnetometro.");
  //}
  
  // Temp     
  //Serial.print("Temperature is ");  Serial.print((((float) mpu.get_temp()) / 333.87 + 21.0), 1);  Serial.println(" degrees C");
  
  //delay(500);

//}
