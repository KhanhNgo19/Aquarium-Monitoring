/*******************************************
 * Aquarium Monitoring
 * Arduino & ESP8266
 * Ngô Minh Khánh & Nguyễn Hữu Hưng
*******************************************/

/*            TÍNH NĂNG (Đã test, còn update)
  - Cho cá ăn bằng nút và giờ định sẵn nhấn trên điện thoại 
  - Đo nhiệt độ nước và hiện thị về điện thoai
*/
 
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BLYNK_PRINT Serial

OneWire oneWire(D2); // digital D2 pin
DallasTemperature sensors(&oneWire);
BlynkTimer timer;
Servo servo;


const int pos = 180;   // Goc  quay cua servo
int button; 
float temp = 0;

char auth[] = "TLSksyC6xuNuI-XsP8nfCG5xrRfg9172"; // Nhập AuthToken
char ssid[] = "Wifi free pass 1-8"; // Nhập tên WiFi
char pass[] = "hoilamdeogi";   // Nhập password WiFi


void setup()
{
    Serial.begin(9600);
    Blynk.begin(auth, ssid, pass);
    sensors.begin();
    timer.setInterval(1000L, sendTemps);
    servo.attach(5); // Pin D1 on NodeMCU ESP8266
    servo.write(0); // Goc 0 độ
}
  
BLYNK_WRITE(V1) 
{  
    button = param.asInt();
}  

void sendTemps()
{
 sensors.requestTemperatures();
 temp = sensors.getTempCByIndex(0);
 Serial.println(String("Temperature= ")+temp+ String(" C"));
 Blynk.virtualWrite(V2, temp);
}

void loop() 
{  
    Blynk.run();
    timer.run();  
    if ( button == 1 )
    {  
        servo.write(pos); 
       // led.on();
        delay(1000);
        servo.write(0);
    }  

}
