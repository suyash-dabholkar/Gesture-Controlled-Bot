/*
 * Gesture Controlled Bot - NRF24L01 Transmitter
 * 
 * This code reads orientation data (Yaw, Pitch, Roll) from an MPU6050 IMU 
 * and transmits it to a receiver bot via an NRF24L01 wireless module.
 * 
 * Hardware:
 * - Arduino (Uno/Nano/Mega)
 * - MPU6050 (I2C)
 * - NRF24L01+ (SPI)
 */

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// --- Configuration ---
// #define PRINT_DEBUG   // Uncomment to enable Serial debugging

// MPU6050 control/status vars
MPU6050 mpu;
bool dmpReady = false;  
uint8_t devStatus;      
uint16_t packetSize;    
uint8_t fifoBuffer[64]; 

// Orientation data containers
Quaternion q;           
VectorFloat gravity;    
float ypr[3];           // [yaw, pitch, roll]

// RF24 configuration
const uint64_t pipeOut = 0xF9E8F0F0E1LL; // MUST MATCH THE RECEIVER ADDRESS
RF24 radio(8, 9); // CE, CSN pins

// Data structure for transmission
struct PacketData 
{
  byte xAxisValue; // Transmitted as 0-254 (127 = center)
  byte yAxisValue; // Transmitted as 0-254 (127 = center)
} data;

/**
 * Initializes the NRF24L01 module as a transmitter.
 */
void setupRadioTransmitter()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.stopListening(); 

  // Initialize with neutral values
  data.xAxisValue = 127; 
  data.yAxisValue = 127; 
}

/**
 * Initializes and calibrates the MPU6050 sensor.
 */
void setupMPU()
{
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock
  #endif

  #ifdef PRINT_DEBUG
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("Initializing MPU6050..."));
  #endif
  
  mpu.initialize();
  devStatus = mpu.dmpInitialize();
  
  if (devStatus == 0) 
  {
      // Automatic calibration: generates offsets based on current orientation
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      
      #ifdef PRINT_DEBUG      
        mpu.PrintActiveOffsets();
        Serial.println(F("Enabling DMP..."));
      #endif
      
      mpu.setDMPEnabled(true);
      dmpReady = true;
      packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else 
  {
      #ifdef PRINT_DEBUG       
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
      #endif
  }
}

void setup()
{
  setupRadioTransmitter();   
  setupMPU();
}

void loop() 
{
  if (!dmpReady) return;

  // Process the latest packet from the MPU6050 FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) 
  {  
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // Convert radians to degrees and constrain to ±90°
    int xAxisValue = constrain(ypr[2] * 180/M_PI, -90, 90);
    int yAxisValue = constrain(ypr[1] * 180/M_PI, -90, 90);

    // Map degree values to 0-254 range for transmission
    data.xAxisValue = map(xAxisValue, -90, 90, 0, 254); 
    data.yAxisValue = map(yAxisValue, -90, 90, 254, 0);

    // Send data over RF
    radio.write(&data, sizeof(PacketData));

    #ifdef PRINT_DEBUG  
      Serial.print("X: "); Serial.print(xAxisValue);
      Serial.print(" | Y: "); Serial.println(yAxisValue);        
    #endif
  }
}