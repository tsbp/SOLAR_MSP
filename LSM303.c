//==============================================================================
#include "io430.h"
#include "LSM303.h"
#include "i2c.h"
//==============================================================================
void lsm303(unsigned char aOp, unsigned char aDev, unsigned char aReg, unsigned char *aBuf, unsigned int aLng)
{
  
  if(aOp == I2C_READ && aLng > 1) MSData[0] = (aReg | BIT7);
  else                               MSData[0] = (aReg);
    
  if(aOp == I2C_WRITE) 
  {
    MSData[1] = *aBuf;
    NUM_BYTES_TX = 2;
  }  
  else
    NUM_BYTES_TX = 1;
   //Transmit process
  Setup_TX(aDev);  
  
  i2cTransmit();
  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  
  if(aOp == I2C_READ)
  {
    Setup_RX(aDev);
    i2cReceive(aLng/*6*/, aBuf);
    while (UCB0CTL1 & UCTXSTP);
    __no_operation();
  }
}
//==============================================================================
unsigned char regValue = 0;
//==============================================================================
void LSM303Init(void)
{
  //============= accelerometer ==========================
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG1, &regValue, 1);
  regValue = 0x77;
  lsm303(I2C_WRITE, LSM303A_I2C_ADDR, LSM303A_CTRL_REG1, &regValue,   1);
  //__delay_cycles(80000L);
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG1, &regValue, 1);
  
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG2, &regValue, 1);
  regValue = 0x00;
  lsm303(I2C_WRITE, LSM303A_I2C_ADDR, LSM303A_CTRL_REG2, &regValue,   1);
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG2, &regValue, 1);
  
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG4, &regValue, 1);
  regValue |= (BIT3 );//| BIT7);
  lsm303(I2C_WRITE, LSM303A_I2C_ADDR, LSM303A_CTRL_REG4, &regValue, 1);
  __delay_cycles(80000L);
  lsm303(I2C_READ,  LSM303A_I2C_ADDR, LSM303A_CTRL_REG4, &regValue, 1);
  
  //============= magnitometer ===========================
  lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_CRA_REG, &regValue, 1);
  regValue = 0x9c;
  lsm303(I2C_WRITE, LSM303M_I2C_ADDR, LSM303M_CRA_REG, &regValue, 1);
  lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_CRB_REG, &regValue, 1);
  
  lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_MR_REG,  &regValue, 1);
  regValue = 0;
  lsm303(I2C_WRITE, LSM303M_I2C_ADDR, LSM303M_MR_REG,  &regValue, 1);
  __delay_cycles(80000L);
  lsm303(I2C_READ,  LSM303M_I2C_ADDR, LSM303M_MR_REG,  &regValue, 1);
}