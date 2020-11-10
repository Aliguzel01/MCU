
/*
 * stm32f407xx_usart_driver.c
 *
 *  Created on: 20 Eyl 2020
 *      Author: HP
 */
#include "stm32f407xx_usart_driver.h"




/*********************************************************************
 * @fn      		  - USART_SetBaudRate
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
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate)
{

	// APB saatini tutan değişken
	uint32_t PCLKx;

	uint32_t usartdiv;

	// Mantissa ve Kesir değerlerini tutacak değişkenler
	uint32_t M_part, F_part;

  uint32_t tempreg = 0 ;

  // APB veri yolu saatinin değerini PCLKx değişkenine alın
  if(pUSARTx == USART1 || pUSARTx == USART6)
  {
	   // USART1 ve USART6, APB2 veri yolunda asılı
	   PCLKx = RCC_GetPCLK2Value ();
  } else
  {
	   PCLKx = RCC_GetPCLK1Value ();
  }

  // OVER8 yapılandırma bitini kontrol edin
  if(pUSARTx-> CR1 & ( 1 << USART_CR1_OVER8))
  {
	   // OVER8 = 1, 8 ile fazla örnekleme
	   usartdiv = (( 25 * PCLKx) / ( 2 * BaudRate));
  } else
  {
	   // 16 ile fazla örnekleme
	   usartdiv = (( 25 * PCLKx) / ( 4 * BaudRate));
  }

  // Mantissa bölümünü hesaplayın
  M_part = usartdiv / 100 ;

  // Mantissa parçasını uygun bit konumuna yerleştirin. USART_BRR'ye bakın
  tempreg |= M_part << 4 ;

  // Kesir kısmını çıkar
  F_part = (usartdiv - (M_part * 100 ));

  // Son kesiri hesapla
  if (pUSARTx-> CR1 & ( 1 << USART_CR1_OVER8))
   {
	  // OVER8 = 1, 8 ile fazla örnekleme /*OVER8 = 1 olduğunda,DIV_Fraction3 biti dikkate alınmaz ve temiz tutulmalıdır. bu yüzden 0x07 ile maskeleme yaptık.referans kılavuzu USART_BRR kaydında yazıyor*/
	  F_part = (((F_part * 8 ) + 50 ) / 100 ) & (( uint8_t ) 0x07 );

   } else
   {
	   // 16 ile fazla örnekleme
	   F_part = (((F_part * 16 ) + 50 ) / 100 ) & (( uint8_t ) 0x0F );

   }

  // Kesirli parçayı uygun bit konumuna yerleştirin. USART_BRR'ye bakın
  tempreg |= F_part;

  // tempreg değerini BRR yazmacına kopyala
  pUSARTx-> BRR = tempreg;

}




