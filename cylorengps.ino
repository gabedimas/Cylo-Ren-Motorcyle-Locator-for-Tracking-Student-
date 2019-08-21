#include <TinyGPS.h> 
#include <SoftwareSerial.h> 
#include <WifiEsp.h> 
#include <WifiEspClient.h> 
#include <WifiEspUdp.h> 
#include <PubSubClient.h> 
#include <GPSFilter.h> 
 
#define BUFFER_SIZE 100 
 
const char *mqtt_server = "m10.cloudmqtt.com";
const int mqtt_port = 11776;
const char *mqtt_user = "xoiihasx"; 
const char *mqtt_pass = "0FJGv4i9u9kR"; 
const char *mqtt_client_name = "arduinoClient1"; 
char ssid[] = "bismillah"; 
char pass[] = "24juli95"; 
int status = WL_IDLE_STATUS; 
 
WifiEspClient espClient; 
PubSubClient client(espClient, mqtt_server, mqtt_port);  
 
TinyGPSPlus gps; 
SoftwareSerial GPSSerial(11,10); 
SoftwareSerial ESP(52,53); 
unsigned long lastUpdate, lastSentence; 
float flat, flon; 
double c, ulat, ulong, dlat, dlong;
int year; 
byte month, day, hour, minute, second; 
 
void setup(){   
  Serial.begin(9600); 
  ESP.begin(9600);
  Wifi.init(&ESP);   
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to WPA SSID: ");    
    Serial.println(ssid);     
    status = Wifi.begin(ssid, pass); 
  } 
  Serial.println("You're connected to the network"); 
  Serial.println(); 
} 
 
//kalman filter void loop(){ 
  //Get Location 
  GPSSerial.begin(9600);  
  if (gps.location.isValid()){   
    flat = gps.location.lat();   
    flon = gps.location.lng();   
    GPSFilter kalman(25);   
    ulat = (double)flat;    
    ulong = (double)flon;   
    c = (millis() - lastSentence)/1000;    
    kalman.update_velocity2d(ulat,ulong,c);   
    lastSentence = millis();   
    if((millis() - lastUpdate) >= 500 || lastUpdate == NULL){       kalman.get_lat_long(&dlat,&dlong); 
      //Serial.print("Lat/Long K: "); 
      printFloat(dlat, 7); 
      Serial.print(", "); 
      printFloat(dlong,7); 
      lastUpdate = millis(); 
    } 
  } 
  smartDelay(1000); 
  if (millis() > 5000 && gps.charsProcessed() < 10){ 
    Serial.println(F("Can't get location")); 
  } 
 
  //Send Data to MQTT   ESP.begin(9600);     
  if (Wifi.status() == WL_CONNECTED) { 
    if (!client.connected()) { 
      Serial.println("Connecting to MQTT server"); 
      if (client.connect(MQTT::Connect(mqtt_client_name).set_auth(mqtt_user, mqtt_pass))){ 
        Serial.println("Connected to MQTT server"); 
        Serial.println(); 
      } 
      else { 
        Serial.println("Could not connect to MQTT server");    
      }  
      }   
      if (client.connected())    
      client.loop();   }  
  SendData(); 
  //Serial.println(); 
  delay(5000);  
} 
//send data lat/lon (location kalman) to MQTT void SendData(){ 
  Serial.print("Lat/Lon : ");
  Serial.print(flat,7);
  Serial.print(", ");
  Serial.print(flon,7);  
  Serial.print("Lat/Lon : ");
  Serial.print(dlat,7);
  Serial.print(", ");
  Serial.print(dlong,7); 
  Serial.print("/");Serial.print("D_15_DD"); 
  Serial.println(); 
  String temp=String(dlat,7)+(", ")+String(dlong,7)+("/")+String("D_15_DD");   
  client.publish("Location",temp); 
} 
 
static void smartDelay(unsigned long ms){  
  unsigned long start = millis();   do  
  { 
    while (GPSSerial.available())       gps.encode(GPSSerial.read()); 
  } while (millis() - start < ms); 
} 
 
void printFloat(double number, int digits){   if (number < 0.0){      Serial.print('-'); 
     number = -number; 
  } 
  double rounding = 0.5;   for (uint8_t i=0; i<digits; ++i)     rounding /= 10.0;     number += rounding;    unsigned long int_part = (unsigned long)number;   double remainder = number - (double)int_part;   Serial.print(int_part);   if (digits > 0)     Serial.print(".");    while (digits-- > 0){     remainder *= 10.0;     int toPrint = int(remainder);     Serial.print(toPrint);     remainder -= toPrint; 
  } 
  return number; 
} 

