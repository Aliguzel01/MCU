/*
 * spi_cmd_handling.c
 *
 *  Created on: 26 Ağu 2020
 *      Author: HP
 */
#include "stm32f407xx.h"
#include<string.h>

//COMMAND CODES
#define COMMAND_LED_CTRL         0x50
#define COMMAND_SENSOR_READ      0x51
#define COMMAND_LED_READ         0x52
#define COMMAND_PRINT            0x53
#define COMMAND_ID_READ          0x54

#define LED_ON     1
#define LED_OFF    0

//arduino analog pins
#define ANALOG_PIN0         0
#define ANALOG_PIN1         1
#define ANALOG_PIN2         2
#define ANALOG_PIN3         3
#define ANALOG_PIN4         4

//arduino led
#define LED_PIN     9

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i++);
}


/*
 * PB14-->SPI2_MISO
 * PB15-->SPI2_MOSI
 * PB13-->SPI2_SCLK
 * PB12-->SPI2_NSS
 * ALT. FUNCTİON MODE: 5
 * datasheet belgesinden alternate function mode şemasına bakabiliriz pim yerleri için
 */

void SPI_GPIOInits(void)
{
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx=GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode=5;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType=GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl=GPIO_PIN_PU;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_SPEED_FAST;

	//SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);


	//MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_14;
    GPIO_Init(&SPIPins);

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);
}