/*********************************************************************
 * @fn      		  - USART_EnableOrDisable
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
void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pUSARTx->CR1 |= (1 << USART_CR1_UE);
	}else
	{
		pUSARTx->CR1 &= ~(1 << USART_CR1_UE);
	}

}




/*********************************************************************
 * @fn      		  - USART_Init
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
void USART_Init(USART_Handle_t *pUSARTHandle)
{
	// Geçici değişken
	uint32_t tempreg = 0;

	/******************************** Configuration of CR1******************************************/

	// Verilen USART çevre birimi için Saati etkinleştirmek için kodu uygulayın
	USART_PeriClockControl(pUSARTHandle->pUSARTx, ENABLE);

	// USART_Mode yapılandırma öğesine göre USART Tx ve Rx motorlarını etkinleştirin
	if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX)
	{
		// Alıcı bit alanını etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR1_RE);

	}else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX)
	{
		// Verici bit alanını etkinleştirmek için kodu uygulayın
		tempreg |= (1 << USART_CR1_TE);

	}else if(pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX)
	{
		// Hem Verici hem de Alıcı bit alanlarını etkinleştirmek için kodu uygulayın
		tempreg |= ( (1 << USART_CR1_RE) | (1 << USART_CR1_TE));
	}

	// Kelime uzunluğu yapılandırma öğesini yapılandırmak için kodu uygulayın
	tempreg |=pUSARTHandle->USART_Config.USART_WordLength << USART_CR1_M;


	// Eşlik kontrol bit alanlarının konfigürasyonu
	if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_EVEN)
	{
		// Eşlik kontrolünü etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR1_PCE);

		// EVEN eşliğini etkinleştirmek için kodu uygulayın
		// Gerekli değil çünkü varsayılan olarak, eşlik kontrolünü etkinleştirdiğinizde EVEN eşlik seçilecek

	}else if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_ODD)
	{
		// Eşlik kontrolünü etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR1_PCE);
	    // ODD eşliğini etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR1_PS);
	}

	// CR1 kaydını programlayın
	pUSARTHandle->pUSARTx->CR1=tempreg;

	/* ******************************* CR2 yapılandırması ************** *************************** */

	tempreg=0;

	// USART çerçeve iletimi sırasında eklenen durdurma bitlerinin sayısını yapılandırmak için kodu uygulayın
	tempreg |=pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP;

	// CR2 kaydını programlayın
	pUSARTHandle-> pUSARTx -> CR2 = tempreg;


	/* ******************************* CR3 Yapılandırması ************** *************************** */

	tempreg=0;

	// USART donanım akış kontrolünün konfigürasyonu
	if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS)
	{
		// CTS akış kontrolünü etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR3_CTSE);

	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS)
	{
		// RTS akış kontrolünü etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR3_RTSE);
	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
	{
		// Hem CTS hem de RTS Akış kontrolünü etkinleştirmek için kodu uygulayın
		tempreg |=(1 << USART_CR3_CTSE);
		tempreg |=(1 << USART_CR3_RTSE);

	}

	pUSARTHandle->pUSARTx->CR3=tempreg;

	/* ******************************* BRR Yapılandırması (Baudrate kaydı) ********** ******************************** */

		// Baud hızını yapılandırmak için kodu uygulayın
	USART_SetBaudRate(pUSARTHandle-> pUSARTx , pUSARTHandle-> USART_Config.USART_Baud );

}




