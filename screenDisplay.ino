

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

int nAngles = 10;
int nRangeBins = 6;

bool targetArray[10][6] = {{0,0,0,1,1,1},{0,1,0,0,1,1},{0,0,1,0,1,1},{0,0,0,1,1,1},{0,0,1,0,1,1},{0,0,0,0,0,1},{0,0,0,0,0,1},{0,0,0,0,0,1},{0,0,0,0,0,1},{0,0,0,0,0,1}};

byte targetBins[128];
byte rangeBins[129];
byte threshold;

byte mode = 1;

float angleStep = 3.1416/nAngles;
float rangeStep = 64/nRangeBins;


void setup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  Serial.begin(115200);
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Rodar");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,40);
  display.println("Jessy Mathault");
  display.display();
  delay(20);
  display.clearDisplay();
}

void loop() {
  char buffer1[5];
  
  if (mode ==0){ // A scope
    Serial.print("srm");
    Serial.readBytes(buffer1, 5);
    while((Serial.available() > 0)){
      int thisChar = Serial.read();
    }
    delay(300); 
    Serial.print("gfb");
    Serial.readBytes(rangeBins, 129);
    while((Serial.available() > 0)){
      int thisChar = Serial.read();
    }
    display.clearDisplay();
    displayAScope();
  }

    
  else if(mode ==1){ // target bins
    Serial.print("srm");
    Serial.readBytes(buffer1, 5);
    while((Serial.available() > 0)){
      int thisChar = Serial.read();
    }
    delay(300); 
    Serial.print("gta");
    Serial.readBytes(targetBins, 128);
    while((Serial.available() > 0)){
      int thisChar = Serial.read();
    }
    display.clearDisplay();
    displayTargetBins();
    }
    
}
  
  
  
//  drawBaseMap();
//  delay(200);
//  addTargets();
//  delay(200);
//  display.clearDisplay();
//  delay(200);
  
void displayTargetBins(){
  for (byte i=0; i<128; i+=1) {
    byte temp = targetBins[i+1]-48;
    display.drawLine(127-i, 0, 127-i, temp*50, WHITE);
  }
  display.display();
}

void displayAScope(){
  for (byte i=0; i<128; i+=1) {
    byte temp = rangeBins[i+1]-48;
    display.drawLine(127-i, 0, 127-i, temp, WHITE);
  }
  display.drawLine(0, (rangeBins[0]-48), 128, (rangeBins[0]-48), WHITE);
  display.display();
}

void displayTargets(){
  for (byte i=0; i<128; i+=1) {
    byte temp = (byte)targetBins[i+1]-48;
    display.drawLine(127-i, 0, 127-i, temp, WHITE);
  }
  display.drawLine(0, (rangeBins[0]-48), 128, (rangeBins[0]-48), WHITE);
  display.display();
}




void drawBaseMap(){
  display.drawCircle(display.width()/2, 0, 16, WHITE);
  display.drawCircle(display.width()/2, 0, 32, WHITE);
  display.drawCircle(display.width()/2, 0, 48, WHITE);
  display.drawCircle(display.width()/2, 0, 63, WHITE);
  display.display();
}

void addTargets(){
  int currentAngle = 0;
  int currentRange = 0;

  for (currentAngle = 0; currentAngle<nAngles; ++currentAngle){
    for (currentRange = 0; currentRange<nRangeBins; ++currentRange){
      if (targetArray[currentAngle][currentRange] == 1){
        int x = 0;
        int y = 0;
        float range = (float)currentRange;
        float angle = (float)currentAngle;
        float resultx = cos(angle*angleStep)*range*rangeStep+64;
        float resulty = sin(angle*angleStep)*range*rangeStep;
        x = (int) resultx;
        y = (int) resulty;

        display.fillCircle(x, y, 2, WHITE);
        
      }
    }
  }
  display.display();
}

