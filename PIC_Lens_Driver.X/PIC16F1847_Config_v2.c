/* Drone Configuration file to configure the slave drones on intial startup
 *
 * All code written for the PIC16F1823
 */

/*******************************************INCLUDES***********************************************/
#include <xc.h>
#include <stdio.h>
//#include "Drone_Slave.h"
#include "PIC16F1847_Config.h"
//#include "drone_I2C.h"


/* Function: void register_config(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to handle the initial register configuration of the PIC */
void register_config(void)
{
    OSCCON = 0b11110000;                // Sets internal oscillator to 32 MHz - pg65
    OPTION_REG = 0b10000101;            // Turns internal pull up resistors off, Prescaler 1:64 - pg175
    INTCON = 0b11000000;                // Enable global & peripheral interrupts - pg88
    PIE1 = 0b00100000;                  // Enables USART Rx interrupt - pg89
    WDTCON = 0b00011101;                // Watchdog Timer, 1:2^19 (Interval ~16s typ), On - pg101
    APFCON0 = 0b10000000;               // Set EUSART RX to RB2, - pg118
    APFCON1 = 0b00000001;               // Set EUSART TX to RB5 - pg118
    ANSELA = 0b00000000;                // Sets all Port A pins to digital mode - pg122
    ANSELB = 0b00000000;                // Sets all Port B pins to digital mode - pg127
    WPUA = 0b00000000;                  // Port A Weak pull-up resistor - 0=disabled - pg121
    WPUB = 0b00000000;			// Port B Weak pull-up resistor - 0=disabled - pg127
    FVRCON = 0b00000000;                // Fixed Voltage Ref Ctrl - pg134
    ADCON0 = 0b00000000;                // Sets A/D parameters - pg143
    ADCON1 = 0b00000000;                // Sets A/D parameters - pg144
    CPSCON0 = 0b00000000;               // Capacitive Touch disable - pg323
    CPSCON1 = 0b00000000;               // Capacitive Touch disable - pg324
    // TRISA = 0b00011000;                 // Set these pins according to the data direction
    // TRISC = 0b00100111;                 // Set these pins according to the data direction
}   // end of register_config


/* Function: void MSSP_config(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to configure the MSSP/I2C register configuration of the PIC */
void MSSP_config(void)
{
    //SSP1ADD = 0b01001111;               // Fclock = 100kHz, 32MHz Fosc, BRG = 79 - pg286
    SSP1ADD = 0b00100111;               // Fclock = 200kHz, 32MHz Fosc, BRG = 39 - pg286
    //SSP1ADD = 0b00011111;               // Fclock = 250kHz, 32MHz Fosc, BRG = 31 - pg286
    
    SSP1STAT = 0b10000000;              // SMP: Slew rate control disabled,
                                        // CKE: Transmit occurs on transition from Idle to active clock state - pg281
    SSP1CON1 = 0b00101000;              // Enable I2C, Enable Clock, I2C Master clock = FOSC/(4 * (SSPxADD+1)) - pg283
    SSP1CON2 = 0b00000000;              // General call disabled, Start condition Idle - pg284
    SSP1CON3 = 0b00001000;              // I2C Start Condition Interrupt disabled - pg285
}


/* Function: void UART_config(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to configure the UART registers on the PIC */
void UART_config(void)
{
    TXSTA = 0b00100100;                 // 8-bit Tx, Enable Tx, Async mode, BRGH = (1) High speed - pg296
    RCSTA = 0b10010000;                 // Enable Serial port, 8-bit Rx, Enable receiver - pg297
    BAUDCON = 0b01001000;               // TX non-inverted data, 16-bit BRG, Auto-Baud disabled - pg298
    SPBRG = 31;                         // BRGH = 1, BRG = 1 => Fosc/[4*(n+1)]; Fosc = 32MHz - pg303
                                        // 68 => 115.2k baud rate, 0.64% error 
                                        // 39 => 200k baud rate, 0.0% error
                                        // 31 => 250k baud rate, 0.0% error
}   // end of UART_config


/* Function: void EE_config(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to configure reading/writing of the EEPROM on the PIC */
void EE_config(void)
{
    EECON1 = 0b00000000;                // Accesses EEPROM - pg115

}   // end of EE_config


/* Function: void Timer_config(void)
 *
 * Arguments:
 * 1. None
 *
 * Return Value:
 * 1. None
 *
 * Description: Code to configure Timers for dealys on the PIC */
void Timer_config(void)
{
    T1CON = 0b00110101;                 // Timer on, Source = FOSC/4, Prescaler = 1:8 - pg185
    T1GCON = 0b00000001;                // Timer0 overflow output - pg186
    TMR1IF = 0;                         // No Overflow
    TMR1 = 0;                           // Reset Counter - Each increment is 1us w/32MHz
    
    T2CON = 0b00000111;                 // Timer2 is on, Prescaler is 64 - pg190
    PR2 = 125;                          // Set period register for 1ms for FOSC = 32MHz - Figure 22-1

}   // end of Timer_config

