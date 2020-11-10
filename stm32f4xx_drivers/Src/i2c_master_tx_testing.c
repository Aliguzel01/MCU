/*
 * i2c_master_tx_testing.c
 *
 *  Created on: 4 Eyl 2020
 *      Author: HP
 */


#include "stm32f407xx.h"
#include<string.h>
#include<stdio.h>

#define MY_ADDR        0x61
#define SLAVE_ADDR     0x68     /*AR duino köle oldugu için onun adresi serial monitoru açınca yazıyor orda*/

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i++);
}

I2C_Handle_t I2C1Handle;

//Some data
uint8_t some_data[]="We are testing I2C master Tx\n";
/*
 * PB6-->SCL
 * PB9-->SDA
 */

void I2C1_GPIOInits(void)
{
	GPIO_Handle_t I2CPins;

	I2CPins.pGPIOx=GPIOB;
    I2CPins.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConfig.GPIO_PinOPType=GPIO_OP_TYPE_OD;
    I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl=GPIO_PIN_PU;
    I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode=4;
    I2CPins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_SPEED_FAST;

    //SCL
    I2CPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_6;
    GPIO_Init(&I2CPins);

    //SDA
    I2CPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_9;/*bu pimi PB7 İLE degis ders 199 notunu oku anlarsın*/
    GPIO_Init(&I2CPins);

}


void I2C1_Inits(void)
{
  I2C1Handle.pI2Cx=I2C1;
  I2C1Handle.I2C_Config.I2C_ACKControl=I2C_ACK_ENABLE;
  I2C1Handle.I2C_Config.I2C_DeviceAddress=MY_ADDR;
  I2C1Handle.I2C_Config.I2C_FMDutyCycle=I2C_FM_DUTY_2;
  I2C1Handle.I2C_Config.I2C_SCLSpeed=I2C_SCL_SPEED_SM;

  I2C_Init(&I2C1Handle);


}

void GPIO_ButtonInit(void)
{
	GPIO_Handle_t  GpioBtn,GpioLed;
	//THİS İS BUUTON CONFİGURATİON
	GpioBtn.pGPIOx = GPIOA;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioBtn);

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

	//enable the i2c peripheral
	I2C_PeripheralControl(I2C1, ENABLE);

	while(1)
	{
		// düğmeye basılıncaya kadar bekleyin
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0))
			;

		//düğmenin geri sıçramasıyla ilgili sorunları önlemek için 200 ms gecikme
		delay();

		//köleye bazı veriler gönder
		I2C_MasterSendData(&I2C1Handle, some_data, strlen((char*) some_data),SLAVE_ADDR);
	}




}
