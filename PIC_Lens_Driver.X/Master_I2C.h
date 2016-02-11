/* 
 * File:   Master_I2C.h
 * Author: Owner
 *
 * Created on September 27, 2013, 8:05 AM
 */

#ifndef MASTER_I2C_H
#define	MASTER_I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

/********************************DEFINES***************************************/



//extern volatile unsigned char I2CError;

/**************************FUNCTION DECLARATIONS*******************************/
//void I2C_Check(unsigned char TxData[], unsigned char RxData[], unsigned char *RxIndex, unsigned char *TxIndex);
unsigned char I2C_Write(unsigned char Address, unsigned char TxData[], unsigned char length);
void I2C_Error(void);
void I2C_Exit(void);
void I2C_Start(void);
void I2C_Stop(void);
unsigned char I2C_MasterWrite(unsigned char data);

#ifdef	__cplusplus
}
#endif

#endif	/* MASTER_I2C_H */

