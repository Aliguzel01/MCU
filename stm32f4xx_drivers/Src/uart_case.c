/*
 * uart_case.c
 *
 *  Created on: 21 Eyl 2020
 *      Author: HP
 */

#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"



//arduino'ya ilettiğimiz 3 farklı mesajımız var
//elbette daha fazla mesaj ekleyebiliriz
char *msg[3] = {"hihihihihihi123","Hello How are you ?","Today is Monday !"};

// arduino'dan gelen yanıt burada saklanacak
char rx_buf[1024];

USART_Handle_t usart2_handle;


//Bu bayrak, alımın tamamlandığını gösterir
uint8_t rxCmplt = RESET;

uint8_t g_data = 0;

extern void initialise_monitor_handles();

void USART2_Init(void)
{
	usart2_handle.pUSARTx = USART2;
	usart2_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;
	usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
	usart2_handle.USART_Config.USART_Mode = USART_MODE_TXRX;
	usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
	usart2_handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
	usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
	USART_Init(&usart2_handle);
}

void 	USART2_GPIOInit(void)
{
	GPIO_Handle_t usart_gpios;

	usart_gpios.pGPIOx = GPIOA;
	usart_gpios.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart_gpios.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	usart_gpios.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	usart_gpios.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	usart_gpios.GPIO_PinConfig.GPIO_PinAltFunMode =7;

	usart_gpios.GPIO_PinConfig.GPIO_PinNumber  = GPIO_PIN_NO_2;
	GPIO_Init(&usart_gpios);

	usart_gpios.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GPIO_Init(&usart_gpios);

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

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
}
int main(void)
{
	uint32_t cnt = 0;


	initialise_monitor_handles();

	USART2_GPIOInit();
    USART2_Init();

    USART_IRQInterruptConfig(IRQ_NO_USART2,ENABLE);

    USART_PeripheralControl(USART2,ENABLE);

    printf("Application is running\n");

    //sonsuza kadar yap
    while(1)
    {
		//	düğmeye basılana kadar bekleyin
		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

		//Düğmenin geri gelmesiyle ilgili sorunları önlemek için 200 ms gecikme
		delay();

		// Sonraki mesaj dizini; cnt değerinin 2'yi geçmediğinden emin olun
		cnt = cnt % 3;

		//İlk önce alımı kesme modunda etkinleştirelim
		// bu kod alma kesintisini etkinleştirir
		while ( USART_ReceiveDataIT(&usart2_handle,(uint8_t*)rx_buf,strlen(msg[cnt])) != USART_READY );

		// 	Engelleme modunda cnt tarafından indekslenen msg'yi gönder
    	USART_SendData(&usart2_handle,(uint8_t*)msg[cnt],strlen(msg[cnt]));

    	printf("Transmitted : %s\n",msg[cnt]);


    	// Şimdi arduino'dan tüm baytlar alınana kadar bekleyelim.
    	// Tüm baytlar alındığında rxCmplt, uygulama geri aramasında ayarlanacaktır.
    	while(rxCmplt != SET);

    	// sadece son baytın boş olduğundan emin olun, aksi takdirde yazdırma sırasında %s başarısız olur
    	rx_buf[strlen(msg[cnt])+ 1] = '\0';

    	// 	Arduino'dan aldığımızı yazdırın
    	printf("Received    : %s\n",rx_buf);

    	// bayrağı geçersiz kılmak
    	rxCmplt = RESET;

    	//  msg[] 'de indekslenen bir sonraki mesaja git
    	cnt ++;
    }


	return 0;
}


void USART2_IRQHandler(void)
{
	USART_IRQHandling(&usart2_handle);
}





void USART_ApplicationEventCallback( USART_Handle_t *pUSARTHandle,uint8_t ApEv)
{
   if(ApEv == USART_EVENT_RX_CMPLT)
   {
			rxCmplt = SET;

   }else if (ApEv == USART_EVENT_TX_CMPLT)
   {
	   ;
   }
}


