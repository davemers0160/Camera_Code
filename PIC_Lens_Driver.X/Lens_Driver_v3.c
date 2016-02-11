/******************************************************************************
 *  Name    : Lens_Driver_v1.c
 *  Author  : David Emerson                                     
 *  Notice  : Copyright (c) 2015
 *          : All Rights Reserved                               
 *  Date    : December 29, 2013, 6:50 PM                      
 *  Version : 1.05
 *  Notes   : All code written for the PIC16F1847                                                  
 ******************************************************************************/

// CONFIG1 WORD
#pragma config FCMEN=OFF, FOSC=INTOSC, WDTE=ON, BOREN=OFF, PWRTE=OFF, MCLRE=OFF, IESO=OFF, CP=OFF, CPD=OFF, CLKOUTEN=OFF
// CONFIG2 WORD
#pragma config LVP=OFF, PLLEN=ON, STVREN=OFF, WRT=OFF, BORV=HI

/********************************INCLUDES**************************************/
#include <xc.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "Drone_Slave.h"
#include "PIC16F1847_Config.h"
#include "Master_I2C.h"

// Program EEPROM with initial data during programing cycle
//__EEPROM_DATA(0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);


/********************************DEFINES***************************************/
//#define DRIVER_ADD      0x46    /* I2C Address of lens driver 0x23 */

// define protocol commands
#define LED_OPS     0x20        /* Used to control the LED on the board */

#define CON         0x40        /* Used to determine connection to the PIC */
#define GET_FW      0x41        /* Used to get firmware version of the PIC */
#define GET_SN      0x42        /* Used to get serial number of the PIC */
#define GET_DRV     0x43        /* Used to get the Lens Driver Chip */
#define GET_I2C     0x44        /* Used to get the current I2C error status */
#define GET_AMP     0x45        /* Used to get the current voltage setting stored by the PIC */

#define SET_VOLT    0x50        /* Used to set the voltage level for the driver */
#define FAST_SET_VOLT  0x51     /* Used to set the voltage level for the driver/ no response back from PIC */

#define BL_RST      0xF0        /* Used to put the pic in reset mode for boot loader firmware update */

// Define Pin Names
#define LED       	LATAbits.LATA7      /* LED Control */

/************************Declare Global Variables******************************/
// The variables are automatically assigned an address and stored in the program memory
const unsigned char SN = {1};			// Serial Number
const unsigned char Firmware[2] = {1,07};	// {Major, Minor}
const unsigned char DriverType = {0};             // Driver IC Type: 1 - HV892
const unsigned char DriverAddress = 0x46;

// Structure to store incoming data received on serial port and send outgoing data
typedef struct {
    unsigned char command;          // command code
    unsigned char byte_count;       // byte count
    unsigned char data[20];        // data
    unsigned short checksum;        // checksum
} protocol;

// volatile declaration allows for variable to be modified inside and outside of
// ISR without compiler removing variable
volatile protocol rx_data;          // initialize data packet for receiving data
volatile protocol tx_data;          // initialize data packet for transmitting data

volatile unsigned char rxd=0;       // variable used to determine if valid data was received

volatile unsigned char I2CError;   // I2C error code
                                    // 0 - No Error
                                    // 1 - Error in sending address
                                    // 2 - Error in sending data

volatile unsigned char Amplitude[1];

/**************************FUNCTION DECLARATIONS*******************************/
unsigned char EE_Read(unsigned char Address);
unsigned char EE_Write(unsigned char Address, unsigned char data);
void putChar(unsigned char data);
//void H_SerOut(unsigned char data[], unsigned char size);
//unsigned char H_SerIn(unsigned char data[], unsigned char length);
unsigned char msDelay(unsigned short x);
void check(protocol *chk_data);
void send_Packet(protocol send_data);

/************************Interrupt service Routine*****************************/
void interrupt ISR(void)
{
    unsigned char RxData, idx;

    if(RCIF == 1)             // check for cause of interrupt
    {
        RCIE = 0;             // disable UART1 Rx interrupt

        RxData = RCREG;                // read in data from UART1 register, read clears RC1IF

        if(RxData == '$')               // check for start byte
        {

            while(RCIF == 0); // wait for another byte of data
            rx_data.command = RCREG;
            while(RCIF == 0);
            rx_data.byte_count = RCREG;

            for(idx=0; idx < rx_data.byte_count; idx++)
            {
                while(RCIF == 0);
                rx_data.data[idx] = RCREG;
            }

            rxd = 1;                    // set received data flag
        }

        RCIF = 0;             // make sure UART1 Rx interrupt bit is clear
    }

}   // end of ISR

