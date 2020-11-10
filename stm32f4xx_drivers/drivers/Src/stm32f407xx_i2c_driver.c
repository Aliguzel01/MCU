/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: 2 Eyl 2020
 *      Author: HP
 */


#include "stm32f407xx_i2c_driver.h"



static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExcuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx,uint8_t SlaveAddr);
static void I2C_ExcuteAddressPhaseRead(I2C_RegDef_t *pI2Cx,uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);

static void  I2C_MasterHandleRXNEInterrupt (I2C_Handle_t * pI2CHandle);
static  void  I2C_MasterHandleTXEInterrupt (I2C_Handle_t * pI2CHandle);


static void I2C_ExcuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx,uint8_t SlaveAddr)
{
	SlaveAddr =SlaveAddr << 1;
	SlaveAddr &=~(1); //Slaveaddr is salve addres +r/nw bit=0
	pI2Cx->DR =SlaveAddr;
}


static void I2C_ExcuteAddressPhaseRead(I2C_RegDef_t *pI2Cx,uint8_t SlaveAddr)
{
	SlaveAddr =SlaveAddr << 1;
	SlaveAddr |= 1; //Slaveaddr is salve addres +r/nw bit=1
	pI2Cx->DR =SlaveAddr;
}



 static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
  pI2Cx->CR1 |=( 1 << I2C_CR1_START);
}


static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
	uint32_t dummy_read;
	//cihaz modunu kontrol et
	if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
	{
		//CİHAZ master mode
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if (pI2CHandle->RxSize == 1)
			{
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

				//ADDR bayargını temizle (read SR1 , read SR2)
				dummy_read = pI2CHandle->pI2Cx->SR1;
				dummy_read = pI2CHandle->pI2Cx->SR2;
				(void) dummy_read;
			}
		} else
		{
			//ADDR bayargını temizle (read SR1 , read SR2)
			dummy_read = pI2CHandle->pI2Cx->SR1;
			dummy_read = pI2CHandle->pI2Cx->SR2;
			(void) dummy_read;
		}

	} else
	{
		//cihaz slave mode
		//ADDR bayargını temizle (read SR1 , read SR2)
		dummy_read = pI2CHandle->pI2Cx->SR1;
		dummy_read = pI2CHandle->pI2Cx->SR2;
		(void) dummy_read;
	}
}






  void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->CR1 |=( 1 << I2C_CR1_STOP);
}



  void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx,uint8_t EnorDi)
  {
	  if(ENABLE == EnorDi)
	  {
		  pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
		  pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
		  pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
	  }else
	  {
		  pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
		  pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
		  pI2Cx->CR2 &= ~(1 << I2C_CR2_ITERREN);

	  }

  }





void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
	   if(pI2Cx==I2C1)

	   {
		   I2C1_PCLK_EN();
	   }else if(pI2Cx==I2C2)
	   {
		   I2C2_PCLK_EN();
	   }else if(pI2Cx==I2C3)
	   {
		   I2C3_PCLK_EN();
	   }
	}
	else
	{


	}

}





/*********************************************************************
 * @fn      		  - I2C_Init
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -
 */


