/**************************************************************
  - Bài tập môn học Cơ sở đo lường & điều khiển số -
  
  Chủ đề    : Aquarium Monitoring.
  Kit/MCU   : ESP8266.
  Thành viên: Ngô Minh Khánh  - 18020698 (nhóm trưởng)
              Nguyễn Hữu Hưng - 18020612
              
***************************************************************
                  TÍNH NĂNG 
             
   DONE: Liên kết ESP8266 vs FireBase
         Liên kết FireBase vs App
         Cho cá ăn bằng nút nhấn trên đt
         Gửi nhiệt độ và khoảng cách đến đt
         Đo nhiệt độ, khoản  g cách
         Điểu khiển sục oxy bằng nút nhấn trên điện thoại
         Sục Oxy 
         Tính năng hẹn giờ cho cá ăn và sục oxy
         Hoàn thiện thành hình cho sản phẩm
         Sưởi nước 
         
   TO DO: Thuyết trình

***************************************************************/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <FirebaseArduino.h>  // ArduinoJson version 5
#include <Servo.h>

const String tenWifi = "Wifi free pass 1-8";
const String mkWifi = "hoilamdeogi";
const String urlFirebase = "notesreact-e32ff.firebaseio.com";
const String secretFirebase = "dOSs0Sev5HZT9eZjQeAWFJkzuPWdJ8TGOWQlEkx8";

const int DS18B20_PIN = 4; //D2
const int TRIG_PIN = 14;   //D5
const int ECHO_PIN = 12;   //D6
const int PUMP_PIN = 13;   //D7
const int HEATER_PIN = 5; //D1
const int SERVO_PIN = 0;   //D3

OneWire oneWire(DS18B20_PIN); // digital D2 pin
DallasTemperature sensors(&oneWire);
Servo servo;
WiFiUDP u;
NTPClient n(u,"2.vn.pool.ntp.org",7*3600);

int chieuCaoBe = 20;
float temperature;
int oxyTimer[4] = {0, 0, 0, 0};
int anTimer[4] = {0, 0, 0, 0};
int hours;
int minutes;
int nhietDoMin;

void setup()
{
    Serial.begin(9600);
    ketNoiWifi();
    ketNoiFirebase();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(HEATER_PIN, HIGH);
    sensors.begin();
    servo.attach(SERVO_PIN); //D3
    servo.write(0); // dat o goc 0 do
    int preHours = Firebase.getInt("Hours");
    int preMinutes = Firebase.getInt("Minutes");
    oxyTimer[2] = preHours;
    oxyTimer[3] = preMinutes;
    anTimer[2] = preHours;
    anTimer[3] = preMinutes;
    
}

void loop() 
{  
    timer();
    oxy();
    suoi();
    choAn();
    mucNuoc();
    printNhietDo();
    Serial.println(" ------------------------ ");
}

void printNhietDo() {
    Serial.println(String(" Nhiet do: ")+ temperature + String(" °C"));
    Serial.println(String(" Nhiet do min: ")+ nhietDoMin + String(" °C"));
}

void suoi() {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    Firebase.setFloat("Nhiet do", temperature);
    String t1 = Firebase.getString("Nhiet do min");
    nhietDoMin = abc(t1);
  
    Serial.print(" Suoi: ");
    if( (temperature < nhietDoMin) || (temperature < 18) ){
        Serial.println("\"ON\"");
        digitalWrite(HEATER_PIN, LOW);
        Firebase.setString("Suoi", "\"ON\"");
    }
    if( (temperature > nhietDoMin) || (temperature == nhietDoMin) || (temperature > 30) ) {
        Serial.println("\"OFF\"");
        digitalWrite(HEATER_PIN, HIGH); 
        Firebase.setString("Suoi", "\"OFF\"");
    }
    
}

