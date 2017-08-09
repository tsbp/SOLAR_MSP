#define LED_GRN (BIT1)
#define LED_RED (BIT0)

#define BLINK_FORWARD   (0x8040)
#define BLINK_BACKWARD  (0x4080)
#define BLINK_UP        (0xc0ff)
#define BLINK_DOWN      (0xffc0)
#define BLINK_WAIT      (0x9000)
#define BLNK_MAX (12000)


void leds(unsigned char aStt);
void blinking(unsigned int blnk);


extern unsigned char ledStt;
extern unsigned int blink;