/********************************Main Routine**********************************/
void main(void) {
    unsigned char idx;

    // Set the pin directions for the Ports
    //        76543210
    TRISA = 0b00100000;                 // Set these pins according to the data direction
                                        // RA5 - MCLR - Input
                                        // RA7 - LED - Output

    TRISB = 0b00010110;                 // Set these pins according to the data direction
                                        // RB1 - SDA - Input
                                        // RB2 - RX - Input
                                        // RB4 - SCL1 - Input
                                        // RB5 - TX - Output

    // Initialize configuration registers, peripherals and timers
    register_config();
    MSSP_config();
    UART_config();
    EE_config();
    Timer_config();

    Amplitude[0] = 0;

    asm("CLRWDT");                  // clear WDT to prevent PIC reset

    SSP1IF = 0;

    // blink LED to indicate PIC functionality
    for(idx = 0; idx < 10; idx++)
    {
        LED = ~LED;
        msDelay(250);
    }
    LED = 0;

    asm("CLRWDT");                  // clear WDT to prevent PIC reset

    while (1)
    {
        asm("CLRWDT");              // clear WDT to prevent PIC reset

        if(rxd==1)                  // check for received data
        {
            switch(rx_data.command)
            {

                case LED_OPS:                      // turn LED on/off
                    LED = rx_data.data[0];
                    tx_data.command = LED_OPS;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = LED;
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;
               
                // send connected indicator with SN, FW Version, DriverType and Current Amplitude
                case CON:                      // send connected message
                    tx_data.command = CON;
                    tx_data.byte_count = 5;
                    tx_data.data[0] = SN;
                    tx_data.data[1] = Firmware[0];
                    tx_data.data[2] = Firmware[1];
                    tx_data.data[3] = DriverType;
                    tx_data.data[4] = Amplitude[0];
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case GET_FW:                      // send firmware version
                    tx_data.command = GET_FW;
                    tx_data.byte_count = 2;
                    tx_data.data[0] = Firmware[0];
                    tx_data.data[1] = Firmware[1];
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case GET_SN:                      // send serial number
                    tx_data.command = GET_SN;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = SN;
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case GET_DRV:
                    tx_data.command = GET_DRV;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = DriverType;
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case GET_I2C:
                    tx_data.command = GET_I2C;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = I2CError;
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case SET_VOLT:
                    // send I2C commands to set voltage
                    Amplitude[0] = rx_data.data[0];
                    
                    I2CError = I2C_Write(DriverAddress, Amplitude, 1);
                    //I2C_Start();
                    //I2CError = I2C_MasterWrite(DriverAddress & 0xFE);
                    //for(idx=0; idx<10; idx++);
                    //I2CError = I2C_MasterWrite(Amplitude) | I2CError;
                    //I2C_Stop();

                    tx_data.command = SET_VOLT;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = I2CError;
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;
                    
                case FAST_SET_VOLT:
                    Amplitude[0] = rx_data.data[0];
                    I2CError = I2C_Write(DriverAddress, Amplitude, 1);
                    break;

                case GET_AMP:
                    tx_data.command = GET_AMP;
                    tx_data.byte_count = 1;
                    tx_data.data[0] = Amplitude[0];
                    check(&tx_data);
                    send_Packet(tx_data);
                    break;

                case BL_RST:
                    //Reset();
                    asm("RESET");
                    break;

                default:
                    break;

            }   // end of switch case

            rxd = 0;                    // clear received data flag
            RCIE = 1;         // enable UART2 Rx interrupt
        }   // end if rxd statement

        else if(RCIE == 0)    // account for an interrupt occurring but with bad input data
        {
            RCIE = 1;         // re-enable Rx interrupt
        }

        asm("CLRWDT");              // clear WDT to prevent PIC reset

    } // end of while(1)

} // end of main


/* Function: unsigned char EE_Read(unsigned short Address)
 *
 * Arguments:
 * 1. Address: address of EEPROM memory to read
 *
 * Return Value:
 * 1. EEDATL: contents of EEPROM at given address
 *
 * Description: Code to read data from EEPROM at the given address*/
unsigned char EE_Read(unsigned char Address)
{
    EEADRL = Address;
    EE_RD = 1; // enable read operation

    while (EE_RD == 1); // wait for read operation to complete

    return EEDATL;
} // end of EE_Read

/* Function: unsigned char EE_Write(unsigned short Address,  unsigned char data)
 *
 * Arguments:
 * 1. Address: address of EEPROM memory to write
 * 2. data: 1-byte of data to write
 *
 * Return Value:
 * 1. Error: Error status of the write operation (0=success, 1=failure)
 *
 * Description: Code to write a byte of data to the given address */