void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len)
{

	uint16_t *pdata;

   // "Len" bayt sayısı aktarılıncaya kadar döngü yapın
	for( uint32_t i = 0 ; i <Len; i ++)
	{
		// SR'de TXE bayrağı ayarlanana kadar beklemek için kodu uygulayın
		while (! USART_GetFlagStatus (pUSARTHandle-> pUSARTx , USART_FLAG_TXE));

		// Bir çerçevede 9BIT veya 8BIT için USART_WordLength öğesini kontrol edin
		if(pUSARTHandle-> USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
		{
			// eğer 9BIT, DR'yi ilk 9 bit dışındaki bitleri maskeleyen 2 bayt ile yüklerse
			pdata = ( uint16_t*) pTxBuffer;
			pUSARTHandle-> pUSARTx ->DR= (*pdata & ( uint16_t ) 0x01FF );

			// USART_ParityControl için kontrol edin
			if(pUSARTHandle-> USART_Config . USART_ParityControl == USART_PARITY_DISABLE)
			{
				// Bu transferde parite kullanılmadığından 9 bitlik kullanıcı verisi gönderilecektir
				// pTxBuffer'ı iki kez artırmak için kodu uygulayın
				pTxBuffer++;
				pTxBuffer++;
			}
			else
			{
				// Bu transferde eşlik biti kullanılır. böylece 8 bit kullanıcı verisi gönderilecek
				// 9. bit, donanım tarafından eşlik biti ile değiştirilecektir
				pTxBuffer ++;
			}
		}
		else
		{
			// Bu 8 bit veri aktarımıdır
			pUSARTHandle-> pUSARTx ->DR = (*pTxBuffer & ( uint8_t ) 0xFF );

			// Tampon adresini artırmak için kodu uygulayın
			pTxBuffer++;
		}
	}

	// SR'de TC bayrağı ayarlanana kadar beklemek için kodu uygulayın
	while (! USART_GetFlagStatus (pUSARTHandle-> pUSARTx , USART_FLAG_TC));

}




void  USART_ReceiveData(USART_Handle_t *pUSARTHandle,uint8_t *pRxBuffer, uint32_t Len)
{
	   // "Len" bayt sayısı aktarılıncaya kadar döngü yapın
		for( uint32_t i = 0 ; i <Len; i ++)
		{
			// SR'de RXNE bayrağı ayarlanana kadar beklemek için kodu uygulayın
			while (! USART_GetFlagStatus (pUSARTHandle-> pUSARTx , USART_FLAG_RXNE));

			// Bir çerçevede 9 bit veri mi yoksa 8 bit mi alacağımıza karar vermek için USART_WordLength değerini kontrol edin
			if (pUSARTHandle-> USART_Config . USART_WordLength == USART_WORDLEN_9BITS)
			{
				// Bir çerçevede 9bit veri alacağız

				// Şimdi, USART_ParityControl kontrolünü kullanıp kullanmadığımızı kontrol edin
				if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
				{
					// Eşlik kullanılmaz, bu nedenle tüm 9 bit kullanıcı verilerinden olacaktır

					// sadece ilk 9 biti oku böylece DR'yi 0x01FF ile maskeleyin
					*(( uint16_t*)pRxBuffer) = (pUSARTHandle-> pUSARTx -> DR & ( uint16_t ) 0x01FF );

					// Şimdi pRxBuffer'ı iki kez artırın
					pRxBuffer++;
					pRxBuffer++;
				}
				else
				{
					// Eşlik kullanılır, bu nedenle 8 bit kullanıcı verisinden ve 1 bit eşittir
					 *pRxBuffer = (pUSARTHandle-> pUSARTx -> DR   & ( uint8_t ) 0xFF );
					 pRxBuffer++;
				}
			}
			else
			{
				// Bir çerçevede 8 bitlik veri alacağız

				// Şimdi, USART_ParityControl kontrolünü kullanıp kullanmadığımızı kontrol edin
				if(pUSARTHandle-> USART_Config . USART_ParityControl == USART_PARITY_DISABLE)
				{
					// Eşlik kullanılmaz, bu nedenle tüm 8 bitler kullanıcı verilerinden olacaktır

					// DR'den 8 bit oku
					 *pRxBuffer = ( uint8_t ) (pUSARTHandle-> pUSARTx -> DR   & ( uint8_t ) 0xFF );
				}

				else
				{
					// Parite kullanılır, yani 7 bit kullanıcı verisinden ve 1 bit parite olacaktır

					// sadece 7 bit okur, dolayısıyla DR'yi 0X7F ile maskeleyin
					 *pRxBuffer = ( uint8_t ) (pUSARTHandle-> pUSARTx -> DR   & ( uint8_t ) 0x7F );

				}

				// Şimdi, pRxBuffer'ı artırın
				pRxBuffer ++;
			}
		}

}

/*********************************************************************
 * @fn      		  - USART_SendDataIT
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
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t txstate = pUSARTHandle-> TxBusyState ;

	if (txstate != USART_BUSY_IN_TX)
	{
		pUSARTHandle-> TxLen = Len;
		pUSARTHandle-> pTxBuffer = pTxBuffer;
		pUSARTHandle-> TxBusyState = USART_BUSY_IN_TX;

		// TXE için kesmeyi etkinleştirmek üzere kodu uygulayın
		pUSARTHandle-> pUSARTx -> CR1 |= ( 1 << USART_CR1_TXEIE);


		// TC için kesmeyi etkinleştirmek için kodu uygulayın
		pUSARTHandle-> pUSARTx -> CR1 |= ( 1 << USART_CR1_TCIE);


	}

	return txstate;

}

/*********************************************************************
 * @fn      		  - USART_ReceiveDataIT
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
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle,uint8_t *pRxBuffer, uint32_t Len)
{

	uint8_t rxstate = pUSARTHandle-> RxBusyState ;

	if(rxstate != USART_BUSY_IN_RX)
	{
		pUSARTHandle-> RxLen = Len;
		pUSARTHandle-> pRxBuffer = pRxBuffer;
		pUSARTHandle-> RxBusyState = USART_BUSY_IN_RX;

		(void) pUSARTHandle-> pUSARTx -> DR ;

		// RXNE için kesmeyi etkinleştirmek için kodu uygulayın
		pUSARTHandle-> pUSARTx -> CR1 |= ( 1 << USART_CR1_RXNEIE);

	}

	return rxstate;

}








void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(pUSARTx == USART1)
		{
			USART1_PCLK_EN();
		}else if (pUSARTx == USART2)
		{
			USART2_PCLK_EN();
		}else if (pUSARTx == USART3)
		{
			USART3_PCLK_EN();
		}
		else if (pUSARTx == UART4)
		{
			UART4_PCLK_EN();
		}else if(pUSARTx == UART5)
		{
			UART5_PCLK_EN();
		}else if(pUSARTx == USART6)
		{
			USART6_PCLK_EN();
		}
	}
	else
	{
		if(pUSARTx == USART1)
		{
			USART1_PCLK_DI();
		}else if (pUSARTx == USART2)
		{
			USART2_PCLK_DI();
		}else if (pUSARTx == USART3)
		{
			USART3_PCLK_DI();
		}
		else if (pUSARTx == UART4)
		{
			UART4_PCLK_DI();
		}else if(pUSARTx == UART5)
		{
			UART5_PCLK_DI();
		}else if(pUSARTx == USART6)
		{
			USART6_PCLK_DI();
		}
	}
}



/*********************************************************************
 * @fn      		  - USART_GetFlagStatus
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
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint8_t StatusFlagName)
{
    if(pUSARTx->SR & StatusFlagName)
    {
    	return SET;
    }

   return RESET;
}







/*********************************************************************
 * @fn      		  - USART_ClearFlag
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Applicable to only USART_CTS_FLAG , USART_LBD_FLAG
 * USART_TC_FLAG,USART_TC_FLAG flags
 *
 */

void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t StatusFlagName)
{
	pUSARTx->SR &= ~( StatusFlagName);

}