void I2C_Init(I2C_Handle_t *pI2CHandle)
{
   uint32_t tempreg=0;

   // i2cx çevre birimi için saati etkinleştirin
   I2C_PeriClockControl(pI2CHandle->pI2Cx,ENABLE);

   //ack kontrol bit
   tempreg |=pI2CHandle->I2C_Config.I2C_ACKControl << 10;
   pI2CHandle->pI2Cx->CR1=tempreg;

   //CR2 kaydının FREQ alanını yapılandıralım
   tempreg=0;
   tempreg |= RCC_GetPCLK1Value()/1000000U;
   pI2CHandle->pI2Cx->CR2= (tempreg & 0x3F);

   //program the device address
   tempreg=0;
   tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
   tempreg |=(1 << 14);  /* referans klavuzunda yazılım tarafından 1 de tut diyor. OAR1 kaydında */
   pI2CHandle->pI2Cx->OAR1=tempreg;

   //CCR calculation
   uint16_t ccr_value=0;
   tempreg=0;
   if( pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
   {
	   //MODE İS standart mode
	   ccr_value= RCC_GetPCLK1Value()/(2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
	   tempreg |= (ccr_value & 0xFFF);
   }else
   {
	   //mode is fast mode
	   tempreg |= (1 << 15);
	   tempreg |=( pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
	   if( pI2CHandle->I2C_Config.I2C_FMDutyCycle==I2C_FM_DUTY_2)
	   {
		   ccr_value= RCC_GetPCLK1Value()/(3 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
	   }else
	   {
		  ccr_value= RCC_GetPCLK1Value()/(25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
	   }
	   tempreg |= (ccr_value & 0xFFF);

   }
   pI2CHandle->pI2Cx->CCR = tempreg;

   //TRISE yapılandırması
   if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
   {
	   //mode is standart mode

	   tempreg = (RCC_GetPCLK1Value()/1000000U )+1;

   }else
   {
	   //mode is fast mode
	   tempreg= ( (RCC_GetPCLK1Value() * 300) / 1000000000U ) + 1;
   }
   pI2CHandle->pI2Cx->TRISE = ( tempreg & 0x3F ); /*trise kaydında 6 bit gecerli oldugu için diğer bitleri maskelememiz lazım 0x3f le 've' işlemi yaparak*/

}





void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
   if(pI2Cx==I2C1)
   {
	   I2C1_REG_RESET();
   }else if(pI2Cx==I2C2)
   {
	   I2C2_REG_RESET();
   }else if(pI2Cx==I2C3)
   {
	   I2C3_REG_RESET();
   }
}


uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if( pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}


void I2C_MasterSendData(I2C_Handle_t *pI2CHandle,uint8_t *pTxbuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	// 1. BAŞLAT koşulunu oluşturun
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	// 2. SR1'deki SB bayrağını kontrol ederek başlangıç ​​oluşturmanın tamamlandığını onaylayın
	 //    Not: SB temizlenene kadar SCL uzatılacaktır (DÜŞÜK konuma çekilecektir)
     while( ! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) );

	// 3. r / nw biti w (0) (toplam 8 bit) olarak ayarlanmış şekilde slave'in adresini gönderin
     I2C_ExcuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

	// 4. SR1'deki ADDR bayrağını kontrol ederek adres aşamasının tamamlandığını onaylayın.
     while( ! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) );

	// 5. ADDR bayrağını yazılım sırasına göre sil
	 //    Not: ADDR temizlenene kadar SCL uzatılacaktır (DÜŞÜK çekilecektir)
     I2C_ClearADDRFlag(pI2CHandle);

	// 6. veriyi len 0 olana kadar gönder
     while(Len > 0)
     {
          while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) );//wait till TXE is set
          pI2CHandle->pI2Cx->DR = *pTxbuffer;
          pTxbuffer++;
          Len--;
     }
	// 7. Len sıfır olduğunda STOP koşulunu oluşturmadan önce TXE = 1 ve BTF = 1 için bekleyin
	//    Not: TXE = 1, BTF = 1, hem SR hem de DR'nin boş olduğu ve sonraki iletimin başlaması gerektiği anlamına gelir
	//    BTF = 1 SCL uzatılacak (DÜŞÜK konuma çekilecek)
     while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) );
     while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF) );

	// 8. DURDURMA koşulu oluşturun ve ana birimin durdurma koşulunun tamamlanmasını beklemesi gerekmez.
	//    Not: STOP oluşturmak, BTF'yi otomatik olarak temizler
     if(Sr==I2C_DISABLE_SR)
     I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

}



