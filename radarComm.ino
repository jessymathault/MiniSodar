
byte rangeBins[128];
bool targetArray[64];
byte counterRangeBins = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (byte n = 0; n<128; n++){
    rangeBins[n] = 0;
  }
  for (byte n = 0; n<64; n++){
    targetArray[n] = 0;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
   if (Serial.available() > 0) {
    char header[3];
    Serial.readBytes(header, 3);
    if (strcmp(header, "gta")==0){ // get target Array command
      Serial.print("gtaOK");
    }
    else if (strcmp(header, "grb")==0){ // get range bins command
      for (byte n = 0; n<128; n++){
        char temp = (char)rangeBins[n]+48;
        Serial.print(temp);
      }
      fillArray();
    }
    else if (strcmp(header, "gbc")==0){ 
      Serial.print("gbcOK");
    }
    else{
      Serial.print("??");
    }
   }
}

void fillArray(){
  // max value in the range bin must be 64!
  rangeBins[counterRangeBins] = counterRangeBins>>1;
  rangeBins[counterRangeBins-1] = 0;
  counterRangeBins = counterRangeBins + 1;
  if (counterRangeBins>126){
    counterRangeBins = 1;
  }
}