/*********************************************************************
 * @fn      		  - USART_IRQInterruptConfig
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
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
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
		else if(IRQNumber >= 6 && IRQNumber < 96 )
		{
			//program ICER2 register
			*NVIC_ICER3 |= ( 1 << (IRQNumber % 64) );
		}
	}

}


/*********************************************************************
 * @fn      		  - USART_IRQPriorityConfig
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
void USART_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority)
{
	//1. first lets find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section  = IRQNumber %4 ;

	uint8_t shift_amount = ( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED) ;

	*(  NVIC_PR_BASE_ADDR + iprx ) |=  ( IRQPriority << shift_amount );

}



/*********************************************************************
 * @fn      		  - USART_IRQHandler
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
void USART_IRQHandling(USART_Handle_t *pUSARTHandle)
{

	uint32_t temp1,temp2,temp3;

	uint16_t *pdata;

/*************************Check for TC flag ********************************************/

    //Implement the code to check the state of TC bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & ( 1 << USART_SR_TC);

	 //Implement the code to check the state of TCEIE bit
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_TCIE);

	if(temp1 && temp2 )
	{
		//this interrupt is because of TC

		//close transmission and call application callback if TxLen is zero
		if ( pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			//Check the TxLen . If it is zero then close the data transmission
			if(! pUSARTHandle->TxLen )
			{
				//Implement the code to clear the TC flag
				pUSARTHandle->pUSARTx->SR &= ~( 1 << USART_SR_TC);

				//Implement the code to clear the TCIE control bit

				//Reset the application state
				pUSARTHandle->TxBusyState = USART_READY;

				//Reset Buffer address to NULL
				pUSARTHandle->pTxBuffer = NULL;

				//Reset the length to zero
				pUSARTHandle->TxLen = 0;

				//Call the application call back with event USART_EVENT_TX_CMPLT
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_TX_CMPLT);
			}
		}
	}