unsigned char EE_Write(unsigned char Address, unsigned char data)
{
    SSP1IE = 0; // disable MSSP interrupts

    EEADRL = Address; // write address to EE address register
    EEDATL = data; // write data to EE data resiger

    EE_WREN = 1; // enable write operations
    EECON2 = 0x55; // initiate write cycle
    EECON2 = 0xAA;
    EE_WR = 1; // begin write operation
    EE_WREN = 0; // disable write operations

    while (EE_WR == 1); // wait for EEPROM write to complete

    SSP1IE = 1; // renable MSSP interrupts

    return EE_ERR; // return error status

} // end of EE_Write

/* Function: void putChar(unsigned char data)
 *
 * Arguments:
 * 1. data: pointer to the location of the data to be sent
 *
 * Return Value:
 * 1. None
 *
 * Description: Function to send a byte of data serially using the hardware serial port*/
void putChar(unsigned char data)
{
    while (TX_BUF == 0); // Wait for UART buffer to be empty to send data
    asm("NOP");
    TXREG = data; // transmit data over USART

} // end of putChar

/* Function: unsigned char H_SerIn(unsigned char data[], unsigned char length)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. temp_char: character recieved via USART
 *
 * Description: Function to receive a byte of data serially using the hardware serial port. */
//unsigned char H_SerIn(unsigned char data[], unsigned char length)
//{
//    unsigned char i;
//
//    for(i=0; i<length; i++)
//    {
//        TMR1 = 535;                         // set timer to rollover in 65ms
//        TMR1IF = 0;
//
//        while((RCIF == 0) && (TMR1IF == 0)); // wait for serial data or clock overflow
//
//        if(TMR1IF == 1)
//        {
//            return 0;                       // if timer overflow return 0 and exit
//        }
//        else
//        {
//            if(OERR == 1)
//            {
//                CREN = 0;
//                data[i] = RCREG;
//                CREN = 1;
//            }
//            else
//            {
//                data[i] = RCREG;
//            }
//        }
//    }
//
//    return 1;
//
//} // end of H_SerIn

/* Function: protocol check(protocol chk_data)
 *
 * Input Arguments:
 * 1. chk_data: This input is a structure of type "protocol" which contains the data
 *              perform the checksum calculations on and increase the byte count.
 *
 * Return Value:
 * 1. chk_data: The output is a structure of type "protocol" containing the calculated
 *              16-bit Fletcher checksum and adjusted byte count.
 *
 * Description: Function designed to calculate the 16-bit Fletcher checksum and
 * increase the byte count by two additional bytes to account for the checksum
 * being sent in the packet */
//protocol check(protocol chk_data)
void check(protocol *chk_data)
{
    unsigned short sum1=0, sum2=0;
    unsigned short i;

    chk_data->byte_count = chk_data->byte_count + 2;      // increase byte count by 2

    sum1 = (sum1 + chk_data->command) % 255;
    sum2 = (sum2 + sum1) % 255;

    sum1 = (sum1 + chk_data->byte_count) % 255;
    sum2 = (sum2 + sum1) % 255;

    for(i=0; i<chk_data->byte_count-2; i++)              // perform CRC calculations
    {
        sum1 = (sum1 + chk_data->data[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }

    // 16-bit Fletcher checksum
    chk_data->checksum = (sum2<<8) | sum1;

    //return chk_data;

}   // end of check

/* Function: void send_Packet(protocol send_data)
 *
 * Input Arguments:
 * 1. send_data: This input is a structure of type "protocol" which contains the data
 *               to send using the specified serial port.
 *
 * Return Value:
 * 1. None
 *
 * Description: This function designed to send the entire contents of a protocol
 * packet to the requesting computer using the specified serial port.
*/
void send_Packet(protocol send_data)
{
    unsigned char idx;

    putChar(send_data.command);           // send command byte
    putChar(send_data.byte_count);        // send byte count

    for (idx = 0; idx < send_data.byte_count-2; idx++)
    {
        putChar(send_data.data[idx]);       // send data bytes
    }

    putChar(send_data.checksum>>8);        // send first 8 MSB's
    putChar(send_data.checksum&0x00FF);    // send last 8 bits

}   // end of send_Packet


/* Function: unsigned char msDelay(unsigned short x)
 *
 * Arguments:
 * 1. x: Number of milliseconds to delay
 *
 * Return Value:
 * 1. None
 *
 * Description: Function to create a millisecond delay using Timer2. */
unsigned char msDelay(unsigned short x)
{
    unsigned short idx;

    for (idx = 0; idx < x; idx++)
    {
        TMR2 = 0;
        while (TMR2 < PR2);
    }

    return 1;

} // end of msDelay

