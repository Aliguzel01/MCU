/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: 21 Ağu 2020
 *      Author: HP
 */

#include "stm32f407xx_spi_driver.h"

/*static anahtar sözcüğü kullan aslında bu fonksiyonların özel yardımcı işlev oldugunu belirtmek
için kullanıyoruz.yani spi_driver.c sürücüsüne özeldir bu kullanıcı bunu çağırmaya kalkarsa
derleyici hata verir bu fonksiyonları   SPI_IRQHandling() fonksiyonunda kullandık. */

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);




/*
 * Periferik Saat kurulumu
 */

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
   if(EnorDi==ENABLE)
   {
	   if(pSPIx==SPI1)
	   {
		   SPI1_PCLK_EN();
	   }else if(pSPIx==SPI2)
	   {
		   SPI2_PCLK_EN();
	   }else if(pSPIx==SPI3)
	   {
		   SPI3_PCLK_EN();
	   }
   }
   else
   {

   }

}

/*
 * Init ve De-init
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{

	//peripheral clock enable
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

//ilk olarak SPI_CR1 kaydını yapılandıralım.

	uint32_t tempreg=0;

	//1. device mode yapılandır.
	tempreg |=pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR;

	//2. bus config i yapılandır
	if(pSPIHandle->SPIConfig.SPI_BusConfig==SPI_BUS_CONFIG_FD)
	{
		//bidi mode should be cleraed
		tempreg &=~(1 << SPI_CR1_BIDIMODE );

	}else if(pSPIHandle->SPIConfig.SPI_BusConfig==SPI_BUS_CONFIG_HD)
	{
		//bidi mode should be set
		tempreg |=( 1 << SPI_CR1_BIDIMODE);
	}else if(pSPIHandle->SPIConfig.SPI_BusConfig==SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		//BIDI mode should be cleared
		tempreg &=~(1 << SPI_CR1_BIDIMODE);
		//RXONLY bit must be set (10. bit RXONLY)
		tempreg |=( 1 << SPI_CR1_RXONLY );
	}

	//3. configure the spi serial clock speed(baud rate)
	tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;

	//4.configure the DFF
	tempreg |=pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF ;

	//5. CONFİGURE THE CPOL
	tempreg |=pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;

	//6.configure the CPHA
	tempreg |=pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

	//7.configure the SSM
	tempreg |=pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;

	pSPIHandle->pSPIx->CR1 =tempreg;
}




void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
  if(pSPIx==SPI1)
  {
	  SPI1_REG_RESET();
  }else if(pSPIx==SPI2)
  {
	  SPI2_REG_RESET();
  }else if(pSPIx==SPI3)
  {
	  SPI3_REG_RESET();
  }

}


uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if(pSPIx->SR & SPI_TXE_FLAG)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}




/*
 * DATA SEND AND RECEİVE
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len)
{
	// 1. TXE ayarlanana kadar bekleyin
	while ( SPI_GetFlagStatus (pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

	// 2. CR1'deki DFF bitini kontrol edin
	if(pSPIx->CR1 & (1 <<SPI_CR1_DFF))
	{
		// 16 bit DFF
		// 1. Verileri DR'ye yükleyin
		pSPIx->DR = *((uint16_t*)pTxBuffer);
		len--;           /*2 bay veri gönderdiğimiz için 2 defa azaltıyoruz*/
		len--;
		(uint16_t*)pTxBuffer++;  /*bir sonraki veriyi görmemiz için arttırma yaptık */

	}else
	{
		// 8 bit DFF
		pSPIx->DR =*pTxBuffer;
		len--;
		pTxBuffer++;
	}
}




void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len)
{
	// 1. RXNE ayarlanana kadar bekleyin
	while ( SPI_GetFlagStatus (pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

	// 2. CR1'deki DFF bitini kontrol edin
	if(pSPIx->CR1 & (1 <<SPI_CR1_DFF))
	{
		// 16 bit DFF
		// 1. verileri DR'den Rxbuffer adresine yükleyin
		*((uint16_t*)pRxBuffer)=pSPIx->DR ;
		len--;
		len--;
		(uint16_t*)pRxBuffer++;  /*bir sonraki veriyi görmemiz için arttırma yaptık */

	}else
	{
		// 8 bit DFF
		*(pRxBuffer) = pSPIx->DR;
		len--;
		pRxBuffer++;
	}


}



/*
 * SPI peripheral control enable or disable
 */

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pSPIx->CR1 |=( 1 << SPI_CR1_SPE);
	}else
	{
		pSPIx->CR1 &=~( 1 << SPI_CR1_SPE);
	}


}


void  SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pSPIx->CR1 |=( 1 << SPI_CR1_SSI);
	}else
	{
		pSPIx->CR1 &=~( 1 << SPI_CR1_SSI);
	}

}


void  SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pSPIx->CR2 |=( 1 << SPI_CR2_SSOE);
	}else
	{
		pSPIx->CR2 &=~( 1 << SPI_CR2_SSOE);
	}


}