/*************************Check for TXE flag ********************************************/

	//Implement the code to check the state of TXE bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & ( 1 << USART_SR_TXE);

	//Implement the code to check the state of TXEIE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_TXEIE);


	if(temp1 && temp2 )
	{
		//this interrupt is because of TXE

		if(pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			//Keep sending data until Txlen reaches to zero
			if(pUSARTHandle->TxLen > 0)
			{
				//Check the USART_WordLength item for 9BIT or 8BIT in a frame
				if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
				{
					//if 9BIT load the DR with 2bytes masking  the bits other than first 9 bits
					pdata = (uint16_t*) pUSARTHandle->pTxBuffer;
					pUSARTHandle->pUSARTx->DR = (*pdata & (uint16_t)0x01FF);

					//check for USART_ParityControl
					if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
					{
						//No parity is used in this transfer , so 9bits of user data will be sent
						//Implement the code to increment pTxBuffer twice
						pUSARTHandle->pTxBuffer++;
						pUSARTHandle->pTxBuffer++;
						pUSARTHandle->TxLen-=2;
					}
					else
					{
						//Parity bit is used in this transfer . so 8bits of user data will be sent
						//The 9th bit will be replaced by parity bit by the hardware
						pUSARTHandle->pTxBuffer++;
						pUSARTHandle->TxLen-=1;
					}
				}
				else
				{
					//This is 8bit data transfer
					pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer  & (uint8_t)0xFF);

					//Implement the code to increment the buffer address
					pUSARTHandle->pTxBuffer++;
					pUSARTHandle->TxLen-=1;
				}

			}
			if (pUSARTHandle->TxLen == 0 )
			{
				//TxLen is zero
				//Implement the code to clear the TXEIE bit (disable interrupt for TXE flag )
				pUSARTHandle->pUSARTx->CR1 &= ~( 1 << USART_CR1_TXEIE);
			}
		}
	}

/*************************Check for RXNE flag ********************************************/

	temp1 = pUSARTHandle->pUSARTx->SR & ( 1 << USART_SR_RXNE);
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_RXNEIE);


	if(temp1 && temp2 )
	{
		//this interrupt is because of rxne
		if(pUSARTHandle->RxBusyState == USART_BUSY_IN_RX)
		{
			if(pUSARTHandle->RxLen > 0)
			{
				//Check the USART_WordLength to decide whether we are going to receive 9bit of data in a frame or 8 bit
				if(pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS)
				{
					//We are going to receive 9bit data in a frame

					//Now, check are we using USART_ParityControl control or not
					if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
					{
						//No parity is used , so all 9bits will be of user data

						//read only first 9 bits so mask the DR with 0x01FF
						*((uint16_t*) pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR  & (uint16_t)0x01FF);

						//Now increment the pRxBuffer two times
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->RxLen-=2;
					}
					else
					{
						//Parity is used, so 8bits will be of user data and 1 bit is parity
						 *pUSARTHandle->pRxBuffer = (pUSARTHandle->pUSARTx->DR  & (uint8_t)0xFF);
						 pUSARTHandle->pRxBuffer++;
						 pUSARTHandle->RxLen-=1;
					}
				}
				else
				{
					//We are going to receive 8bit data in a frame

					//Now, check are we using USART_ParityControl control or not
					if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
					{
						//No parity is used , so all 8bits will be of user data

						//read 8 bits from DR
						 *pUSARTHandle->pRxBuffer = (uint8_t) (pUSARTHandle->pUSARTx->DR  & (uint8_t)0xFF);

					}

					else
					{
						//Parity is used, so , 7 bits will be of user data and 1 bit is parity

						//read only 7 bits , hence mask the DR with 0X7F
						 *pUSARTHandle->pRxBuffer = (uint8_t) (pUSARTHandle->pUSARTx->DR  & (uint8_t)0x7F);

					}

					//Now , increment the pRxBuffer
					pUSARTHandle->pRxBuffer++;
					 pUSARTHandle->RxLen-=1;
				}


			}//if of >0

			if(! pUSARTHandle->RxLen)
			{
				//disable the rxne
				pUSARTHandle->pUSARTx->CR1 &= ~( 1 << USART_CR1_RXNEIE );
				pUSARTHandle->RxBusyState = USART_READY;
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_RX_CMPLT);
			}
		}
	}


