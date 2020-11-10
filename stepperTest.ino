int motorPin1 = 6;
int motorPin2 = 7;
int motorPin3 = 8;
int motorPin4 = 9;
int delayTime = 100;
int delayTime2 = 300;

int nStep = 0;

void setup() {
  Serial.begin(115200);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
}

void loop() {

  for (int n = 0; n<25; n++){
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin1, LOW);
    delay(delayTime);
    digitalWrite(motorPin1, HIGH);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin2, LOW);
    delay(delayTime);
    digitalWrite(motorPin2, HIGH);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin3, LOW);
    delay(delayTime);
    digitalWrite(motorPin3, HIGH);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin4, LOW);
    delay(delayTime);
    digitalWrite(motorPin4, HIGH);
    delay(delayTime);

  }

  for (int n = 0; n<25; n++){
    digitalWrite(motorPin4, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin1, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin4, LOW);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin4, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin3, LOW);
    delay(delayTime);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin4, LOW);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin2, LOW);
    delay(delayTime);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin1, HIGH);
    nStep =nStep +1;
    //Serial.println(nStep);
    delay(delayTime2);
    digitalWrite(motorPin1, LOW);
    delay(delayTime);

  }





}

