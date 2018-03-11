//
// MIT License.
//

// Simple Arduino program to poll magnetometer vaues from an MPU9255
// AK8963 internal module.
// Refer to https://www.invensense.com/ for chip documentation.

#include <Wire.h>

#define    MPU_ADDRESS            0x68
#define    AKM_ADDRESS            0x0C

#define MPU_INTPINCFG_REGISTER 0x37
#define AKM_ST1_REGISTER 0x02
#define AKM_ST2_REGISTER 0x09
#define AKM_CTRL_REGISTER 0x0A
#define AKM_DATA0_REGISTER 0x03

struct MagnetometerData
{
  int16_t x;
  int16_t y;
  int16_t z;
};

// Read from I2C device from the address and register specified. 
void I2CRead(uint8_t Address, uint8_t Register, uint8_t ByteCount, uint8_t* Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
 
  // Perform the read.
  Wire.requestFrom(Address, ByteCount); 

  uint8_t index=0;
  while (Wire.available() && (index < ByteCount))
  {
      Data[index++] = Wire.read();
  }
}
// Writes ingle byte of data to I2C device using address and register
void I2CWriteByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}

void I2CWaitUntilMatch(uint8_t Address, uint8_t Register, uint8_t ExpectedBits)
{
  uint8_t ST1;
  do
  {
    I2CRead(Address, Register, 1, &ST1);
  }
  while (!(ST1 & ExpectedBits));
}

void ReadMagnetometerValues()
{
  // Set by-pass mode (bit1) to reach the magnetometer directly.
  uint8_t pinCfgState = 0x00;
  I2CRead(MPU_ADDRESS, MPU_INTPINCFG_REGISTER, 1, &pinCfgState);
  I2CWriteByte(MPU_ADDRESS, MPU_INTPINCFG_REGISTER, (pinCfgState | 0x02));

  // Assert the control register
  I2CWriteByte(AKM_ADDRESS, AKM_CTRL_REGISTER, 0x01);
 
  // Read register Status 1 and wait for Data Ready
  I2CWaitUntilMatch(AKM_ADDRESS, AKM_ST1_REGISTER, 0x1);

  uint8_t rawData[6];
  I2CRead(AKM_ADDRESS, AKM_DATA0_REGISTER, 6, rawData);

  // Read the Status 2 register to clear up Data Ready (DRDY) and Data Overrun (DOR)
  uint8_t ST2;
  I2CRead(AKM_ADDRESS, AKM_ST2_REGISTER, 1, &ST2);

  // Restore the by-pass mode bit to what it was before the function.
  I2CWriteByte(MPU_ADDRESS, MPU_INTPINCFG_REGISTER, pinCfgState);
  
  MagnetometerData compassData;
  compassData.x = (rawData[1] << 8 | rawData[0]);
  compassData.y = (rawData[3] << 8 | rawData[2]);
  compassData.z = (rawData[5] << 8 | rawData[4]);

  Serial.print(compassData.x, DEC);
  Serial.print("\t");
  Serial.print(compassData.y, DEC);
  Serial.print("\t");
  Serial.print(compassData.z, DEC);
  Serial.println("");
}

void setup()
{
  // Initialize I2C.
  Wire.begin();

  // Initialize Serial for debugging output.
  Serial.begin(115200);
}

void loop()
{
  ReadMagnetometerValues();
  Serial.println("");
  delay(2000);
}

