#ifndef __UART_H
#define __UART_H
//==============================================================================
// EXTERNAL COMMANDS
//==============================================================================
#define OK			(0xff)
#define BAD			(0x00)

#define ID_SLAVE	(0x3C)
#define ID_MASTER	(0x7E)

#define CMD_ANGLE   (0x10)
#define CMD_AZIMUTH	(0x11)
#define CMD_LEFT	(0x20)
#define CMD_RIGHT	(0x21)
#define CMD_UP	    (0x22)
#define CMD_DOWN	(0x23)
#define CMD_STATE	(0xA0)
#define CMD_CFG		(0xC0)
//==============================================================================
#pragma pack(1)
typedef struct
  {    
    int roll;
    int pitch;   
    int heading;      
  }sAngles;
#pragma pack()
//==============================================================================
#pragma pack(1)
typedef struct
  {    
    int a;
    int b;   
    int c;      
  }sDATA;
#pragma pack()
//==============================================================================
#pragma pack(1)
typedef union
{
  unsigned char byte[18];
  struct
  {
    unsigned char _C0;
    unsigned char cmd;
    sDATA         data;
    sAngles       angles;   
    unsigned int  light;  
    unsigned int  crc;
  };
}uPACK;
#pragma pack()
//==============================================================================
#define UART_INTERVAL    (10)
//==============================================================================
void msgTransmitt(unsigned char *aBuf, unsigned int aCnt);
void uartInit(void);

//extern uPACK txBuf;
extern unsigned char rxBuf[];
extern unsigned char txEn;
extern unsigned int byteCntr;
extern unsigned int uart_cntr;
#endif