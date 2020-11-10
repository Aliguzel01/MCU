/*
 * 002led_button.c
 *
 *  Created on: 18 Ağu 2020
 *      Author: HP
 */

#include "stm32f407xx.h"

#define HIGH    1
#define BTN_PRESSED  HIGH

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i++);
}


int main(void)
{

	GPIO_Handle_t GpioLed,GpioBtn;

	//THİS İS LED CONFİGURATİON
	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD,ENABLE);

	GPIO_Init(&GpioLed);

	//THİS İS BUUTON CONFİGURATİON
	GpioBtn.pGPIOx=GPIOA;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_0;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_IN;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed=GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl=GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOA, ENABLE);
	GPIO_Init(&GpioBtn);

	while(1)
	{
		if(GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0)==BTN_PRESSED)
		{
         delay();
		GPIO_ToggleOutputPin(GPIOD,GPIO_PIN_NO_12);

		}
	}
	return 0;
}
