#include "i2c.h"

void	I2C_Init(void)
{
	TWBR = TWBRX;
	TWSR = TWPSX;		
    TWCR = 0x00;
}

uint8_t I2C_Start( void )
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	
	while (!(TWCR & (1<<TWINT)));
	
	// if ((TWSR & 0xF8) != TW_START)
	// 	return 0;
	
	return 1;
}

uint8_t I2C_Write( uint8_t data )
{
	TWDR = data;
	
	TWCR = (1<<TWINT)|(1<<TWEN) ;
	
	while (!(TWCR & (1<<TWINT)));
	
	// if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
	// 	return 0;
	
	return 1;
}

uint8_t I2C_ReStart( void )
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	
	while (!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) != TW_REP_START)
		return 0;
	
	return 1;
}

uint8_t I2C_Read( uint8_t ACK_NACK)
{
	while (!(TWCR & (1<<TWINT)));
	
	TWCR = (ACK_NACK)?((1<<TWINT)|(1<<TWEN)|(1<<TWEA)):((1<<TWINT)|(1<<TWEN));
	
	while (!(TWCR & (1<<TWINT)));
	
	return	TWDR;
}

void I2C_Stop( void )
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while(TWCR & (1<<TWSTO));
}
