#include <stdint.h>
#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ov7670.h"

void setColor(void) {
  wrSensorRegs8_8(yuv422_ov7670);
}

void setRes(void) {
  wrReg(REG_COM3, 4); // REG_COM3 enable scaling
  wrSensorRegs8_8(qvga_ov7670);
  wrReg(0x11, 11); // PCLK prescaler
}

void camInit(void) {
  wrReg(0x12, 0x80);
  _delay_ms(100);
  wrSensorRegs8_8(ov7670_default_regs);
  wrReg(REG_COM10, 32);//PCLK does not toggle on HBLANK.
}

void arduinoUnoInit(void) {
  cli();//disable interrupts

  /* Setup the 8mhz PWM clock
    This will be on pin 11*/
  DDRB |= (1 << 3);//pin 11
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  TCCR2A = (1 << COM2A0) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << WGM22) | (1 << CS20);
  OCR2A = 0;//(F_CPU)/(2*(X+1))
  DDRC &= ~15;//low d0-d3 camera
  DDRD &= ~252;//d7-d4 and interrupt pins
  _delay_ms(3000);

  //set up twi for 100khz
  TWSR &= ~3;//disable prescaler for TWI
  TWBR = 72;//set to 100khz

  //enable serial
  UBRR0H = 0;
  UBRR0L = 1;//0 = 2M baud rate. 1 = 1M baud. 3 = 0.5M. 7 = 250k 207 is 9600 baud rate.
  UCSR0A |= 2;//double speed aysnc
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);//Enable receiver and transmitter
  UCSR0C = 6;//async 1 stop bit 8bit char no parity bits
}


void StringPgm(const char * str) {
  do {
    while (!(UCSR0A & (1 << UDRE0)));//wait for byte to transmit
    UDR0 = pgm_read_byte_near(str);
    while (!(UCSR0A & (1 << UDRE0)));//wait for byte to transmit
  } while (pgm_read_byte_near(++str));
}

static void captureImg(){
  uint16_t y, x;

  StringPgm(PSTR("*RDY*"));

  while (!(PIND & 8));//wait for high
  while ((PIND & 8));//wait for low

    y = 240;
  while (y--){
        x = 320;
      //while (!(PIND & 256));//wait for high
    while (x--){
      while ((PIND & 4));//wait for low
      if( (y >= 60) && ( y < 180 ) && (x >= 80) && ( x < 240 ) ){
          UDR0 = (PINC & 15) | (PIND & 240);
          while (!(UCSR0A & (1 << UDRE0)));//wait for byte to transmit
      }
      while (!(PIND & 4));//wait for high
      while ((PIND & 4));//wait for low
      while (!(PIND & 4));//wait for high
    }
    //  while ((PIND & 256));//wait for low
  }
    _delay_ms(100);
}

void setup(){
  arduinoUnoInit();
  camInit();
  setRes();
  setColor();
  
  pinMode(13, OUTPUT); // led for visual debugging
  pinMode(12, INPUT); // simulate the door phone button

  // a small flash after which the interphone can be played
  digitalWrite(13, HIGH);
  _delay_ms(200);
  digitalWrite(13, LOW);
}


int lettura = 0, old_lettura = 0;

void loop(){
  
  lettura = digitalRead(12); // read button status

  // capture the image only if the button come pressed and not if it was remained pressed
  if (old_lettura != lettura && old_lettura == 0) {
    
    digitalWrite(13, HIGH); // high when the button come pressed, wait 3 seconds
    _delay_ms(3000);
    digitalWrite(13, LOW); // little blink when the photo starts
    _delay_ms(200);
    
    digitalWrite(13, HIGH); // led high while capturing
    captureImg();
    digitalWrite(13, LOW); // led low when finished
    
    _delay_ms(4000); // wait 4 seconds before strat a new capturing
    digitalWrite(13, HIGH); // little blink to signal that it is possible to start a new capturing
    _delay_ms(200);
    digitalWrite(13, LOW);
  }
  
  old_lettura = lettura; // old_lettura remains high until the next time that lettura come back high, double read avoided if the button didn't come released
  delay(200);
}
