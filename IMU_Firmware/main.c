/*
    10-5-11
 Copyright Spark Fun Electronics� 2011
 Aaron Weiss
 aaron at sparkfun dot com
 
 	9DOF-Razor-IMU-AHRS compatible
 	
 	ATMega328@3.3V w/ external 8MHz resonator
 	High fuse 0xDA
 Low fuse 0xFF
 	EXT fust 0xF8
 	
 	Default Baud: 57600bps
 	
 	Revision Notes:
 	Hardware v22
 	Firmware: 
 	v18 took self test off of menu, explain how to use it in help menu
 	v19 added baud rate selection, default to 57600bps, various bug fixes
 	v19i fixed baud menu return bugs
 	v20 using ITG3200 gyro
 	v21 added auto self test upon startup (see notes)
 	v22 corrected HMC output, x and y registers are different in the HMC5883
 	
 	ADXL345: Accelerometer
 	HMC5883: Magnetometer
 	ITG3200: Pitch, Roll, and Yaw Gyro
 	
 	Notes: 
 	
 	-To get out of autorun, hit ctrl-z
 	-max baud rate @8MHz is 57600bps
 	-self-test startup: LED blinks 5 times then OFF = GOOD, LED ON = BAD
 	
 */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "types.h"
#include "defs.h"
#include "i2c.h"

#define STATUS_LED 5 //stat LED is on PB5

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define ITG3200_R 0xD1	// ADD pin is pulled low
#define ITG3200_W 0xD0 

///============Initialize Prototypes=====//////////////////
void init(void);
unsigned int UART_Init(unsigned int ubrr);
uint8_t uart_getchar(void);
static int uart_putchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
void i2cInit(void);

///============Function Prototypes=========/////////////////
void accelerometer_init(void);
void auto_raw(void);
void baud_menu(void);
void check_baud(void);
void config_menu(void);
void config_read(void);
void gyro_init(void);
void help(void);
void magnetometer(void);
void magnetometer_init(void);
void print_adxl345(void);
void print_hmc5883(void);
void print_itg3200(void);
void raw(void);
void self_test(void);
uint16_t x_accel(void);
uint16_t y_accel(void);
uint16_t z_accel(void);
uint16_t x_gyro(void);
uint16_t y_gyro(void);
uint16_t z_gyro(void);
uint16_t sqrtTest(int16_t InitialGuess, int16_t Number);

///============EEPROM Protoypes============//////////////////
void write_to_EEPROM(unsigned int Address, unsigned char Data);
unsigned char read_from_EEPROM(unsigned int Address);

///============Display Strings============//////////////////
const char wlcm_str[] PROGMEM = "\n\n\r9DOF IMU Firmware v22 \n\r==========================";
const char accel[] PROGMEM = "\n\r[1]Accelerometer: ADXL345 \n\r";
const char mag[] PROGMEM = "[2]Magnetometer: HMC5883 \n\r";
const char gyro[] PROGMEM = "[3]Gyroscope: ITG-3200 \n\r";
const char raw_out[] PROGMEM = "[4]Raw Output\n\r";
const char baud_change[] PROGMEM = "[5]Change Baud Rate: ";
const char autorun[] PROGMEM = "[Ctrl+z]Toggle Autorun\n\r";
const char help_[] PROGMEM = "[?]Help\n\r";

///============Global Vars=========/////////////////
uint16_t x_mag, y_mag, z_mag; //x, y, and z magnetometer values
long baud;


/////===========MAIN=====================/////////////////////
int main(void)
{

  int16_t x,y,z,i;
  uint16_t sqr,srt;
  init();
  
  // set UART speed to 57600
  UART_Init(16);
  baud = 57600;

  while(1)
  {	

    //check to see if autorun is set, if it is don't print the menu
    //if(read_from_EEPROM(1) == 48) config_read();
    //else config_menu();
    
    // reset x, y, z variables
    x = 0;
    y = 0;
    z = 0;
    
    // collect 16 samples on each axis and sum together
    for(i=0;i<16;i++)
    {
      x += x_accel();
      y += y_accel();
      z += z_accel();

      delay_ms(62);      // delay designed to put average out on a 1 second interval
    }
    
    // take the average
    x=x>>4;
    y=y>>4;
    z=z>>4;
    
    // print out th results of the 
    printf("%d,",x);
    printf("%d,",y);
    printf("%d\n\r",z);
  }

}  // end of main

void accelerometer_init(void)
{

  //initialize
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x2D);    //power register
  i2cWaitForComplete();
  i2cSendByte(0x08);    //measurement mode
  i2cWaitForComplete();
  i2cSendStop();

  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x31);    //data format
  i2cWaitForComplete();
  i2cSendByte(0x08);    //full resolution
  i2cWaitForComplete();
  i2cSendStop();

}


