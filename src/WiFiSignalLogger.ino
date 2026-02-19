#include <WiFi.h>
#include "FS.h"
#include <LittleFS.h>
#include <SimpleFTPServer.h>
#define wifi WiFi

//--------------- HOW TO CONNECT -------------
//Using an FTP client (such as FileZilla, or most file explorers), type the boards local IP
// (shown on the serial monitor after it's connected to WiFi) and type
//username: esp32
//password: esp32

//I/O values
//constexpr for defining variable values at compile time
constexpr int buttonMeasurePoint = 15;
constexpr int buttonChangePlace = 22;
constexpr int buttonDelete = 23;
constexpr int LEDWrite = 21;
constexpr int LEDDelete = 19;

//WiFi Data and file name
//---------------- YOUR WIFI NETWORK HERE -------------
constexpr char* ssid = "YourSSID";
constexpr char* password = "YourPassword";
//-----------------------------------------------------


constexpr char* filename = "/Measurements.csv";
constexpr char* PlaceFile = "/Config.txt";

FtpServer ftpSrv; //FTP Server

//Aux vars
unsigned short pointCount = 0, placeCount, lineCount = 0;

//Exclusion of current file, and creation of a blank file
void deleteMeasurements(){
  digitalWrite(LEDDelete, HIGH);
  Serial.println("Deleting file...");
  LittleFS.remove(filename);
  if(!LittleFS.exists(filename)){
    Serial.println("File Deleted Sucessfully");
  }
  File file = LittleFS.open(filename, "w");
  file.println("Place, Point, Measure, Strength");
  file.close();
  LittleFS.remove(PlaceFile); //delete aux file for place definition
  digitalWrite(LEDDelete, LOW);

  //reset aux vars
  placeCount = 0;
  lineCount = 1;
  pointCount = 0;
  
  
}

// Point measure
void measurePoint(){

  //Getting the current place to measure
  File lastPlaceFile = LittleFS.open(PlaceFile, "r");
  if (lastPlaceFile) {
    placeCount = lastPlaceFile.parseInt();
    lineCount = lastPlaceFile.parseInt();
    lastPlaceFile.close();
  } else {
    // If file doesn't exist yet, start at 0
    placeCount = 0; 
    lineCount = 1;
  }

  //Signal measure
  digitalWrite(LEDWrite, HIGH);
  Serial.print("Measuring point ");
  Serial.print(pointCount);
  Serial.println("...");
  File file = LittleFS.open(filename, "a"); 

  if(!file){
    Serial.println("Couldn't save measure...");
    return;
  }

  float measure = WiFi.RSSI(); //Measured in dbm

  //Writing line to csv file
  file.print(placeCount);
  file.print(", ");
  file.print(pointCount);
  file.print(", ");
  file.print(measure);
  file.println(", ");
  digitalWrite(LEDWrite, HIGH);

  Serial.print("Measured sucessfully: ");
  Serial.println(measure);
  file.close();
  pointCount++; //next point ID

  File config = LittleFS.open(PlaceFile, "w");
  if (config) {
    config.print(placeCount);
    config.print(",");
    config.print(lineCount);
    config.print(",");
    config.print(pointCount); 
    config.close();
  }

  digitalWrite(LEDWrite, LOW);
}

