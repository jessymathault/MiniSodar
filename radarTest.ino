#define trigPin 11
#define sensePin A1
#define numberOfRangeBins 128

#define motorPin1 6
#define motorPin2 7
#define motorPin3 8
#define motorPin4 9



// Fast ADC ------------------------
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


// ----------------------------------

int delayTime = 100;
byte count = 0;


// Range sensor:
//  - trigger pin -> 11
//  - sense pin -> A0 *lowpass filter with 104 CAP and 10k res


unsigned long timestamp = 0;
unsigned long currentTime = 0;
//unsigned long timeArray[numberOfRangeBins];
int currentRangeBins[numberOfRangeBins];
bool targetVector[numberOfRangeBins];


// Options
bool videoIntegration = 0;
bool enableCFAR = 1;
bool debugMode = 0;
byte oversamplingFactor = 5;


// Target Detection Mode
//ratio of maximum
bool enableAGC = 0;
int targetDetectionTreshold = 700; // 1000 = max peak

// Standard deviation multiple
bool enableAdaptativeThreshold = 1;
float stdDevThreshold = 2.5;




int numberOfSamples = oversamplingFactor*numberOfRangeBins;

void setup() {
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(sensePin, INPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  // look for the next valid integer in the incoming serial stream:
  if (debugMode){
    while (Serial.available() > 0) {
      // look for the newline. That's the end of your
      // sentence:
      Serial.println("I'm here");
      if (Serial.read() == '\n') {
        measureRange();
      }
    }
  }
  
  scanFcn();
  measureRange();
}




void scanFcn(){
  if (count == 0){
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    count = 1;
  }
  else if (count == 1){
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    delay(delayTime);
    digitalWrite(motorPin2, LOW);
    count = 2;
  }
  else if (count == 2){
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
    delay(delayTime);
    digitalWrite(motorPin3, LOW);
    count = 3;
  }
  else if (count == 3){
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
    delay(delayTime);
    digitalWrite(motorPin4, LOW);
    count = 0;
  }
  
}

void measureRange(){
  bool samplingComplete = 0;
  unsigned long sampleTimingStart = 0;
  int binNumber = 0;
  int signalProccesingBuffer[numberOfRangeBins];
  int n = 0;
  int signalCopies = 0;
  int numberOfPulses = 1;

  // Options
  if (videoIntegration){
    numberOfPulses = 4;
  }

  // Reset range bins
  for (n=0; n<numberOfRangeBins; ++n){
    currentRangeBins[n] = 0; // 
  }
  // Reset buffer
  for (n=0; n<numberOfRangeBins; ++n){
    signalProccesingBuffer[n] = 0; // 
  }
  
  
  // SAMPLE pulses
  timestamp = micros();
  for (signalCopies=0; signalCopies<numberOfPulses; ++signalCopies){
    // Send pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    sampleTimingStart = micros();
    samplingComplete = 0;
    binNumber = 0;
    byte currentOverSampleNumber = 0;
    // rangeBin Sampling
    while (!samplingComplete){
      
      // Sample at a given interval
      currentTime = micros();
      if (currentTime - sampleTimingStart >= 30){
        // real 44.3 us per sample
        sampleTimingStart = micros();
        //timeArray[binNumber] = sampleTimingStart;
        if (currentOverSampleNumber < (oversamplingFactor-1)){
          signalProccesingBuffer[binNumber] = signalProccesingBuffer[binNumber] + analogRead(sensePin);
          currentOverSampleNumber = currentOverSampleNumber +1;
        }
        else{
          signalProccesingBuffer[binNumber] = signalProccesingBuffer[binNumber] + analogRead(sensePin);
          currentOverSampleNumber = 0;
          binNumber = binNumber+1;
        }
        
        if (binNumber == numberOfRangeBins){
          samplingComplete = 1;
          // once done, get out.
        }
      }
    // else wait
    }
    currentTime = micros();
    // Signal processing - integration
    for (n=0; n<numberOfRangeBins; ++n){
      if (videoIntegration){
        int temp = signalProccesingBuffer[n]>>2;
        currentRangeBins[n] = currentRangeBins[n] + temp; // 
      }
      else{
        currentRangeBins[n] = signalProccesingBuffer[n];
      }
    }
    if (videoIntegration){
      delayMicroseconds(500);
    }
  }

  
  // CFAR processing
  if (enableCFAR){
    // signalProcessingBuffer is the CFAR threshold
    for (n=4; n<numberOfRangeBins-4; ++n){
      signalProccesingBuffer[n] = currentRangeBins[n-4]+currentRangeBins[n-3]+currentRangeBins[n-2]+currentRangeBins[n-1]+currentRangeBins[n+1]+currentRangeBins[n+2]+currentRangeBins[n+3]+currentRangeBins[n+4]; // 
      signalProccesingBuffer[n] = signalProccesingBuffer[n]>>3;
    }
    // remove threshold from value
    for (n=0; n<numberOfRangeBins; ++n){
      if (n<4){
        currentRangeBins[n] = 0;
      }
      else if (n>numberOfRangeBins-4){
        currentRangeBins[n] = 0;
      }
      else{
        currentRangeBins[n] = currentRangeBins[n] - signalProccesingBuffer[n];
      }
    }
  }


  // AGC
  int gain = 1;
  if (enableAGC){
    int maxPeak = 0;
    for (n=0; n<numberOfRangeBins; ++n){
      if (currentRangeBins[n]>maxPeak){
        maxPeak = currentRangeBins[n];
      }
    }
    //Detection
    gain = 1000/maxPeak;
    if (videoIntegration){
      if (gain>50){
        gain =50;
      }
    }
    else{
      if (gain>25){
        gain =25;
      }
    }
    // Detect Targets
    for (n=0; n<numberOfRangeBins; ++n){
      targetVector[n] = ((currentRangeBins[n]*gain)>targetDetectionTreshold);
    }
  }
  else if(enableAdaptativeThreshold){
    float variance = 0;
    float temp = 0;
    float stdDev = 0;
    for (n=0; n<numberOfRangeBins; ++n){
      temp = (float) currentRangeBins[n];
      variance = variance + temp*temp; // 
    }
    variance = variance / numberOfRangeBins;
    stdDev = pow(variance, 0.5);
    stdDev = stdDev*stdDevThreshold;
    int threshold = (int)stdDev;
    for (n=0; n<numberOfRangeBins; ++n){
      targetVector[n] = ((currentRangeBins[n])>threshold);
    }
  }

  


  if (debugMode){
    // Send DATA
    //Serial.print("Timestamp: ");
    Serial.println("Start Sampling Time");
    Serial.println(timestamp);
    Serial.println("Stop Sampling Time");
    Serial.println(currentTime);
    

    Serial.print("Threshold,");
    Serial.print(targetDetectionTreshold);
    Serial.println("");
    
//    Serial.print("Time Bins,");
//    //Serial.println("];");
//    for (n=0; n<numberOfRangeBins; ++n){
//      Serial.print(timeArray[n]);
//      Serial.print(", ");
//    }
    Serial.println("");
    
    Serial.print("Range Bins,");
    for (n=0; n<numberOfRangeBins; ++n){
      Serial.print(currentRangeBins[n]);
      Serial.print(", ");
    }
    Serial.println("");
    Serial.print("Target Vector,");
    for (n=0; n<numberOfRangeBins; ++n){
      Serial.print(targetVector[n]);
      Serial.print(", ");
    }
    Serial.println("");
    //Serial.println("];");
  }

  // print after each 
  for (n=0; n<numberOfRangeBins; ++n){
    Serial.print(targetVector[n]);
    Serial.print(", ");
  }
  Serial.println("");
}

