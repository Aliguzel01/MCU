/*
 * i2c_slave_tx_string.c
 *
 *  Created on: 15 Eyl 2020
 *      Author: HP
 */



#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"





#define SLAVE_ADDR  0x68

#define MY_ADDR    SLAVE_ADDR

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
}

I2C_Handle_t I2C1Handle;


uint8_t Tx_buf[32]="STM32 Slave mode testing..";

/*
 * PB6-> SCL
 * PB7 -> SDA
 */

void I2C1_GPIOInits(void)
{
	GPIO_Handle_t I2CPins;

	I2CPins.pGPIOx = GPIOB;
	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	I2CPins. GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//scl
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&I2CPins);


	//sda
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
	GPIO_Init(&I2CPins);


}

void I2C1_Inits(void)
{
	I2C1Handle.pI2Cx = I2C1;
	I2C1Handle.I2C_Config.I2C_ACKControl= I2C_ACK_ENABLE;
	I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;
	I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
	I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

	I2C_Init(&I2C1Handle);

}

void GPIO_ButtonInit(void)
{
	GPIO_Handle_t GPIOBtn,GpioLed;

	//this is btn gpio configuration
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GPIOBtn);

	//this is led gpio configuration
	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD,ENABLE);

	GPIO_Init(&GpioLed);

}

int main(void)
{

	GPIO_ButtonInit();

	//i2c pin ınit
	I2C1_GPIOInits();

	//i2c peripheral configuration
	I2C1_Inits();

	//I2C IRQ configurations
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

	I2C_SlaveEnableDisableCallbackEvents(I2C1,ENABLE);

	//enable the i2c peripheral
	I2C_PeripheralControl(I2C1, ENABLE);

	// ack biti, PE = 1'den sonra 1 yapılır. I2C çevre birimi PE 1 yapınca ack yı wtkinleştir diyor CR1 KAYDINDA ACK BİTİNİ OKU İYİCE
	I2C_ManageAcking(I2C1,I2C_ACK_ENABLE);

   while(1);

}


void I2C1_EV_IRQHandler(void)
{
	I2C_EV_IRQHandling(&I2C1Handle);

}

void I2C1_ER_IRQHandler(void)
{
	I2C_ER_IRQHandling(&I2C1Handle);

}


void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{

	static  uint8_t commandCode = 0 ;
	static   uint8_t Cnt = 0 ;

	if(AppEv == I2C_EV_DATA_REQ)
	{
		// Usta biraz veri istiyor. köle göndermek zorunda
		if(commandCode == 0x51 )
		{
			// uzunluk bilgisini ustaya gönder
			I2C_SlaveSendData(pI2CHandle-> pI2Cx , strlen (( char *) Tx_buf));
		} else  if (commandCode == 0x52 )
		{
			// Tx_buf içeriğini gönder
			I2C_SlaveSendData(pI2CHandle-> pI2Cx , Tx_buf[Cnt++]);

		}
	} else  if (AppEv == I2C_EV_DATA_RCV)
	{
		// Veriler, slave'in okumasını bekliyor. köle onu okumalı
		commandCode = I2C_SlaveReceiveData (pI2CHandle-> pI2Cx );

	} else  if (AppEv == I2C_ERROR_AF)
	{
		// Bu sadece slave txing sırasında olur.
		// Master NACK'i gönderdi. bu yüzden köle, efendinin ihtiyacı olmadığını anlamalı
		// daha fazla veri.
		commandCode = 0xff;
		Cnt = 0;
	}
	else  if (AppEv == I2C_EV_STOP)
	{
		// Bu yalnızca bağımlı alım sırasında olur.
		// Master, slave ile I2C iletişimini sonlandırdı.
	}


}