/*
 * IRQ configuration and ISR handling
 */

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (IRQNumber <= 31) {
			//program ISER0 register
			*NVIC_ISER0 |= (1 << IRQNumber);

		} else if (IRQNumber > 31 && IRQNumber < 64) //32 to 63
				{
			//program ISER1 register
			*NVIC_ISER1 |= (1 << (IRQNumber % 32));
		} else if (IRQNumber >= 64 && IRQNumber < 96) {
			//program ISER2 register //64 to 95
			*NVIC_ISER3 |= (1 << (IRQNumber % 64));
		}
	} else
	{
		if (IRQNumber <= 31) {
			//program ICER0 register
			*NVIC_ICER0 |= (1 << IRQNumber);
		} else if (IRQNumber > 31 && IRQNumber < 64) {
			//program ICER1 register
			*NVIC_ICER1 |= (1 << (IRQNumber % 32));
		} else if (IRQNumber >= 64 && IRQNumber < 96) {
			//program ICER2 register
			*NVIC_ICER3 |= (1 << (IRQNumber % 64));
		}
	}

}

void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//İLK önce ipr kaydini bulalim
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);


}




uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pTxBuffer,uint32_t len)
{

	uint8_t state=pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX)
	{
	// 1. Tx arabellek adresini ve Len bilgilerini bazı global değişkenlere kaydedin
	pSPIHandle->pTxBuffer=pTxBuffer;
	pSPIHandle->TxLen=len;
	// 2. SPI durumunu iletimde meşgul olarak işaretleyin, böylece
	//     iletim bitene kadar başka hiçbir kod aynı SPI çevre birimini devralamaz
	pSPIHandle->TxState=SPI_BUSY_IN_TX;
	// 3. SR'de TXE bayrağı her ayarlandığında kesintiye uğramak için TXEIE kontrol bitini etkinleştirin
    pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);

	}

	return state;
}


uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pRxBuffer,uint32_t len)
 {
	uint8_t state = pSPIHandle->RxState;
	if (state != SPI_BUSY_IN_RX)
	{
		// 1. Rx arabellek adresini ve Len bilgilerini bazı global değişkenlere kaydedin
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = len;
		// 2. SPI durumunu alımda meşgul olarak işaretleyin, böylece
		//     Alım bitene kadar başka hiçbir kod aynı SPI çevre birimini devralamaz
		pSPIHandle->RxState = SPI_BUSY_IN_RX;
		// 3. SR'de RXNEIE bayrağı her ayarlandığında kesme almak için RXNEIE kontrol bitini etkinleştirin
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);

	}

	return state;
}








void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
   uint8_t temp1,temp2;

   //İlk TXE bayragını kontrol edelim
   temp1=pHandle->pSPIx->SR & (1 << SPI_SR_TXE);
   temp2=pHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);

   if( temp1 && temp2)
   {
	   //handle TXE
	   spi_txe_interrupt_handle(pHandle);
   }

   // RXNE bayragını kontrol edelim
   temp1=pHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
   temp2=pHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);

   if( temp1 && temp2)
   {
	   //handle RXNE
	   spi_rxne_interrupt_handle(pHandle);
   }

   // OVR bayragını kontrol edelim
   temp1=pHandle->pSPIx->SR & (1 << SPI_SR_OVR);
   temp2=pHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);

   if( temp1 && temp2)
   {
	   //handle  ovr error
	   spi_ovr_err_interrupt_handle(pHandle);
   }


}



//bazı yardımcı fonksiyon uygulamaları

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//  CR1'deki DFF bitini kontrol edin
	if(pSPIHandle->pSPIx->CR1 & (1 <<SPI_CR1_DFF))
	{
		// 16 bit DFF
		// 1. Verileri DR'ye yükleyin
		pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen--;           /*2 bay veri gönderdiğimiz için 2 defa azaltıyoruz*/
		pSPIHandle->TxLen--;
		(uint16_t*)pSPIHandle->pTxBuffer++;  /*bir sonraki veriyi görmemiz için arttırma yaptık */

	}else
	{
		// 8 bit DFF
		pSPIHandle->pSPIx->DR =*pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}
	if( !pSPIHandle->TxLen)
	{
		// TxLen sıfırdır, bu nedenle spi iletimini kapatın ve uygulamaya
		// TX bitti.

		// bu, kesintilerin TXE bayrağını ayarlamasını önler
        SPI_CloseTransmission(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
	}

}




static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	// dff'ye göre rxing yapın
	if(pSPIHandle->pSPIx->CR1 & ( 1 << 11))
	{
		//16 bit
		*((uint16_t*)pSPIHandle->pRxBuffer) = (uint16_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -= 2;
		pSPIHandle->pRxBuffer++;
		pSPIHandle->pRxBuffer++;

	}else
	{
		//8 bit
		*(pSPIHandle->pRxBuffer) = (uint8_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}

	if(! pSPIHandle->RxLen)
	{
		//reception is complete
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
	}


}




static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	// 1. ovr bayrağını sil
	if( pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		pSPIHandle->pSPIx->DR;
		pSPIHandle->pSPIx->SR;

	}
    (void)temp;
	// 2. uygulamayı bilgilendirin
	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVR_ERR);

}



void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer=NULL;
	pSPIHandle->TxLen=0;
	pSPIHandle->TxState=SPI_READY;
}


void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &=~(1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer=NULL;
	pSPIHandle->RxLen=0;
	pSPIHandle->RxState=SPI_READY;

}


void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;
	temp=pSPIx->DR;
	temp=pSPIx->SR;
    (void)temp;
}


__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{

	// Bu zayıf bir uygulama. kullanıcı uygulaması bu işlevi geçersiz kılabilir.
}

