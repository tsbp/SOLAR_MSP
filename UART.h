//==============================================================================
// EXTERNAL COMMANDS
//==============================================================================
#define CMD_SET_AZIMUTH     (0x10)
#define CMD_SET_ANGLE       (0x11)

#define CMD_UP     (0x20)
#define CMD_DOWN   (0x21)
#define CMD_RIGHT  (0x22)
#define CMD_LEFT   (0x23)
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
void msgTransmitt(void);
void uartInit(void);

extern uPACK txBuf;
extern unsigned char rxBuf[10];
extern unsigned char txEn;