#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#define		F_SCL	 100000UL

#define		TWBRX0	 (((F_CPU/F_SCL)-16)/2)
#define		TWBRX1	 TWBRX0/4
#define		TWBRX2	 TWBRX0/16
#define		TWBRX3	 TWBRX0/64

#define		NACK	0
#define		ACK		1

#if		(TWBRX0<=0xFF)
#define		TWBRX	TWBRX0
#define		TWPSX	0
#elif	(TWBRX1<=0xFF)
#define		TWBRX	TWBRX1
#define		TWPSX	1
#elif	(TWBRX2<=0xFF)
#define		TWBRX	TWBRX2
#define		TWPSX	2
#elif	(TWBRX3<=0xFF)
#define		TWBRX	TWBRX3
#define		TWPSX	3
#else
#define		TWBRX	0
#define		TWPSX	0
#endif

void		I2C_Init(void);
uint8_t		I2C_Start(void);
uint8_t		I2C_ReStart(void);
void		I2C_Stop(void);
uint8_t		I2C_Write(uint8_t );
uint8_t		I2C_Read(uint8_t );

#endif /* I2C_H_ */