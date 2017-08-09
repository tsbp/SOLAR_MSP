//==============================================================================
#pragma pack(1)
typedef struct
  {    
    int x;
    int y;   
    int z;      
  }sRAW;
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
    sRAW          compass;
    sRAW          accel;   
    unsigned int  light;  
    unsigned int  crc;
  };
}uPACK;
#pragma pack()
//==============================================================================
void msgTransmitt(void);
void uartInit(void);

extern uPACK txBuf;
extern unsigned char rxBuf[10];
extern unsigned char txEn;