void  I2C_MasterReceiveData (I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	// 1. BAŞLAT koşulunu oluşturun
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	// 2. SR1'deki SB bayrağını kontrol ederek başlangıç ​​oluşturmanın tamamlandığını onaylayın
	//    Not: SB temizlenene kadar SCL uzatılacaktır (DÜŞÜK konuma çekilecektir)
	 while( ! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) );

	// 3. r / nw biti R (1) 'e ayarlanmış olarak slave'in adresini gönderin (toplam 8 bit)
	 I2C_ExcuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

	// 4. SR1'deki ADDR bayrağını kontrol ederek adres aşaması tamamlanana kadar bekleyin
     while( ! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR) );



	// slave'den sadece 1 bayt okuma prosedürü
	if(Len == 1)
	{
		// Onaylamayı Devre Dışı Bırak
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

		// ADDR bayrağını sil
		I2C_ClearADDRFlag(pI2CHandle);

		// RXNE 1 olana kadar bekle
		while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

		// DURDUR koşulu oluştur
		if(Sr == I2C_DISABLE_SR )
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		// verileri arabelleğe oku
	   *pRxBuffer = pI2CHandle->pI2Cx->DR;

	}



	// Len> 1 olduğunda slave'den veri okuma prosedürü
	if(Len > 1)
	{
		// ADDR bayrağını sil
		I2C_ClearADDRFlag(pI2CHandle);

		// Len sıfır olana kadar verileri okuyun
        for(uint32_t i=Len;i>0;i--)
        {
		// RXNE 1 olana kadar bekle
			while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

			if (i == 2)//eger son iki bayt kalmışsa
			{
				// Onaylamayı Devre Dışı Bırak
		       I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

				// DURDUR koşulu oluştur
		       if(Sr == I2C_DISABLE_SR )
			   I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

			 }

		// verileri veri kütüğünden arabelleğe oku
			*pRxBuffer = pI2CHandle->pI2Cx->DR;
		// arabellek adresini artır
        	pRxBuffer++;
        }




		// ACKing'i yeniden etkinleştir
    	if(pI2CHandle->I2C_Config.I2C_ACKControl== I2C_ACK_ENABLE)
    	{
    		I2C_ManageAcking(pI2CHandle->pI2Cx,I2C_ACK_ENABLE);
    	}

	}



}



void I2C_ManageAcking(I2C_RegDef_t *pI2Cx,uint8_t EnorDi)
{
	if(EnorDi == I2C_ACK_ENABLE)
	{
		//enable the ack
		pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
	}else
	{
		//dısable the ack
		pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
	}


}



uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxbuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxbuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);

	}

	return busystate;

}



uint8_t  I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);
	}

	return busystate;

}




/*
 * IRQ configuration and ISR handling
 */

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			//program ISER0 register
			*NVIC_ISER0 |= ( 1 << IRQNumber );

		}else if(IRQNumber > 31 && IRQNumber < 64 ) //32 to 63
		{
			//program ISER1 register
			*NVIC_ISER1 |= ( 1 << (IRQNumber % 32) );
		}
		else if(IRQNumber >= 64 && IRQNumber < 96 )
		{
			//program ISER2 register //64 to 95
			*NVIC_ISER3 |= ( 1 << (IRQNumber % 64) );
		}
	}else
	{
		if(IRQNumber <= 31)
		{
			//program ICER0 register
			*NVIC_ICER0 |= ( 1 << IRQNumber );
		}else if(IRQNumber > 31 && IRQNumber < 64 )
		{
			//program ICER1 register
			*NVIC_ICER1 |= ( 1 << (IRQNumber % 32) );
		}
		else if(IRQNumber >= 64 && IRQNumber < 96 )
		{
			//program ICER2 register
			*NVIC_ICER3 |= ( 1 << (IRQNumber % 64) );
		}
	}

}





void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//1. first lets find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section  = IRQNumber %4 ;

	uint8_t shift_amount = ( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED) ;

	*(  NVIC_PR_BASE_ADDR + iprx ) |=  ( IRQPriority << shift_amount );

}



static  void  I2C_MasterHandleTXEInterrupt (I2C_Handle_t * pI2CHandle)
{

	if(pI2CHandle-> TxLen > 0 )
	{
		// 1. verileri DR'ye yükleyin
		pI2CHandle-> pI2Cx -> DR = * (pI2CHandle-> pTxBuffer );

		// 2. TxLen'i azaltın
		pI2CHandle->TxLen--;

		// 3. Tampon adresini artırın
		pI2CHandle-> pTxBuffer++;

	}

}



