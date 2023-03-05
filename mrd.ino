#include <LiquidCrystal.h>

#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_DISTANCE_WIDTH // In case NEC is not received correctly. Universal decoder for pulse distance width protocols
#include <IRremote.hpp>

//const int rs = 12, en = 4, d4 = 5, d5 = 6, d6 = 3, d7 = 2;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int IR_RECEIVE_PIN = 2;
const int relayPin1 = 9;
const int relayPin2 = 10;
//int redPin= 7;
//int greenPin = 6;
//int bluePin = 5;
const int actuatorDelay = 2100;
//const int actuatorDelay = 1000;
const int breathDelay = 5000;
const int numCompressions = 30;
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;
enum Remote {
  ONE=0x16,
  TWO=0x19,
  THREE=0xD,
  FOUR=0xC,
  FIVE=0x18,
  SIX=0x5E,
  SEVEN=0x8,
  EIGHT=0x1C,
  NINE=0x5A,
  ZERO=0x52,
  OK=0x40,
  LEFT=0x44,
  RIGHT=0x43,
  UP=0x46,
  DOWN=0x15,
  STAR=0x42,
  POUND=0x4A
};

const Remote START_BUTTON = ONE;
const Remote PAUSE_BUTTON = THREE;
unsigned long startTime;
unsigned long duration;
unsigned long EXPECTED_DURATION = 60000; // 1 minute duration


void emptyIRBuffer() {
  while (IrReceiver.decode()) IrReceiver.resume();
  Serial.println("Cleared buffer");
}

boolean pollForButton(Remote button){
  Serial.print("Polling for button: ");
  Serial.println(button);
  return IrReceiver.decodedIRData.command == button;
}

void extendActuator() {
  Serial.println("Extending actuator");
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, LOW); 
  delay(actuatorDelay);
}

void retractActuator() {
  Serial.println("Retracting actuator");
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, HIGH); 
  delay(actuatorDelay);
}
void pauseActuator() {
  Serial.println("Pausing actuator");
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);
  delay(actuatorDelay);
}

// do chest compression
void doChestCompression() {
  Serial.println("Doing chest compression");
  extendActuator();
  pauseActuator();
  retractActuator();
  pauseActuator();
}

// give 2 breaths
void giveBreaths() {
  Serial.println("Giving 2 breaths");
//  printWords("Start breaths...");
//  playFrequency(200, 1000);
 delay(breathDelay);
}

void startTimer() {
  startTime = millis();
  Serial.println(0);
}

boolean timerCheck() {
  duration = millis() - startTime;
  Serial.print(duration/1000);
  Serial.println(" seconds elapsed");
  if (duration >= EXPECTED_DURATION) return true;
  return false;
}

//void playFrequency(int frequency, int duration){
//  int period = 1000000L/frequency;
//  int pulse = period;
//  for (long i = 0; i < duration; i += period) {
//    digitalWrite(speakerPin, HIGH);
//    delayMicroseconds(pulse);
//    digitalWrite(speakerPin, LOW);
//    delayMicroseconds(pulse);
//  }
//}
//
//void printWords(String word) {
//  lcd.setCursor(0, 1);
//  lcd.print(word);
//}


void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
//  pinMode(speakerPin, OUTPUT);
//  lcd.begin(16, 2);
//  lcd.print("Mr.D");
}


void loop() {
//  Serial.println("Starting");
  if (IrReceiver.decode()) {

    Serial.println("got button");
    if (IrReceiver.decodedIRData.command == START_BUTTON) {
      startTimer();
      emptyIRBuffer();
      
      while(!pollForButton(PAUSE_BUTTON)) {
        if (timerCheck()) break;
        // start chest compressions
        int counter = 0;
        while(counter < numCompressions) {
          if (timerCheck()) break;
           
          doChestCompression();          
          // poll for new input
          emptyIRBuffer();
          if (pollForButton(PAUSE_BUTTON)) break;
          counter++;
        }

        giveBreaths();

      }
      pauseActuator();
    }
    
   IrReceiver.resume(); // Enable receiving of the next value
  }
}
