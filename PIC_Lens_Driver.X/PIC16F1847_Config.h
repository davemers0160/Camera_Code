/* 
 * File:   PIC16F1847_Config.h
 * Author: Owner
 *
 * Created on September 27, 2013, 6:33 AM
 */

#ifndef PIC16F1847_CONFIG_H
#define	PIC16F1847_CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

/*******************************************DEFINES************************************************/
#define SSP1IE          PIE1bits.SSP1IE         /* MSSP Interrupt enable bit - pg89 */
#define SSP1IF          PIR1bits.SSP1IF         /* MSSP Interrupt Flag bit - pg93 */
#define DA              SSP1STATbits.D_nA       /* I2C Data/Address Bit indicator - pg281 */
#define P               SSP1STATbits.P          /* I2C Stop bit - pg281 */
#define S               SSP1STATbits.S          /* I2C Start bit - pg281 */
#define RW              SSP1STATbits.R_nW       /* I2C Read/Write Status Bit - pg281 */
#define BF              SSP1STATbits.BF         /* I2C buffer full indicator - pg281 */
#define WCOL            SSP1CON1bits.WCOL       /* I2C Write Collision Detect Bit - pg283 */
#define SSP1OV          SSP1CON1bits.SSPOV      /* I2C Overflow Indicator Bit - pg283 */
#define SSP1EN          SSP1CON1bits.SSPEN      /* I2C Synchronous Serial Port Enable Bit - pg283 */
#define CKP             SSP1CON1bits.CKP        /* I2C SCK Release Control - pg283 */
#define ACKSTAT         SSP1CON2bits.ACKSTAT    /* I2C ACK/NACK Indicator bit - pg284 */
#define PEN             SSP1CON2bits.PEN        /* I2C Stop Condition Enable bit - pg284 */
#define SEN             SSP1CON2bits.SEN        /* I2C Start Condition Enable bit - pg284 */

#define RCIE            PIE1bits.RCIE           /* USART RX Interrupt Enable bit - pg89 */
#define RCIF            PIR1bits.RCIF           /* USART Receive Interrupt Flag bit - pg93 */
#define TX_BUF          TXSTAbits.TRMT          /* USART TX Buffer full bit - pg296 */
#define CREN            RCSTAbits.CREN          /* USART RX Overflow indicator - pg297 */
#define FERR            RCSTAbits.FERR          /* USART RX Framing Error bit - pg297 */
#define OERR            RCSTAbits.OERR          /* USART RX Overrun Error bit - pg297 */

#define EE_RD           EECON1bits.RD           /* EEPROM Read Control bit - pg115 */
#define EE_WR           EECON1bits.WR           /* EEPROM Write Control bit - pg115 */
#define EE_WREN         EECON1bits.WREN         /* EEPROM Write Enable bit - pg115 */
#define EE_ERR          EECON1bits.WRERR        /* EEPROM Write Error status bit - pg115 */


/*************************************FUNCTION DECLARATIONS****************************************/
void register_config(void);
void MSSP_config(void);
void UART_config(void);
void EE_config(void);
void Timer_config(void);


#ifdef	__cplusplus
}
#endif

#endif	/* PIC16F1847_CONFIG_H */

