/* Drone code file to define I2C slave states for drone
 *
 * All code written for the PIC16F1847
 */

/*********************************INCLUDES*************************************/
#include <xc.h>
#include <stdio.h>
#include "PIC16F1847_Config.h"
#include "Master_I2C.h"

/*****************************Global Variables*********************************/
//extern volatile unsigned char TxIndex, RxIndex;
//extern volatile unsigned char RxData[RX_BUFF];
//extern volatile unsigned char TxData[TX_BUFF];


void I2C_Start(void)
{
    SEN = 1;
    while(SEN==1);
    while(SSP1IF == 0);
}


void I2C_Stop(void)
{
    PEN  = 1;
    while(PEN==1);
    SSP1IF = 0;
}

unsigned char I2C_MasterWrite(unsigned char data)
{

    while(SSP1IF==1)
    {
        SSP1IF = 0;
        SSP1BUF = data;
        //while(BF == 1);                 // wait for SSP1BUF to send all data out
        while ( ( SSP1CON2 & 0x1F ) || ( SSP1STAT & 0x04 ) );
        if(ACKSTAT == 1)
        {
            I2C_Error();
            return 1;
        }
    }
    return 0;

}

/* Function: void I2C_Write(unsigned char Address, unsigned char TxData[], unsigned char length)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to handle the various I2C communications states for reading
 * and writing data States based on Microchip's AppNote AN734 Data should be in
 * the form of two control bytes and 0-10 data bytes SSP1STAT (xxDPSRxB)
 */

/*
I2C_Start();
I2CError = I2C_MasterWrite(DriverAddress & 0xFE);
I2CError = I2C_MasterWrite(Amplitude) | I2CError;
I2C_Stop();
 */
unsigned char  I2C_Write(unsigned char Address, unsigned char TxData[], unsigned char length)
{
    unsigned char idx;

    // START
    // send start condition and wait for the hardware to clear the bit
    SEN = 1;
    while(SEN==1);
    while(SSP1IF == 0);

    // send Address
    while(SSP1IF==1)
    {
        SSP1IF = 0;
        SSP1BUF = Address & 0xFE;
        //while(BF == 1);                 // wait for SSP1BUF to send all data out
        while ( ( SSP1CON2 & 0x1F ) || ( SSP1STAT & 0x04 ) );
        if(ACKSTAT == 1)
        {
            I2C_Error();
            return 1;
        }
    }



    for(idx=0; idx<length; idx++)
    {

        while(SSP1IF==1)
        {
            SSP1IF = 0;
            SSP1BUF = TxData[idx];
            //while(BF == 1);                 // wait for SSP1BUF to send all data out
            while ( ( SSP1CON2 & 0x1F ) || ( SSP1STAT & 0x04 ) );
            if(ACKSTAT == 1)
            {
                I2C_Error();
                return 1;
            }
        }                       // clear write collision bit

//        while(WCOL == 1)
//        {
//            WCOL = 0;
//            SSP1BUF = TxData[idx];      // send byte of data on I2C bus
//        }
    }

//    while(BF == 1);             // wait for SSP1BUF to send all data out
//    while ( ( SSP1CON2 & 0x1F ) || ( SSP1STAT & 0x04 ) );
//    if(ACKSTAT != 1)
//    {
//        PEN = 1;
//        return 2;
//    }

    // Send stop condition
    PEN  = 1;
    while(PEN==1);
    SSP1IF = 0;
    return 0;
}



/* Function: void I2C_Exit(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to handle the errors with I2C communications */
void I2C_Error(void)
{
    unsigned char temp;
    
    temp = SSP1BUF;
    SSP1OV = 0;                         // clear overflow bit
    SSP1IF = 0;                         // Reset Interrupt Flag
    CKP = 1;                            // Clock high - Enable Clocking

}   // end of I2C_Error


/* Function: void I2C_Exit(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to handle the exit from I2C communications */
void I2C_Exit(void)
{
    unsigned char temp;

    while(SSP1OV==1)                    // check for overflow condition
    {
        SSP1OV = 0;
        temp = SSP1BUF;                 // empty buffer
    }
}   // end of I2C_Exit