#include "io430.h"
#include "UART.h"
#include "COMMON.h"
//==============================================================================
//uPACK txBuf = {._C0 = 0xc0};
unsigned char rxBuf[20];

//==============================================================================
void msgTransmitt(unsigned char *aBuf, unsigned int aCnt)
{
  for(unsigned int i = 0; i < aCnt; i++)
  {
    while (!(IFG2&UCA0TXIFG)); 
    UCA0TXBUF  = aBuf[i]; 
  }
  for(unsigned int i = 0; i < sizeof(rxBuf); i++) rxBuf[i] = 0;     
  //ledStt &= ~LED_RED;  
}
//==============================================================================
void uartInit(void)
{
  P1SEL  |= (BIT1 | BIT2);                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= (BIT1 | BIT2);                      
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 16;                              // 1MHz 115200
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE; 
}
//==============================================================================
unsigned int byteCntr = 0;
unsigned int uart_cntr;
//==============================================================================
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   //static unsigned int cntr = 0, lng = 0;
   //static unsigned int stuffing = 0, rec_counter=0;  // ������� �������� ����  
   unsigned char rxByte = UCA0RXBUF;
   //============================
//   if(rxByte = ID_MASTER && cntr == 0)  
//     
//     
//   if (cntr == 2) 
//   else if (cntr == 2) // data length 
//   {
//     lng  = rxByte;
//     rxBuf[cntr] = rxByte;
//   }
   
   rxBuf[byteCntr] = rxByte;
   uart_cntr = UART_INTERVAL;  
   state.enTrans = 1;
   if (byteCntr < sizeof(rxBuf)) byteCntr++;
   //else cntr = 0;
//   if(rxByte == 0xc0)
//   {     
//     ledStt |= LED_RED;
//     rec_counter = 0;     
//   }
//   else 
//    {
//      if ((rxByte == 0xdb) && !stuffing) stuffing = 1;
//      else if (stuffing) 
//      {
//        if      (rxByte == 0xdc) rxByte = 0xc0;
//        else if (rxByte == 0xdd) rxByte = 0xdb;
//        stuffing = 0;
//      }
//      if(!stuffing)
//      {        
//         if(rec_counter < sizeof(rxBuf)) rxBuf[rec_counter] = rxByte;
//         if(rxBuf[3] == 0xcc && rxBuf[4] == 0xcc)  state.cmdIn = 1;
//         rec_counter++;
//      } 
//    }
}