static void  I2C_MasterHandleRXNEInterrupt (I2C_Handle_t * pI2CHandle)
{
	// Veri alımını yapmalıyız
	if(pI2CHandle-> RxSize == 1 )
	{
		* pI2CHandle-> pRxBuffer = pI2CHandle-> pI2Cx -> DR ;
		pI2CHandle-> RxLen--;

	}


	if(pI2CHandle-> RxSize > 1 )
	{
		if(pI2CHandle-> RxLen == 2 )
		{
			// ack bitini temizle
			I2C_ManageAcking(pI2CHandle-> pI2Cx , DISABLE);
		}

			// DR'yi oku
			* pI2CHandle-> pRxBuffer = pI2CHandle-> pI2Cx -> DR ;
			pI2CHandle-> pRxBuffer ++;
			pI2CHandle-> RxLen--;
	}

	if(pI2CHandle-> RxLen == 0 )
	{
		// I2C veri alımını kapatın ve uygulamayı bilgilendirin

		// 1. durdurma koşulunu oluşturun
		if(pI2CHandle-> Sr == I2C_DISABLE_SR)
			I2C_GenerateStopCondition (pI2CHandle-> pI2Cx );

		// 2. I2C rx'i kapatın
		I2C_CloseReceiveData(pI2CHandle);

		// 3. Uygulamayı bilgilendirin
		I2C_ApplicationEventCallback (pI2CHandle, I2C_EV_RX_CMPLT);
	}
}


void  I2C_CloseReceiveData (I2C_Handle_t * pI2CHandle)
{
	// ITBUFEN Kontrol Bitini devre dışı bırakmak için kodu uygulayın
	pI2CHandle-> pI2Cx -> CR2 &= ~ ( 1 << I2C_CR2_ITBUFEN);

	// ITEVFEN Kontrol Bitini devre dışı bırakmak için kodu uygulayın
	pI2CHandle-> pI2Cx -> CR2 &= ~ ( 1 << I2C_CR2_ITEVTEN);

	pI2CHandle-> TxRxState = I2C_READY;
	pI2CHandle-> pRxBuffer = NULL ;
	pI2CHandle-> RxLen = 0;
	pI2CHandle-> RxSize= 0;

	if(pI2CHandle-> I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking (pI2CHandle-> pI2Cx ,ENABLE);
	}

}

void  I2C_CloseSendData(I2C_Handle_t * pI2CHandle)
{
	// ITBUFEN Kontrol Bitini devre dışı bırakmak için kodu uygulayın
	pI2CHandle-> pI2Cx ->CR2 &= ~ ( 1 << I2C_CR2_ITBUFEN);

	// ITEVFEN Kontrol Bitini devre dışı bırakmak için kodu uygulayın
	pI2CHandle-> pI2Cx ->CR2 &= ~ ( 1 << I2C_CR2_ITEVTEN);


	pI2CHandle-> TxRxState = I2C_READY;
	pI2CHandle-> pTxBuffer = NULL;
	pI2CHandle-> TxLen = 0 ;
}




void I2C_SlaveSendData(I2C_RegDef_t *pI2C,uint8_t data)
{

	pI2C->DR=data;
}



uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C)
{

	return (uint8_t)pI2C->DR;
}



