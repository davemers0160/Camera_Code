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
#include "Arduino.h"


//#define STATUS_LED 5     /* stat LED is on PB5 */
//#define ODROID_PWR  PORTB4   /* Pin to toggle power on and off for the Odroid */

#define ON          1    /* Define Power On as 1 */
#define OFF         0    /* Define Power Off as 0 */

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
uint16_t SQRT(int16_t InitialGuess, uint16_t Number);


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

int HUB_PWR = 15;
int ODROID_PWR = 16;
int STATUS_LED = 17;


/////===========MAIN=====================/////////////////////
int main(void)
{
  int16_t i;
  int16_t Ax0, Ay0, Az0, Ax, Ay, Az;
  int16_t Vx, Vy, Vz;
  int16_t Vx0 = 0, Vy0 = 0, Vz0 = 0;
  uint16_t X2, Y2, Z2;  
  uint16_t VX2, VY2, VZ2;
  uint16_t sqr, sq_root;
  uint16_t V_sqr;
  uint16_t V;
  
  uint8_t odroid_status=0;
  
  init();
  
  // set UART speed to 57600
  UART_Init(16);
  baud = 57600;


  sbi(PORTB,5);
  
  Ax0 = 0;
  Ay0 = 0;
  Az0 = 0;
  
  while(1)
  {	

    //check to see if autorun is set, if it is don't print the menu
    //if(read_from_EEPROM(1) == 48) config_read();
    //else config_menu();
    
    // reset x, y, z variables
    Ax = 0;
    Ay = 0;
    Az = 0;
    
    // collect 16 samples on each axis and sum together
    for(i=0;i<16;i++)
    {
      Ax += x_accel();
      Ay += y_accel();
      Az += z_accel();

      delay_ms(62);      // delay designed to put average out on a 1 second interval
    }
    
    // take the average and one extra division by 2
    Ax = Ax>>4;
    Ay = Ay>>4;
    Az = Az>>4;
    
    // The  Accelerations are now averaged
    
    // print out the accelerations
    printf("%d,%d,%d,",Ax,Ay,Az);
//    printf("%d,", Ax);
//    printf("%d,", Ay);
//    printf("%d,", Az);  
//    printf("\r\n");
    
    
    Vx = Vx0 - (Ax-Ax0);
    Vy = Vy0 - (Ay-Ay0);
    Vz = Vz0 - (Az-Az0);

    printf("%d,%d,%d,",Vx,Vy,Vz);
    
    
    Vx = Vx>>2;
    Vy = Vy>>2;
    Vz = Vz>>2;
    
    VX2 = (Vx*Vx);
    VY2 = (Vy*Vy);
    VZ2 = (Vz*Vz);  
    
    // debug
//    printf("Vx = %4d\tVx0 = %4d\tVx^2 = %5u\r\n",Vx,Vx0,VX2);
//    printf("Vy = %4d\tVy0 = %4d\tVy^2 = %5u\r\n",Vy,Vy0,VY2);
//    printf("Vz = %4d\tVz0 = %4d\tVz^2 = %5u\r\n",Vz,Vz0,VZ2);
    
    // divide by 4 to reduce the size 
//    Ax = Ax>>2;
//    Ay = Ay>>2;
//    Az = Az>>2;   
    
//    X2 = (Ax*Ax);
//    Y2 = (Ay*Ay);
//    Z2 = (Az*Az);
    
    V_sqr = (VX2 + VY2 + VZ2);
    
    V = 4*SQRT(Vx, V_sqr);
    
    // debug
    //printf("V_sqr = %5u\tV = ",V_sqr);
    
    printf("%u\n\r",V);
    //odroid_status++;
    
    //cbi(PORTB,5);
    odroid_status ^= 0x01;
    //sbi(PORTB,5);
    
    Vx0 = Vx;
    Vy0 = Vy;
    Vz0 = Vz;
    
    Ax0 = Ax;
    Ay0 = Ay;
    Az0 = Az;
    
    // print out th results of the 

//    printf("%u,",X2);
//    printf("%u,",Y2);
//    printf("%u,",Z2);    
//    printf("%u,",sqr);
    
    //printf("%u\n\r",sq_root);
    // test to toggle PB4 pin - ODROID_PWR
    //digitalWrite(ODROID_PWR, !digitalRead(ODROID_PWR));

    
  }

}  // end of main

/////////////////////////////////////////////////////////////////////////////////

uint16_t SQRT(int16_t InitialGuess, uint16_t Number)
{
	uint16_t max_iterations = 20;

	int16_t sqrt_err = 1000;
	int16_t min_sqrt_err = 1;
	int16_t Xo = InitialGuess;

	int16_t iteration_count;

	uint16_t result = 0;
	if (Number == 0) // zero check
	{
		return result;
	}	

	if (Xo<0)
	{
		Xo = Xo*(-1);
	}

	iteration_count = 0;
	// printf("Number = %4u\r\n", Number);
	// printf("result = %4u ",result);
	// printf("Xo = %4d ",Xo);	
	// printf("sqrt_err = %4d ",sqrt_err);
	// printf("\r\n");

	while (iteration_count < max_iterations && ((sqrt_err > min_sqrt_err || sqrt_err < -min_sqrt_err)))
	{

		//result = (uint16_t)(Xo - (Xo * Xo - Number) / (2 * Xo));
		result = (Xo + (Number / Xo))>>1;
		//result = Xo - (Xo >> 1) + ((Number / Xo) >> 1);

		//printf("inner: %4d ", ((int16_t)((Xo * Xo) - Number) / (2 * Xo)));
		// printf("result = %4u ", result);
		// printf("Xo = %4d ", Xo);
		
		sqrt_err = (int16_t)(result - Xo);
		Xo = (int16_t)(result);
		
		// printf("sqrt_err = %4d ", sqrt_err);
		// printf("\r\n");
		iteration_count++;
	}
	return result;

}   // end of SQRT

////////////////////////////////////////////////////////////////////////////

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
  DDRB = 0b01111000; //PORTB4, B5 output for stat LED
  DDRC = 0b00010000; //PORTC4 (SDA), PORTC5 (SCL), PORTC all others are inputs
  DDRD = 0b00000010; //PORTD (TX output on PD1)
  PORTC = 0b00110000; //pullups on the I2C bus

  cbi(PORTB, 5);

  i2cInit();
  accelerometer_init();
  
  pinMode(STATUS_LED,OUTPUT);

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


