
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// --- Configuration ---
#define SIGNAL_TIMEOUT 500  // Failsafe timeout in milliseconds

const uint64_t pipeIn = 0xF9E8F0F0E1LL; // MUST MATCH THE TRANSMITTER ADDRESS
RF24 radio(8, 9); 
unsigned long lastRecvTime = 0;

struct PacketData
{
  byte xAxisValue;    
  byte yAxisValue;
} receiverData;

// Right Motor
const int enableRightMotor = 5; 
const int rightMotorPin1 = 2;
const int rightMotorPin2 = 3;

// Left Motor
const int enableLeftMotor = 6;
const int leftMotorPin1 = 4;
const int leftMotorPin2 = 7;

/**
 * Sets motor direction and speed.
 * @param rightMotorSpeed -255 to 255
 * @param leftMotorSpeed  -255 to 255
 */
void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  // Right Motor Direction
  if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);    
  } else if (rightMotorSpeed > 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);      
  } else {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);      
  }
  
  // Left Motor Direction
  if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);    
  } else if (leftMotorSpeed > 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);      
  } else {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);      
  }  

  // Set Speed (PWM)
  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor, abs(leftMotorSpeed));    
}

void setup()
{
  // Initialize Motor Pins
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  rotateMotor(0, 0); 
    
  // Initialize Radio
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening(); 

  #ifdef PRINT_DEBUG
    Serial.begin(115200);
    Serial.println("Receiver Initialized. Waiting for data...");
  #endif
}

void loop()
{
    int rightMotorSpeed = 0;
    int leftMotorSpeed = 0;

    // Check for incoming RF data
    if (radio.isChipConnected() && radio.available())
    {
      radio.read(&receiverData, sizeof(PacketData)); 
      
      // Map 0-254 received values back to -255 to 255 range
      int mappedYValue = map(receiverData.yAxisValue, 0, 254, -255, 255); 
      int mappedXValue = map(receiverData.xAxisValue, 0, 254, -255, 255); 
      
      int motorDirection = (mappedYValue < 0) ? -1 : 1;
    
      // Differential Drive Logic
      rightMotorSpeed = abs(mappedYValue) - mappedXValue;
      leftMotorSpeed = abs(mappedYValue) + mappedXValue;
  
      // Clamp values to valid PWM range
      rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
      leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);
      
      rotateMotor(rightMotorSpeed * motorDirection, leftMotorSpeed * motorDirection);

      lastRecvTime = millis();  
      
      #ifdef PRINT_DEBUG  
        Serial.print("X: "); Serial.print(receiverData.xAxisValue);
        Serial.print(" | Y: "); Serial.println(receiverData.yAxisValue);      
      #endif
    }
    else
    {
      // Failsafe: Stop motors if signal is lost for more than SIGNAL_TIMEOUT
      unsigned long now = millis();
      if (now - lastRecvTime > SIGNAL_TIMEOUT) 
      {
        rotateMotor(0, 0);   
      }
    }
}