//calculation of the arithmetic average of the current point,
//and change of place
void makeAvg(){

  //preventing divide by 0
  if (pointCount == 0) {
    Serial.println("No points measured. Cannot calculate average.");
    return;
  }

  //opening config file
  File config = LittleFS.open(PlaceFile, "r");
  if (config) {
    placeCount = config.parseInt();
    lineCount = config.parseInt();
    pointCount = config.parseInt();
    config.close();
  } else {
    // If file doesn't exist yet, start at 0
    placeCount = 0; 
  }

  //opening file for read
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading.");
    return;
  }
  digitalWrite(LEDWrite, HIGH);
  digitalWrite(LEDDelete, HIGH);
  float sum = 0;
  String line;

  Serial.print("Storing place ");
  Serial.print(placeCount);
  Serial.println("...");

  //go through file until the first measure of current place
  for(int i = 0; i< lineCount; i++){
    file.readStringUntil('\n');
  }

    //Power average:
  //1. sum = sum+ 10^(dbmvalue/10);
  //2. avg = sum/n (n = number of measurements);
  //3. powerAvg = 10 * log10(avg)

  //parsing of the values of current place
  for(int i = 0; i< pointCount; i++){
    line = file.readStringUntil('\n');
    int comma = line.indexOf(',');
    comma = line.indexOf(',',comma+1);
    int comma2 = line.indexOf(',',comma+1);
    double newValue = line.substring(comma+2, comma2).toDouble();
    sum = sum + pow(10,newValue/10.0); 
  }
  file.close();

  //storing average
  file = LittleFS.open(filename, "a");
  
  double avg = sum/(pointCount);
  double powerAvg = round(10*log10(avg));

  Serial.print("Place avg:");
  Serial.println(powerAvg);
  if(file){
    file.print(placeCount);
    file.print(", avg, ");
    file.print(powerAvg);
    file.print(", ");

    //classifying average
    if (powerAvg >= -40) file.println("Very Strong");
    else if (powerAvg >= -60) file.println("Strong");
    else if (powerAvg >= -70) file.println("Ok");
    else if (powerAvg >= -80) file.println("Weak");
    else if (powerAvg >= -90) file.println("Very Weak");
    else file.println("Unusable");
    file.close();
    Serial.println("Average stored sucessfully. Next place");
  }
  
  placeCount++;
  lineCount += (pointCount + 1);
  pointCount = 0;

  //updating config
  config = LittleFS.open(PlaceFile, "w");
  if (config) {
    config.print(placeCount);
    config.print(",");
    config.print(lineCount);
    config.print(",");
    config.print(pointCount); 
    config.close();
  }

  digitalWrite(LEDWrite, LOW);
  digitalWrite(LEDDelete,LOW);
}

void setup(){
  //serial and IO inicialization
  Serial.begin(115200);
  pinMode(buttonMeasurePoint, INPUT_PULLUP);
  pinMode(buttonChangePlace, INPUT_PULLUP);
  pinMode(LEDWrite,OUTPUT);
  pinMode(LEDDelete,OUTPUT);
  pinMode(buttonDelete, INPUT_PULLUP);

  //littleFS inicialization
  if(!LittleFS.begin()){
    Serial.println("Can't open LittleFS. Bye bye...");
    return;
  }

  //Wifi connection
  Serial.println("LittleFS opened sucessfully :)");
  WiFi.begin(ssid,password);

  int timeout_counter = 0;
  while (WiFi.status() != WL_CONNECTED && timeout_counter < 20) {
    delay(500);
    Serial.print(".");
    timeout_counter++;
  }
  if(WiFi.status() == WL_CONNECTED){
  Serial.println("\nConnected sucessfuly!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  }
  else{
    Serial.println("Falha ao conectar!");
    Serial.println(WiFi.status());
  }  
  ftpSrv.begin("esp32","esp32"); //ftp server

  //config file inicialization
  File config = LittleFS.open(PlaceFile, "r");
  if (config) {
    placeCount = config.parseInt(); // Saved Place ID
    lineCount = config.parseInt();  // Saved Line Marker
    pointCount = config.parseInt(); // Saved Point Count (Survives reboot!)
    config.close();
  } 
  else {
    placeCount = 0;
    lineCount = 1;
    pointCount = 0;
  }

  //Main file inicialization
  if(LittleFS.exists(filename)){
    Serial.println("There's an existing file. If you want to erase it at any moment, press the small button");  
  }
  else{
    File file = LittleFS.open("/TesteWifi.csv","a");
    if(file){
      Serial.println("File opened sucessfully!");
    }
    else{
    Serial.println("Can't open file...");
    return;
    }
    file.println("Place, Point, Measure, Strength");
    file.close();

    Serial.println("Operation:");
    Serial.println("- Press the white button to get a point measure");
    Serial.println("- Press the red button to change place");
  }
}

//loop with FTP handler and button handlers
void loop(){
  ftpSrv.handleFTP();

  if(digitalRead(buttonMeasurePoint) == LOW){
    measurePoint();
    delay(1000);
    
  }
  if(digitalRead(buttonChangePlace) == LOW){
    makeAvg();
    delay(1000);
  }
  if(digitalRead(buttonDelete) == LOW){
    deleteMeasurements();
    delay(1000);
  } 
  
}

