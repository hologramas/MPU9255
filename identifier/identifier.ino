//
// MIT License.
//

// Simple Arduino program to detect if an MPU9250 or 
// an MPU9255 is connected to the board via I2C and if the
// MPU is equiped with an AK8963 Magnotometer.
// Refer to https://www.invensense.com/ for chip documentation.

#include <Wire.h>

#define    MPU_ADDRESS            0x68
#define    AKM_ADDRESS            0x0C

#define MPU_WHO_AM_I_REGISTER 0x75
#define MPU_INTPINCFG_REGISTER 0x37
#define AKM_DEVICEID_REGISTER 0x00

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

uint8_t IdentifyMPUModel()
{
  uint8_t mpuId = 0;
  I2CRead(MPU_ADDRESS, MPU_WHO_AM_I_REGISTER, 1, &mpuId);

  switch (mpuId)
  {
    case 0x71:
      Serial.println("MPU9250 identified over I2C");
      break;

    case 0x73:
      Serial.println("MPU9255 identified over I2C");
      break;

    case 0x00:
      Serial.println("No response over I2C");
      break;

    default:
      Serial.print("Unknown MPU id obtained over I2C: 0x");
      Serial.println(mpuId, HEX);
      break;
  }

  return mpuId;
}

uint8_t IdentifyAKMMagnetometer()
{
  // Set by-pass mode (bit1) to reach the magnetometer directly.
  uint8_t pinCfgState = 0x00;
  I2CRead(MPU_ADDRESS, AKM_DEVICEID_REGISTER, 1, &pinCfgState);
  I2CWriteByte(MPU_ADDRESS, MPU_INTPINCFG_REGISTER, (pinCfgState | 0x02));
  
  uint8_t AKMId = 0;
  I2CRead(AKM_ADDRESS, AKM_DEVICEID_REGISTER, 1, &AKMId);

  switch (AKMId)
  {
    case 0x48:
      Serial.println("AK8963 Magnetometer identified over I2C");
      break;

    case 0x00:
      Serial.println("No response over I2C");
      break;

    default:
      Serial.print("Unknown Magnetometer id obtained over I2C: 0x");
      Serial.println(AKMId, HEX);
      break;
  }

  // Restore the by-pass mode bit to what it was before the function.
  I2CWriteByte(MPU_ADDRESS, MPU_INTPINCFG_REGISTER, pinCfgState);

  return AKMId;
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
  Serial.println("Identifying MPU mode...");

  if (IdentifyMPUModel() != 0)
  {
    IdentifyAKMMagnetometer();
  }

  Serial.println("");
  delay(2000);
}

