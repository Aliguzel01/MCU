/*
 * spi_tx_testing.c
 *
 *  Created on: 25 Ağu 2020
 *      Author: HP
 */
#include "stm32f407xx.h"
#include<string.h>


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
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl=GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_SPEED_FAST;

	//SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	/*sadece usta kullandıgımız için köle kullanmıyoruz MISO VE NSS PİNİNİ İPTAL EDELİM */
	//MISO
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_14;
	//GPIO_Init(&SPIPins);

	//NSS
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NO_12;
	//GPIO_Init(&SPIPins);
}


void SPI2_Inits(void)
{
	SPI_Handle_t SPI2handle;

	SPI2handle.pSPIx=SPI2;
	SPI2handle.SPIConfig.SPI_BusConfig=SPI_BUS_CONFIG_FD;
	SPI2handle.SPIConfig.SPI_DeviceMode=SPI_DEVICE_MODE_MASTER;
	SPI2handle.SPIConfig.SPI_SclkSpeed=SPI_SCLK_SPEED_DIV2;//generates sclk of 8MHz
	SPI2handle.SPIConfig.SPI_DFF=SPI_DFF_8BITS;
	SPI2handle.SPIConfig.SPI_CPOL=SPI_CPOL_LOW;
	SPI2handle.SPIConfig.SPI_CPHA=SPI_CPHA_LOW;
	SPI2handle.SPIConfig.SPI_SSM=SPI_SSM_EN; //software slave management enabled for NSS pin

	SPI_Init(&SPI2handle);

}


int main(void)
{
	char user_data[]="hello Word";
	// bu işlev, GPIO pinlerini SPI2 pinleri gibi davranacak şekilde başlatmak için kullanılır
	SPI_GPIOInits();

	// Bu işlev, SPI2 çevre birim parametrelerini başlatmak için kullanılır
	SPI2_Inits();

	// bu fonksiyon NSS sinyalini dahili olarak yüksek yapar ve MODF hatasını önler
    SPI_SSIConfig(SPI2, ENABLE);

	// SPI2 çevre birimini etkinleştirin
       SPI_PeripheralControl(SPI2, ENABLE);

	// veri göndermek için
	SPI_SendData(SPI2,(uint8_t*)user_data,strlen(user_data));

	// SPI'nın meşgul olmadığını doğrulayalım
     while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );

	// SPI2 çevre birimini devre dışı bırakın
	SPI_PeripheralControl(SPI2, DISABLE);

    while(1);
	return 0;
}
