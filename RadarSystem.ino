// Range sensor:
//  - trigger pin -> 11
//  - sense pin -> A0 * highpass filter with 104 CAP and 10k res
//  - sense pin -> A1 direct connection

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


// Global Variables ----------------------------------------
int samplingInterval = 30; // real 36.5 us per sample
unsigned long timestamp = 0;
unsigned long currentTime = 0;
int currentRangeBins[numberOfRangeBins];
bool targetVector[numberOfRangeBins];
bool PPIVector[numberOfRangeBins/2];
byte rangeBinVector[numberOfRangeBins];
int threshold = 0;
float formattedThreshold = 0;

// Signal Processing Options -------------------------------
bool videoIntegration = 0; // Enable mutiple return integration
bool enableCFAR = 1; // enable CFAR thresholding
byte oversamplingFactor = 5; // number of integrated samples within a single range bin


// Target Detection Mode ------------------------------------
//ratio of maximum
bool enableAGC = 0;
int targetDetectionTreshold = 700; // 1000 = max peak

// Standard deviation thresholding
bool enableAdaptativeThreshold = 1;
float stdDevThreshold = 2.5;


// Useful metrics
int numberOfSamples = oversamplingFactor*numberOfRangeBins;
float maxrange = ((float) oversamplingFactor * numberOfRangeBins * ((float) samplingInterval + 6.5)*343.0)/(2000000);



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
  
  // Read header to get command type                    -------------------------
   if (Serial.available() > 0) {
    char header[3];
    Serial.readBytes(header, 3);
    
    // Get Target Array command -> return targetVector
    if (strcmp(header, "gta")==0){ 
      char temp;
      for (byte n = 0; n<numberOfRangeBins; n++){
        temp = (char)targetVector[n]+48;
        Serial.print(temp);
      }
    }

    // Get PPI Array command -> return PPIVector
    if (strcmp(header, "gpi")==0){ 
      char temp;
      for (byte n = 0; n<numberOfRangeBins/2; n++){
        temp = (char)PPIVector[n] +48;
        Serial.print(temp);
      }
    }
    
    // Get Formatted Bins command -> return threshold + currentRangeBins -------------------------
    else if (strcmp(header, "gfb")==0){  
      char temp;
      temp = (char) formattedThreshold +48;
      Serial.print(temp);
      for (byte n = 0; n<numberOfRangeBins; n++){
        temp = (char)rangeBinVector[n]+48;
        Serial.print(temp);
      }
    }

    // Get Range Bins command -> return threshold + currentRangeBins -------------------------
    else if (strcmp(header, "grb")==0){  
      Serial.print(threshold);
      Serial.print(", ");
      for (byte n = 0; n<numberOfRangeBins; n++){
        Serial.print(currentRangeBins[n]);
        Serial.print(", ");
      }
      Serial.println("");
    }

    // Start Range Measure -> update targetVector and range bins with values -----
    else if (strcmp(header, "srm")==0){ 
      Serial.print("srmOK");
      measureRange();
    }
    else{
      Serial.print("??");
    }
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
  int maxPeak = 0;
  for (n=0; n<numberOfRangeBins; ++n){
    if (currentRangeBins[n]>maxPeak){
      maxPeak = currentRangeBins[n];
    }
  }

  // AGC
  int gain = 1;
  if (enableAGC){
    threshold = targetDetectionTreshold;
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
      targetVector[n] = ((currentRangeBins[n]*gain)>threshold);
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
    threshold = (int)stdDev;
    for (n=0; n<numberOfRangeBins; ++n){
      targetVector[n] = ((currentRangeBins[n])>threshold);
    }
  }


  float factor = 64.0 / ((float) maxPeak);
  // Assign range bin values between 0 and 64
  formattedThreshold = (float) threshold * factor;
  for (n=0; n<numberOfRangeBins; ++n){
    int temp = 0;
    float value = 0;
    if (currentRangeBins[n]>0){
      value = (float) currentRangeBins[n];
      value = value * factor;
      rangeBinVector[n] = (byte) value; 
    }
    else{
      temp = 0;
    }
  }

  for (n=0; n<numberOfRangeBins/2; ++n){
    PPIVector[n] = targetVector[n*2] || targetVector[n*2+1];
  }
  
}