/*************************Check for CTS flag ********************************************/
//Note : CTS feature is not applicable for UART4 and UART5

	//Implement the code to check the status of CTS bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & ( 1 << USART_SR_CTS);

	//Implement the code to check the state of CTSE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_CTSE);

	//Implement the code to check the state of CTSIE bit in CR3 (This bit is not available for UART4 & UART5.)
	temp3 = pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_CTSIE);


	if(temp1  && temp2 )
	{
		//Implement the code to clear the CTS flag in SR
		pUSARTHandle->pUSARTx->SR &=  ~( 1 << USART_SR_CTS);

		//this interrupt is because of cts
		USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_CTS);
	}

/*************************Check for IDLE detection flag ********************************************/

	//Implement the code to check the status of IDLE flag bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & ( 1 << USART_SR_IDLE);

	//Implement the code to check the state of IDLEIE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_IDLEIE);


	if(temp1 && temp2)
	{
		//Implement the code to clear the IDLE flag. Refer to the RM to understand the clear sequence
		temp1 = pUSARTHandle->pUSARTx->SR &= ~( 1 << USART_SR_IDLE);

		//this interrupt is because of idle
		USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_IDLE);
	}

/*************************Check for Overrun detection flag ********************************************/

	//Implement the code to check the status of ORE flag  in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_ORE;

	//Implement the code to check the status of RXNEIE  bit in the CR1
	temp2 = pUSARTHandle->pUSARTx->CR1 & USART_CR1_RXNEIE;


	if(temp1  && temp2 )
	{
		//Need not to clear the ORE flag here, instead give an api for the application to clear the ORE flag .

		//this interrupt is because of Overrun error
		USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
	}



/*************************Check for Error Flag ********************************************/

//Noise Flag, Overrun error and Framing Error in multibuffer communication
//We dont discuss multibuffer communication in this course. please refer to the RM
//The blow code will get executed in only if multibuffer mode is used.

	temp2 =  pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_EIE) ;

	if(temp2 )
	{
		temp1 = pUSARTHandle->pUSARTx->SR;
		if(temp1 & ( 1 << USART_SR_FE))
		{
			/*
				This bit is set by hardware when a de-synchronization, excessive noise or a break character
				is detected. It is cleared by a software sequence (an read to the USART_SR register
				followed by a read to the USART_DR register).
			*/
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_FE);
		}

		if(temp1 & ( 1 << USART_SR_NE) )
		{
			/*
				This bit is set by hardware when noise is detected on a received frame. It is cleared by a
				software sequence (an read to the USART_SR register followed by a read to the
				USART_DR register).
			*/
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_NE);
		}

		if(temp1 & ( 1 << USART_SR_ORE) )
		{
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
		}
	}


}






/*********************************************************************
 * @fn      		  - USART_ApplicationEventCallback
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
__weak void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,uint8_t event)
{

}

