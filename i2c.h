#ifndef I2C_H
#define I2C_H

void i2cInit(void);
//void i2cWrite(unsigned char aSub, unsigned char *aBuf, unsigned int aLng);
//void i2cRead(void);


void Setup_TX(unsigned char aDevice);
void Setup_RX(unsigned char aDevice);
void i2cTransmit(void);
void i2cReceive(unsigned int aCnt, unsigned char *aBuf);
//==============================================================================
extern unsigned char RxBuffer[];
extern unsigned char MSData[];
extern unsigned int  NUM_BYTES_TX;
#endif