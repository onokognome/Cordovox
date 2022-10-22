


/********************
 * SPI Frame
 * |  0  |  1  |  0  |  0  | A2 | A1 | A0 | RW | A7 | A6 | A5 | A4 | A3 | A2 | A1 | A0 |
 * 
 * A2, A1, A0 in high byte are chip addressses (we aren't using)
 * 
 * IOCON Register
 *   BANK | MIRROR | SEQOP | DISSLW | HAEN | ODR | INTPO | - |
 *   
 *   BANK = 1 : register banks are separate
 *   MIRROR = 0 : interupts are bank specific (but we're not using them anyway)
 *   SEQOP = 1 : address pointer does not increment
 *   DISSLW = 0 : slew rate control for SDA enabled
 *   HAEN = 0; Disables the chip address pins
 *   ODR = 0 : active driver out (INTPOL bit active, but we don't use this anyway)
 *   INTPO = 0 : interrupt polarity active low (but we don't use this)
 *   
 *  We won't configure interrupts. Pin direction defaluts to input. Polarity devaluts to no change.  
 *   
 */

 // chip setup 

#define OPCODE_READ   0x4100
#define OPCODE_WRITE  0x4000
#define IOCON_ADDR    0x0A  //0x05
#define IOCON_VALUE   0x00
#define IODIRA_ADDR   0x00
#define IODIRB_ADDR   0x01

#define GPPUA_ADDR    0x0C    // input pull config register
#define GPPUB_ADDR    0x0D
#define GPPU_VALUE    0xff    // turn them on

#define GPIOA         0x12
#define GPIOB         0x13