uint16_t x_accel(void)
{
  //0xA6 for a write
  //0xA7 for a read

    uint8_t dummy, xh, xl;
  uint16_t xo;

  //0x32 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x32);    //X0 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  xl = i2cGetReceivedByte();	//x low byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();	

  //0x33 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x33);    //X1 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  xh = i2cGetReceivedByte();	//x high byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();
  xo = xl|(xh << 8);
  return xo;
}

uint16_t y_accel(void)
{		
  //0xA6 for a write
  //0xA7 for a read

    uint8_t dummy, yh, yl;
  uint16_t yo;

  //0x34 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x34);    //Y0 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  yl = i2cGetReceivedByte();	//x low byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();	

  //0x35 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x35);    //Y1 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  yh = i2cGetReceivedByte();	//y high byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();
  yo = yl|(yh << 8);
  return yo;
}

uint16_t z_accel(void)
{	
  //0xA6 for a write
  //0xA7 for a read

    uint8_t dummy, zh, zl;
  uint16_t zo;

  //0x36 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x36);    //Z0 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  zl = i2cGetReceivedByte();	//z low byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();	

  //0x37 data registers
  i2cSendStart();
  i2cWaitForComplete();
  i2cSendByte(0xA6);    //write to ADXL
  i2cWaitForComplete();
  i2cSendByte(0x37);    //Z1 data register
  i2cWaitForComplete();

  i2cSendStop();		 //repeat start
  i2cSendStart();

  i2cWaitForComplete();
  i2cSendByte(0xA7);    //read from ADXL
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();
  zh = i2cGetReceivedByte();	//z high byte
  i2cWaitForComplete();
  i2cReceiveByte(FALSE);
  i2cWaitForComplete();
  dummy = i2cGetReceivedByte();	//must do a multiple byte read?
  i2cWaitForComplete();
  i2cSendStop();
  zo = zl|(zh << 8);	
  return zo;
}

/*********************
 ****Initialize****
 *********************/

void init (void)
{
  //1 = output, 0 = input
  DDRB = 0b01100000; //PORTB4, B5 output for stat LED
  DDRC = 0b00010000; //PORTC4 (SDA), PORTC5 (SCL), PORTC all others are inputs
  DDRD = 0b00000010; //PORTD (TX output on PD1)
  PORTC = 0b00110000; //pullups on the I2C bus

  cbi(PORTB, 5);

  i2cInit();
  accelerometer_init();

  //check_baud();
}

unsigned int UART_Init(unsigned int ubrr)
{
  int ubrr_new;
  // set baud rate
  ubrr_new = ubrr; 
  UBRR0H = ubrr_new>>8;
  UBRR0L = ubrr_new;

  // Enable receiver and transmitter 
  UCSR0A = (1<<U2X0); //double the speed
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);

  // Set frame format: 8 bit, no parity, 1 stop bit,   
  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);

  stdout = &mystdout; //Required for printf init
  return(ubrr);
}

uint8_t uart_getchar(void)
{
  while( !(UCSR0A & (1<<RXC0)) );
  return(UDR0);
}

static int uart_putchar(char c, FILE *stream)
{
  if (c == '\n') uart_putchar('\r', stream);

  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;

  return 0;
}

/*********************
 **EEPROM Map and Functions***
 *********************/
//Address Map for EEAR
//0x01-0x07 used

//Description: Writes an unsigned char(Data)  to the EEPROM at the given Address
//Pre: Unsigned Int Address contains address to be written to
//	 Unsigned Char Data contains data to be written
//Post: EEPROM "Address" contains the "Data"
//Usage: write_to_EEPROM(0, 'A');
void write_to_EEPROM(unsigned int Address, unsigned char Data)
{
  //Interrupts are globally disabled!

  while(EECR & (1<<EEPE)); //Wait for last Write to complete
  //May need to wait for Flash to complete also!
  EEAR = Address;			//Assign the Address Register with "Address"
  EEDR=Data;				//Put "Data" in the Data Register
  EECR |= (1<<EEMPE); 	//Write to Master Write Enable
  EECR |= (1<<EEPE);  	//Start Write by setting EE Write Enable
}

//Description: Reads the EEPROM data at "Address" and returns the character
//Pre: Unsigned Int Address is the address to be read
//Post: Character at "Address" is returned
//Usage: 	unsigned char Data;
//		Data=read_from_EEPROM(0);
unsigned char read_from_EEPROM(unsigned int Address)
{
  //Interrupts are globally disabled!

  while(EECR & (1<<EEPE));	//Wait for last Write to complete
  EEAR = Address;				//Assign the Address Register with "Address"
  EECR |= (1<<EERE); 			//Start Read by writing to EER
  return EEDR;				//EEPROM Data is returned
}


