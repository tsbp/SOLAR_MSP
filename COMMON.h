//==============================================================================
#ifndef COMMON_H
#define COMMON_H
//==============================================================================
// INDICATION
//==============================================================================
#define LED_GRN (BIT1)
#define LED_RED (BIT0)
//==============================================================================
#define BLINK_FORWARD   (0x8040)
#define BLINK_BACKWARD  (0x4080)
#define BLINK_UP        (0xc0ff)
#define BLINK_DOWN      (0xffc0)
#define BLINK_WAIT      (0x9000)
#define BLNK_MAX (180)
//==============================================================================
void leds(unsigned char aStt);
void blinking(unsigned int blnk);
//==============================================================================
extern unsigned long operation_duration;
extern unsigned char ledStt;
extern unsigned int blink;
//==============================================================================
// STATE
//==============================================================================
typedef union
{
  unsigned char byte;
  struct
  { 
    unsigned char i2c           :1;
    unsigned char mPres         :1;
    unsigned char enTrans       :1;
    unsigned char cmdIn         :1;
    unsigned char moveH         :1;
    unsigned char moveV         :1;
    unsigned char moving        :1;
  };
}
uSTATE;
extern uSTATE state;
//==============================================================================
// RELAYS
//==============================================================================

#define REL_PWR1        (BIT2)
#define REL_DIR1        (BIT1)
#define REL_PWR2        (BIT0)
#define REL_DIR2        (BIT5)
#define STOP_MOTORS     {P1OUT &= ~(BIT5); P2OUT &= ~(BIT0 | BIT1 | BIT2); state.moving = 0;    }

#define VERTICAL        (0)
#define HORIZONTAL      (1)
#define FORWARD         (0)
#define BACKWARD        (1)

#define DIRECTION_DELAY (2000000L)
//==============================================================================
void move(unsigned int aPlane, unsigned int aDir);
//==============================================================================
#endif