// Nano datalogger example
// Made by Ethan Grey

// Include needed libraries
#include <Wire.h> // Used for I2C connection for the clock
#include <SPI.h> // Used for SPI connection for the SD card
#include <SD.h> // Used for interacting with the SD card

// Define I2C pins
// Make sure to connect a 10k resister to 5v for SCL and SDA individually as this is required for I2C
#define SCL A5 // Clock line for I2C
#define SDA A4 // Data line for I2C
//Define Clock address
#define clockAddr 0b1101000 // The address for the clock is binary 1101000, 0b signifies a binary number
// Define the time
#define MONTH 5 // The month to set the clock to
#define DATE 6 // The date to set the clock to
#define YEAR 18 // The year to set the clock to (00-99)
#define DOW 1 // Day of the week to set the clock to
#define HOURS 21 // Hour to set the clock to (in 24hr clock)
#define MINUTES 05 // Minute to set the clock to
#define SECONDS 00 // Second to set the clock to

// Define SPI pins
#define SD_CS 10 // SD card chip select pin for SPI
#define MOSI 11 // Master Out Slave In pin for SPI
#define MISO 12 // Master In Slave Out pin for SPI
#define SCK 13 // Serial Clock pin

bool sd = true; // Defaults to having an SD card inserted, if SD cannot be initalized then it will be ignored

void setup() {
  // Initalize needed busses
  Serial.begin(9600); // Initialize serial for display purposes
  Wire.begin(); // Initialize I2C
  SetupClock(MONTH, DATE, YEAR, DOW, HOURS, MINUTES, SECONDS); // Set the clock date, run this once and then reupload with it commented out
                                                               // If left uncommented, the clock will always start at that specific date when the arduino gets rebooted
  
  SPI.begin(); // Initialize SPI
  if(!SD.begin(SD_CS)) // Initialize SD
    {
      Serial.println("SD failed to initialize!"); // If it couldn't be initialized, then print an error message
      sd = false;
    }
  
  pinMode(SD_CS,OUTPUT); // Set the SD card chip select pin as an output
}

void loop() {
  String date = GetDate(); // Get the date from the clock in text form
  
  if(sd) // If we have an sd card
  {
    File sdData = SD.open("data.txt", FILE_WRITE); // open data.txt so that we can write to it
    if(sdData)
      {
        sdData.println(date); // Print the date to the data.txt on the sd card
        sdData.close(); // Close the file to save the changes
      }
    else
      {
        Serial.println("Couldn't open file from SD card"); // If we couldn't open the file on the sd card, print an error
      }
  }
  
  Serial.println(date); // Print the date to the serial monitor
  delay(1000); // Wait a second between reads
}

void WriteI2C(byte addr, byte reg, byte data) // Writes data to the register of the device
{
  Wire.beginTransmission(addr); // Begin communication with the device
  Wire.write(reg); // Specify the register to write to
  Wire.write(data); // Write the data
  Wire.endTransmission(); // Stop sending to the device
}

void SetupClock(byte month, byte date, byte year, byte dow, byte hour, byte minute, byte second) // Set the time for the clock
{
  WriteI2C(clockAddr, 0x06, year); // Write the year to the clock
  WriteI2C(clockAddr, 0x05, month); // Write the year to the clock
  WriteI2C(clockAddr, 0x04, date); // Write the year to the clock
  WriteI2C(clockAddr, 0x03, dow); // Write the year to the clock
  WriteI2C(clockAddr, 0x02, hour); // Write the year to the clock
  WriteI2C(clockAddr, 0x01, minute); // Write the year to the clock
  WriteI2C(clockAddr, 0x00, second); // Write the year to the clock
}

byte BCDtoDecimal(byte bcd) // Converts a 1 byte binary coded decimal number to its equivalent decimal number
{
  byte tens = (bcd & 0b11110000) >> 4; // Get the tens digit
  byte ones = bcd & 0b00001111; // Get the ones digit
  byte result = (tens * 10) + ones; // Add them to get the decimal equivalent
  return result;
}

byte GetOneByte(byte addr, byte reg) // Since all clock data is only 1 byte long, we only need to request 1 byte of data at a time
// addr is the address of the device and reg is the register that the data is held in
{  
  Wire.beginTransmission(addr); // Begin communication with the device
  Wire.write(reg); // Ask for the data in the specified register
  Wire.endTransmission(); // Stop sending to the device

  Wire.requestFrom(addr, 1); // Request 1 byte of data from the device
  while(Wire.available() == 0); // Wait until there is data being sent
  byte data = Wire.read(); // Read the data being recived

  return data;
}
  
  
void GetClockTime(byte data[7]) // Uses a 7 byte array to store the clock data
{
  // From 0-6, the order is Year, Month, Date, Day of the week, Hours, Minutes, Seconds
  data[0] = GetOneByte(clockAddr,0x06); // Get Year
  data[1] = GetOneByte(clockAddr,0x05); // Get Month
  data[2] = GetOneByte(clockAddr,0x04); // Get Date
  data[3] = GetOneByte(clockAddr,0x03); // Get Day of the week
  data[4] = GetOneByte(clockAddr,0x02); // Get Hours
  data[5] = GetOneByte(clockAddr,0x01); // Get Minutes
  data[6] = GetOneByte(clockAddr,0x00); // Get Seconds
}

String GetDate() // Returns a string of the date
{
  byte clockData[7]; // Create an array to hold the data from the clock
  GetClockTime(clockData); // Get the data from the clock
  
  String date;
  for (byte c = 0; c < 7 ; c++) // For each section of data...
    {
      date = date + String(BCDtoDecimal(clockData[c])); // Add each part onto the end of date, after being converted from BCD to decimal
      if (c != 6) // If it isn't the last part of the data, put a ':' after to separate values
        {
          date = date + ':'; 
        }
    }
  return date;
}