void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	// Bir cihazın hem ana hem de bağımlı modu için kesinti işleme
	uint32_t temp1,temp2,temp3;

	temp1=pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
	temp2=pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);

	temp3=pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_SB);
	// 1. SB olayı tarafından oluşturulan kesinti için tutamaç
	// 	Not: SB bayrağı yalnızca Ana modda kullanılabilir
	if(temp1 && temp3)
	{
	// Kesinti, SB olayı nedeniyle üretildi
	// Bu blok bağımlı modda yürütülemeyecek çünkü bağımlı SB her zaman sıfırdır(yani köle için SB  bayragı 0 dır.)
	// Bu blokta adres aşamasını çalıştıralım
		if(pI2CHandle->TxRxState==I2C_BUSY_IN_TX)
		{
			I2C_ExcuteAddressPhaseWrite(pI2CHandle->pI2Cx,pI2CHandle->DevAddr);
		}else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_ExcuteAddressPhaseRead(pI2CHandle->pI2Cx,pI2CHandle->DevAddr);
		}
	}


	temp3=pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);
	// 2. ADDR olayı tarafından oluşturulan kesme için Tanıtıcı
	// Not: Ana modda: Adres gönderilir
	//  Bağımlı modda: Adres kendi adresiyle eşleştiğinde
	if(temp1 && temp3)
	{

	// ADDR olayı nedeniyle kesinti üretildi
	}


	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);
	// 3. BTF (Bitti Bayt Transferi) olayı tarafından oluşturulan kesme için Kulp
	if (temp1 && temp3)
	{

		// BTF bayrağı ayarlandı
		if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			// TXE'nin de ayarlandığından emin olun.
			if (pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE))
			{

				// BTF, TXE = 1
				if(pI2CHandle->TxLen == 0)
				{

				// 1. STOP koşulunu oluşturun
				if(pI2CHandle->Sr == I2C_DISABLE_SR)
					I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

				// 2. tutamaç yapısının tüm üye öğelerini sıfırlayın.
				I2C_CloseSendData(pI2CHandle);

				// 3. uygulamaya aktarımın tamamlandığını bildir
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
				}

			}
		} else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
          ;
		}

	}


	temp3=pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF);
	// 4. STOPF olayı tarafından oluşturulan kesme için Tutamaç
	// Not: Algılamayı durdur bayrağı yalnızca bağımlı mod için geçerlidir. Usta için bu bayrak asla ayarlanmayacak
	// Aşağıdaki kod bloğu, ana modda STOPF ayarlanmayacağı için master tarafından yürütülmeyecektir
	if(temp1 && temp3)
	{

	// STOF bayrağı ayarlandı
	// STOPF'yi temizle (yani 1) SR1'i oku 2) CR1'e yaz)
		pI2CHandle->pI2Cx->CR1 |=0x0000;

	// Uygulamaya STOP algılandığını bildir
		I2C_ApplicationEventCallback(pI2CHandle,I2C_EV_STOP);
	}


	temp3=pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);
	// 5. TXE olayı tarafından oluşturulan kesme için Tanıtıcı
	if(temp1 && temp2 && temp3)
	{
		// Cihaz modunu kontrol edin
		if(pI2CHandle-> pI2Cx -> SR2 & ( 1 << I2C_SR2_MSL))
		{
			// TXE bayrağı ayarlandı
			// Veri aktarımını yapmalıyız
			if(pI2CHandle-> TxRxState == I2C_BUSY_IN_TX)
			{
				I2C_MasterHandleTXEInterrupt (pI2CHandle);
			}
		} else
		{
			// köle
			// bağımlı birimin gerçekten verici modunda olduğundan emin olun
		   if(pI2CHandle-> pI2Cx -> SR2 & ( 1 << I2C_SR2_TRA))
		    {
		    	I2C_ApplicationEventCallback (pI2CHandle, I2C_EV_DATA_REQ);
		    }
		}
	}




	temp3 = pI2CHandle-> pI2Cx -> SR1 & ( 1 << I2C_SR1_RXNE);
	// 6. RXNE olayı tarafından oluşturulan kesme için Tanıtıcı
	if(temp1 && temp2 && temp3)
	{
		// cihaz modunu kontrol edin.
		if(pI2CHandle-> pI2Cx -> SR2 & ( 1 << I2C_SR2_MSL))
		{
			// Cihaz ustadır

			// RXNE bayrağı ayarlandı
			if(pI2CHandle-> TxRxState == I2C_BUSY_IN_RX)
			{
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);

			}

		} else
		{
			// köle
			// Slave'in gerçekten alıcı modunda olduğundan emin olun
			if(! (pI2CHandle-> pI2Cx -> SR2 & ( 1 << I2C_SR2_TRA)))
			{
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
			}
		}
	}
}






/* ************************************************ ********************
 * @fn - I2C_ER_IRQHandling
 *
 * @brief -
 *
 * @param [içinde] -
 * @param [içinde] -
 * @param [içinde] -
 *
 * @dönüş -
 *
 * @Note - Kodu tamamlayın, ayrıca bu makroları sürücüde tanımlayın
						başlık dosyası
						#define I2C_ERROR_BERR 3
						#define I2C_ERROR_ARLO 4
						#define I2C_ERROR_AF 5
						#define I2C_ERROR_OVR 6
						#define I2C_ERROR_TIMEOUT 7
 */

