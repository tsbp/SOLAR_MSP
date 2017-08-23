//==============================================================================
#include "BH1715.h"
#include "i2c.h"
//==============================================================================
void BH1715(unsigned char aOp, unsigned char aDev, unsigned char aOpCode, unsigned char *aBuf, unsigned int aLng)
{
  
//  if(aOp == LSM303_READ && aLng > 1) MSData[0] = (aReg | BIT7);
//  else                               MSData[0] = (aReg);
    
  if(aOp == I2C_WRITE) 
  {
    MSData[0] = aOpCode;//*aBuf;
    NUM_BYTES_TX = 1;
    Setup_TX(aDev);
    i2cTransmit(); 
     while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
  }  
//  else
//    NUM_BYTES_TX = 1;
   //Transmit process
   
  
  
 
  
  if(aOp == I2C_READ)
  {
    Setup_RX(aDev);
    i2cReceive(aLng/*6*/, aBuf);
    while (UCB0CTL1 & UCTXSTP);
    __no_operation();
  }
}