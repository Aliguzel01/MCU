/*
 * spi_txonly_arduino.c
 *
 *  Created on: 25 Ağu 2020
 *      Author: HP
 */

#include "stm32f407xx.h"
#include<string.h>


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
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_14;
	//GPIO_Init(&SPIPins);

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
	GPIO_Handle_t  GpioBtn;
	//THİS İS BUUTON CONFİGURATİON
	GpioBtn.pGPIOx = GPIOA;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioBtn);

}

int main(void)
{
	char user_data[]="hello Word";

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

    /*Önce uzunluk bilgisini göndermek lazım.
		 çünkü arduino (köle)kac veri baytı gönderecegimizi bilmiyor.*/
		uint8_t dataLen = strlen(user_data);
		SPI_SendData(SPI2, &dataLen, 1);


	// veri göndermek için
	SPI_SendData(SPI2,(uint8_t*)user_data,strlen(user_data));

	// SPI'nın meşgul olmadığını doğrulayalım
     while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );

	// SPI2 çevre birimini devre dışı bırakın
	SPI_PeripheralControl(SPI2, DISABLE);

     }


	return 0;
}