void  I2C_ER_IRQHandling (I2C_Handle_t * pI2CHandle)
{

	uint32_t temp1, temp2;

    // CR2'deki ITERREN kontrol bitinin durumunu öğrenin
	temp2 = (pI2CHandle-> pI2Cx -> CR2 ) & ( 1 << I2C_CR2_ITERREN);


/* ********************** Otobüs hatasını kontrol et ********************** ************* */
	temp1 = (pI2CHandle-> pI2Cx -> SR1 ) & ( 1 << I2C_SR1_BERR);
	if(temp1 && temp2)
	{
		// Bu Bus hatasıdır

		// Veri yolu hata bayrağını temizlemek için kodu uygulayın
		pI2CHandle-> pI2Cx -> SR1 &= ~( 1 << I2C_SR1_BERR);

		// Uygulamayı hatayla ilgili olarak bilgilendirmek için kodu uygulayın
	   I2C_ApplicationEventCallback (pI2CHandle, I2C_ERROR_BERR);
	}

/* ********************** Tahkim kayıp hatasını kontrol edin ********************* ************** */
	temp1 = (pI2CHandle-> pI2Cx -> SR1 ) & ( 1 << I2C_SR1_ARLO);
	if(temp1 && temp2)
	{
		// Bu tahkim kayıp hatasıdır

		// Tahkim kayıp hata bayrağını temizlemek için kodu uygulayın
		pI2CHandle-> pI2Cx -> SR1 &= ~( 1 << I2C_SR1_ARLO);

		// Uygulamayı hatayla ilgili olarak bilgilendirmek için kodu uygulayın
		I2C_ApplicationEventCallback (pI2CHandle, I2C_ERROR_ARLO);

	}

/* ********************** ACK hatası olup olmadığını kontrol edin ********************* ************** */

	temp1 = (pI2CHandle-> pI2Cx -> SR1 ) & ( 1 << I2C_SR1_AF);
	if(temp1 && temp2)
	{
		// Bu ACK hatası hatasıdır

	    // ACK hatası hata bayrağını temizlemek için kodu uygulayın
		pI2CHandle-> pI2Cx -> SR1 &= ~ ( 1 << I2C_SR1_AF);

		// Uygulamayı hatayla ilgili olarak bilgilendirmek için kodu uygulayın
		I2C_ApplicationEventCallback (pI2CHandle, I2C_ERROR_AF);
	}

/* ********************** Taşma / geri alma hatasını kontrol et ********************* *************** */
	temp1 = (pI2CHandle-> pI2Cx -> SR1 ) & ( 1 << I2C_SR1_OVR);
	if(temp1 && temp2)
	{
		// Bu Overrun / underrun

	    // Overrun / underrun hata bayrağını temizlemek için kodu uygulayın
		pI2CHandle-> pI2Cx -> SR1 &= ~( 1 << I2C_SR1_OVR);

		// Uygulamayı hatayla ilgili olarak bilgilendirmek için kodu uygulayın
		I2C_ApplicationEventCallback (pI2CHandle, I2C_ERROR_OVR);
	}

/* ********************** Zaman aşımı hatasını kontrol edin ********************* ************** */
	temp1 = (pI2CHandle-> pI2Cx -> SR1 ) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1 && temp2)
	{
		// Bu zaman aşımı hatasıdır

	    // Zaman aşımı hata bayrağını temizlemek için kodu uygulayın
		pI2CHandle-> pI2Cx -> SR1  &= ~ ( 1 << I2C_SR1_TIMEOUT);

		// Uygulamayı hatayla ilgili olarak bilgilendirmek için kodu uygulayın
		I2C_ApplicationEventCallback (pI2CHandle, I2C_ERROR_TIMEOUT);
	}

}




void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx,uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		pI2Cx->CR1 |= ( 1 << I2C_CR1_PE);
	}else
	{
		pI2Cx->CR1 &= ~( 1 << I2C_CR1_PE);
	}

}