void SPI2_Inits(void)
{
	SPI_Handle_t SPI2handle;

	SPI2handle.pSPIx=SPI2;
	SPI2handle.SPIConfig.SPI_BusConfig=SPI_BUS_CONFIG_FD;
	SPI2handle.SPIConfig.SPI_DeviceMode=SPI_DEVICE_MODE_MASTER;
	SPI2handle.SPIConfig.SPI_SclkSpeed=SPI_SCLK_SPEED_DIV8;//generates sclk of 2MHz
	SPI2handle.SPIConfig.SPI_DFF=SPI_DFF_8BITS;
	SPI2handle.SPIConfig.SPI_CPOL=SPI_CPOL_LOW;
	SPI2handle.SPIConfig.SPI_CPHA=SPI_CPHA_LOW;
	SPI2handle.SPIConfig.SPI_SSM=SPI_SSM_DI; //hardware slave management enabled for NSS pin

	SPI_Init(&SPI2handle);

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

uint8_t SPI_VerifyResponse(uint8_t ackbyte)
{
	if(ackbyte==0xF5)
	{
		//ack
		return 1;
	}
	return 0;//nack

}


int main(void)
{
	 uint8_t dummy_write=0xff;
	 uint8_t dummy_read;

	GPIO_ButtonInit();

	// bu işlev, GPIO pinlerini SPI2 pinleri gibi davranacak şekilde başlatmak için kullanılır
	SPI_GPIOInits();

	// Bu işlev, SPI2 çevre birim parametrelerini başlatmak için kullanılır
	SPI2_Inits();

	/*
		* SSOE 1 yapmak NSS çıkışını etkinleştirir.
		* NSS pini, donanım tarafından otomatik olarak yönetilir.
		* yani SPE = 1 olduğunda, NSS düşük seviyeye çekilecektir
		* ve NSS pini SPE = 0 olduğunda yüksek olacaktır
		*/
     SPI_SSOEConfig(SPI2, ENABLE);

     while(1)
     {
    	// düğmeye basılıncaya kadar bekleyin
     while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

     //düğmenin geri sıçramasıyla ilgili sorunları önlemek için 200 ms gecikme
     delay();

	// SPI2 çevre birimini etkinleştirin
       SPI_PeripheralControl(SPI2, ENABLE);

     //1. CMD_LED_CTRL  	<pin no(1)>     <value(1)>
       uint8_t commandcode=COMMAND_LED_CTRL;
       uint8_t ackbyte;
       uint8_t args[2];
       //send command
       SPI_SendData(SPI2,&commandcode, 1);

       //RXNE yi temizlemek için kukla okuma yap
       SPI_ReceiveData(SPI2,&dummy_read,1);

       //Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
       SPI_SendData(SPI2,&dummy_write, 1);

       //alınan ack baytını oku
       SPI_ReceiveData(SPI2,&ackbyte,1);

     if( SPI_VerifyResponse(ackbyte) )
     {
    	 //send arguments
    	 args[0]=LED_PIN;
    	 args[1]=LED_ON;
    	 SPI_SendData(SPI2,args, 2);
     }
     //end of COMMAND_LED_CTRL

     //2. CMD_SENOSR_READ   <analog pin number(1) >

    	// düğmeye basılıncaya kadar bekleyin
      while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

      //düğmenin geri sıçramasıyla ilgili sorunları önlemek için 200 ms gecikme
      delay();

     commandcode=COMMAND_SENSOR_READ;
     //send command
     SPI_SendData(SPI2,&commandcode, 1);

     //  RXNE yi temizlemek için kukla okuma yap
     SPI_ReceiveData(SPI2,&dummy_read,1);

     //Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
     SPI_SendData(SPI2,&dummy_write, 1);

     // //alınan ack baytını oku
     SPI_ReceiveData(SPI2,&ackbyte,1);

     if( SPI_VerifyResponse(ackbyte) )
     {
    	 args[0]=ANALOG_PIN0;
    	 //arguman gönderme
    	 SPI_SendData(SPI2,args,1); //bir bayt gönderme
    	 //  RXNE yi temizlemek için kukla okuma yap
    	 SPI_ReceiveData(SPI2,&dummy_read, 1);

    	 // Slave'in verilerle hazır olabilmesi için biraz gecikme ekleyin
    	 delay();

         //Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
         SPI_SendData(SPI2,&dummy_write, 1);

         uint8_t analog_read;
         SPI_ReceiveData(SPI2,&analog_read, 1);

     }

     // 3. CMD_LED_READ <pin no (1)>

 	// düğmeye basılıncaya kadar bekleyin
   while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

   //düğmenin geri sıçramasıyla ilgili sorunları önlemek için 200 ms gecikme
   delay();

     commandcode=COMMAND_LED_READ;
     //send command
     SPI_SendData(SPI2,&commandcode, 1);

     //  RXNE yi temizlemek için kukla okuma yap
     SPI_ReceiveData(SPI2,&dummy_read,1);

     //Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
		SPI_SendData(SPI2, &dummy_write, 1);

		// //alınan ack baytını oku
	SPI_ReceiveData(SPI2, &ackbyte, 1);

	if (SPI_VerifyResponse(ackbyte))
	{
		args[0] = LED_PIN;

		//arguman gönderme
		SPI_SendData(SPI2, args, 1);

		//  RXNE yi temizlemek için kukla okuma yap
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Slave'in verilerle hazır olabilmesi için biraz gecikme ekleyin
		delay();

		//Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
		SPI_SendData(SPI2, &dummy_write, 1);

		uint8_t led_status;
		SPI_ReceiveData(SPI2, &led_status, 1);
 }

	//4. CMD_PRINT 		<len(2)>  <message(len) >
		// düğmeye basılıncaya kadar bekleyin
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0))
			;

		//düğmenin geri sıçramasıyla ilgili sorunları önlemek için 200 ms gecikme
		delay();

		commandcode = COMMAND_PRINT;
		//send command
		SPI_SendData(SPI2, &commandcode, 1);

		//  RXNE yi temizlemek için kukla okuma yap
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		//Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
		SPI_SendData(SPI2, &dummy_write, 1);

		// //alınan ack baytını oku
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		uint8_t message[] = "Hello ! How are you ??";
		if (SPI_VerifyResponse(ackbyte)) {
			args[0] = strlen((char*) message);

			//argüman gönderme
			SPI_SendData(SPI2, args, 1); //gönderme uzunlugu

			SPI_SendData(SPI2, message, args[0]);
		}

	// 5. CMD_ID_READ

	commandcode=COMMAND_ID_READ;
	//send command
	SPI_SendData(SPI2,&commandcode,1);

	//  RXNE yi temizlemek için kukla okuma yap
	SPI_ReceiveData(SPI2,&dummy_read,1);

	//Kölenin yanıtını almak için bazı sahte bitler gönderin(1 byte)
	SPI_SendData(SPI2,&dummy_write, 1);

	// //alınan ack baytını oku
	SPI_ReceiveData(SPI2,&ackbyte,1);

	uint8_t id[11];
	uint32_t i=0;
	if( SPI_VerifyResponse(ackbyte) )
	{
		// slave'den 10 bayt kimliği oku
		for(i=0;i<10;i++)
		{
			// Slave'den veri almak için sahte bayt gönder
			SPI_SendData(SPI2,&dummy_write,1);
			SPI_ReceiveData(SPI2,&id[i],1);
		}
		id[11]='\0';

	}



	// SPI'nın meşgul olmadığını doğrulayalım
	while (SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG));

	// SPI2 çevre birimini devre dışı bırakın
	SPI_PeripheralControl(SPI2, DISABLE);


     }


	return 0;
}