void choAn(){
    String status = Firebase.getString("An");
    if(status == "\"ON\"") { 
        servo.write(180); 
        delay(1000);
        servo.write(0);
        Firebase.setString("An", "\"OFF\"");
    }
  /*
  if(status == "\"ON\"") { 
     servo.write(180); 
     if(countAn[0] == 0)
         countAn[0] = millis();
     else 
         countAn[1] = millis();
  }
  if( (millis() - countAn[0]) > 1000 ) {
     servo.write(0);
     Firebase.setString("An", "\"OFF\"");
     countAn[0] = 0;
  }
  */
    if(status == "\"OFF\"") { 
        servo.write(0); 
    }
    Serial.print(" Cho an: ");
    Serial.println(status);
   
}

void mucNuoc() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    const unsigned long duration= pulseIn(ECHO_PIN, HIGH);
    double distance = duration/29/2;
    distance = chieuCaoBe - distance;
    Firebase.setFloat("Muc nuoc", distance);
    if(duration==0){
        Serial.println("Warning: no pulse from sensor");
    } 
    else{
        Serial.print(" Muc nuoc: ");
        Serial.print(distance);
        Serial.println(" cm");
    }
}

void oxy() {
    String status = Firebase.getString("Oxy");
    if(status == "\"ON\"") {
        digitalWrite(PUMP_PIN, LOW);
    }
    if(status == "\"OFF\"") {
        digitalWrite(PUMP_PIN, HIGH);
    }
    Serial.print(" Oxy: ");
    Serial.println(status);
}

void currentTime() {
    n.update();
    hours = n.getHours();
    minutes = n.getMinutes();
    Firebase.setInt("Hours", hours);
    Firebase.setInt("Minutes", minutes); 
}

void timer() {
    currentTime();
    Serial.println(String(" Time: ") + hours + String(":") + minutes);
    String oxyTimerEN = Firebase.getString("Oxy Timer_EN");
    int oxyTimerH = Firebase.getString("Oxy Timer_H").toInt();
    int oxyTimerM = Firebase.getString("Oxy Timer_M").toInt();
    //Serial.print(" O:");
    //printArr(oxyTimer);
    swapArr(oxyTimer);
    oxyTimer[2] = hours;
    oxyTimer[3] = minutes;
    if(oxyTimerEN == "true") {
        if ( (hours==oxyTimerH) && (minutes==oxyTimerM) ) {
            if( (oxyTimer[0]!=oxyTimer[2]) || (oxyTimer[1]!=oxyTimer[3]) ) {
                Firebase.setString("Oxy", "\"ON\""); 
            }
        }
    }
    //Serial.print(" O:");
    //printArr(oxyTimer);

    String anTimerEN = Firebase.getString("An Timer_EN");
    int anTimerH = Firebase.getString("An Timer_H").toInt();
    int anTimerM = Firebase.getString("An Timer_M").toInt();
    //Serial.print(" A:");
    //printArr(anTimer);
    swapArr(anTimer);
    anTimer[2] = hours;
    anTimer[3] = minutes;
    if(anTimerEN == "true") {
        if ( (hours==anTimerH) && (minutes==anTimerM) ) {
            if( (anTimer[0]!=anTimer[2]) || (anTimer[1]!=anTimer[3]) ) {
                Firebase.setString("An", "\"ON\""); 
             }
        }
    }
    //Serial.print(" A:");
    //printArr(anTimer);
}

void swapArr(int a[]) {
    int t1 = a[0];
    a[0] = a[2];
    a[2] = t1;

    int t2 = a[1];
    a[1] = a[3];
    a[3] = t2;
}

int abc(String a) {
    int n = a.length();
    char char1[n];
    strcpy(char1, a.c_str());

    a = "";
    for(int i = 1; i < n-1; i++) {
        a += char1[i];
    }
    return a.toInt();
}

void printArr(int a[]) {
    Serial.print(" ");
    for(int i = 0; i < 4; i++) {
        Serial.print(a[i]);
        Serial.print("|");
    }
    Serial.println("");   
}

void ketNoiWifi() {
    Serial.println(" Bat dau ket noi wifi");
    WiFi.begin(tenWifi,mkWifi);
    while(!(WiFi.status() == WL_CONNECTED)) {
        delay(500);
        Serial.print("...");
    }
  
    Serial.println("");
    Serial.println(" Ket noi thanh cong") ;
}

void ketNoiFirebase() {
    Firebase.begin(urlFirebase,secretFirebase);
}
