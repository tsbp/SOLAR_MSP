#include "io430.h"
#include "i2c.h"
//==============================================================================
//#define NUM_BYTES_TX 1                         // How many bytes?
//#define NUM_BYTES_RX 3

unsigned int  NUM_BYTES_TX = 1;                         // How many bytes?
unsigned int  NUM_BYTES_RX = 7;

#define LSM303_ADDR (0x19)

int RXByteCtr, RPT_Flag = 0;                // enables repeated start when 1
unsigned char RxBuffer[8];       // Allocate 128 byte of RAM
unsigned char *PTxData;                     // Pointer to TX data
unsigned char *PRxData;                     // Pointer to RX data
unsigned char TXByteCtr, RX = 0;
unsigned char MSData[10] = {0x20, 0x00};

#define I2C_SPEED       (9)
//==============================================================================
void Setup_TX(unsigned char aDevice){
  __disable_interrupt();
  RX = 0;
  IE2 &= ~UCB0RXIE;  
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = I2C_SPEED;                             // fSCL = SMCLK/12 = ~100kHz
  UCB0BR1 = 0;
  UCB0I2CSA = aDevice;//LSM303_ADDR;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0TXIE;                          // Enable TX interrupt
  
  //RPT_Flag = 1;
}
//==============================================================================
void Setup_RX(unsigned char aDevice){
  __disable_interrupt();
  RX = 1;
  IE2 &= ~UCB0TXIE;  
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = I2C_SPEED;                             // fSCL = SMCLK/12 = ~100kHz
  UCB0BR1 = 0;
  UCB0I2CSA = aDevice;//LSM303_ADDR;                         // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0RXIE;                          // Enable RX interrupt
}
//==============================================================================
void i2cTransmit(void){
    PTxData = MSData;                      // TX array start address
    TXByteCtr = NUM_BYTES_TX;                  // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    
    
}
//==============================================================================
void i2cReceive(unsigned int aCnt, unsigned char *aBuf){
    PRxData = aBuf;//(unsigned char *)RxBuffer;    // Start of RX buffer
    RXByteCtr = aCnt;// - 1;              // Load RX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
//==============================================================================
void i2cInit(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
}
////==============================================================================

//-------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C slave
// to the MSP430 memory. It is structured such that it can be used to receive
// any 2+ number of bytes by pre-loading RXByteCtr with the byte count.
//-------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if(RX == 1)// Master Recieve?
  {                              
    RXByteCtr--;                              // Decrement RX byte counter
    if (RXByteCtr)
    {
      *PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData    
    }
    else
    {
      if(RPT_Flag == 0)  UCB0CTL1 |= UCTXSTP;                // No Repeated Start: stop condition
//      if(RPT_Flag == 1)
//      {                    // if Repeated Start: do nothing
//          RPT_Flag = 0;
//      }
      *PRxData = UCB0RXBUF;                   // Move final RX data to PRxData
      IE2 &= ~(UCB0TXIE | UCB0RXIE);
      __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
    }
  }  
  else // Master Transmit
  {                                    
      if (TXByteCtr)                        // Check TX byte counter
      {
        UCB0TXBUF = *PTxData;                   // Load TX buffer
        PTxData++;        
        TXByteCtr--;                            // Decrement TX byte counter
      }
      else
      {
//        if(RPT_Flag == 1){
//        RPT_Flag = 0;
//        PTxData = MSData;                      // TX array start address
//        TXByteCtr = NUM_BYTES_TX;                  // Load TX byte counter
//        __bic_SR_register_on_exit(CPUOFF);
//        }
//        else
        {
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
        }
      }
  }
  
}

