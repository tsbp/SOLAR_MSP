#include "io430.h"
#include "UART.h"
#include "LEDS.h"
//==============================================================================
uPACK txBuf = {._C0 = 0xc0};
unsigned char rxBuf[10];
unsigned char txEn = 0;
//==============================================================================
void msgTransmitt(void)
{
  for(unsigned int i = 0; i < sizeof(txBuf); i++)
  {
    while (!(IFG2&UCA0TXIFG)); 
    UCA0TXBUF  = txBuf.byte[i]; 
  }
  for(unsigned int i = 0; i < sizeof(rxBuf); i++) rxBuf[i] = 0;     
  ledStt &= ~LED_RED;  
}
//==============================================================================
void uartInit(void)
{
  P1SEL  |= (BIT1 | BIT2);                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= (BIT1 | BIT2);                      
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 69;                              // 1MHz 115200
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE; 
}