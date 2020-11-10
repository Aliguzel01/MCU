/*
 * stm32f407xx_spi_driver.h
 *
 *  Created on: 21 Ağu 2020
 *      Author: HP
 */

#ifndef INC_STM32F407XX_SPI_DRIVER_H_
#define INC_STM32F407XX_SPI_DRIVER_H_
#include "stm32f407xx.h"

/*
 * SPIx çevre birimi için yapılandırma yapısı
 */
typedef struct
{
	uint8_t 	SPI_DeviceMode;          /*  @SPI_DeviceMode   */
	uint8_t 	SPI_BusConfig;
	uint8_t 	SPI_DFF;
	uint8_t 	SPI_CPHA;
	uint8_t 	SPI_CPOL;
	uint8_t 	SPI_SSM;
	uint8_t 	SPI_SclkSpeed;

}SPI_Config_t;



/*
 * SPIx çevre birimi için tutamak yapısı
 */

 typedef struct
 {
    SPI_RegDef_t    *pSPIx;         /*Bu, SPIx (x: 0,1,2) çevre biriminin temel adresini tutar*/
    SPI_Config_t    SPIConfig;
	uint8_t  		*pTxBuffer; /* ! <Uygulamayı saklamak için. Tx arabellek adresi> */
	uint8_t  		*pRxBuffer;	/* ! <Uygulamayı saklamak için. Rx arabellek adresi> */
	uint32_t  		TxLen;		/* ! <Tx len saklamak için> */
	uint32_t  		RxLen;		/* ! <Tx len saklamak için> */
	uint8_t  		TxState;	/* ! <Tx durumunu saklamak için> */
	uint8_t  		RxState;	/* ! <Rx durumunu saklamak için> */
 }SPI_Handle_t;


 /*
  * SPI application states
  */
 #define SPI_READY 					    0
 #define SPI_BUSY_IN_RX 				1
 #define SPI_BUSY_IN_TX 				2

 /*
  * olası SPI uygulama olayları
  */
#define  SPI_EVENT_TX_CMPLT       1
#define  SPI_EVENT_RX_CMPLT       2
#define  SPI_EVENT_OVR_ERR        3
#define  SPI_EVENT_CRC_ERR        4




 /*
  * @SPI_DeviceMode
  */
 #define SPI_DEVICE_MODE_MASTER    1
 #define SPI_DEVICE_MODE_SLAVE     0


 /*
  * @SPI_BusConfig
  */
 #define SPI_BUS_CONFIG_FD                1                      /*FD=full duplex  çift yönlü iletişim  */
 #define SPI_BUS_CONFIG_HD                2                       /*HD=half dublex yarı yönlü iletişim */
 #define SPI_BUS_CONFIG_SIMPLEX_RXONLY    3                        /*tek yönlü sadece alıcı iletişim */

 /*
  * @SPI_SclkSpeed
  */
 #define SPI_SCLK_SPEED_DIV2             	0 /*cevrsel saati bölerek seri saat üretimini bulma */
 #define SPI_SCLK_SPEED_DIV4             	1
 #define SPI_SCLK_SPEED_DIV8             	2
 #define SPI_SCLK_SPEED_DIV16             	3
 #define SPI_SCLK_SPEED_DIV32             	4
 #define SPI_SCLK_SPEED_DIV64             	5
 #define SPI_SCLK_SPEED_DIV128             	6
 #define SPI_SCLK_SPEED_DIV256             	7

 /*
  * @SPI_DFF
  */
 #define SPI_DFF_8BITS 	0                        /*veri format biçimi kaç bit oldugu*/
 #define SPI_DFF_16BITS  1

 /*
  * @CPOL
  */
 #define SPI_CPOL_HIGH 1
 #define SPI_CPOL_LOW 0

 /*
  * @CPHA
  */
 #define SPI_CPHA_HIGH 1
 #define SPI_CPHA_LOW 0

 /*
  * @SPI_SSM
  */
 #define SPI_SSM_EN     1
 #define SPI_SSM_DI     0


/*
 * SPI related status flags definitions
 */
#define SPI_TXE_FLAG           (1 << SPI_SR_TXE)
#define SPI_RXNE_FLAG          (1 << SPI_SR_RXNE)
#define SPI_BUSY_FLAG          (1 << SPI_SR_BSY)




 /* ************************************************ *****************************************
  * Bu sürücü tarafından desteklenen API'ler
  * API'ler hakkında daha fazla bilgi için işlev tanımlarını kontrol edin
 ************************************************** *************************************** */


 /*
  * Periferik Saat kurulumu
  */

 void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi);

 /*
  * Init ve De-init
  */
 void SPI_Init(SPI_Handle_t *pSPIHandle);
 void SPI_DeInit(SPI_RegDef_t *pSPIx);



 /*
  * DATA SEND AND RECEİVE
  */
 void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *pTxBuffer,uint32_t len);
 void SPI_ReceiveData(SPI_RegDef_t *pSPIx,uint8_t *pRxBuffer,uint32_t len);



 uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pTxBuffer,uint32_t len);
 uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pRxBuffer,uint32_t len);

 /*
  * IRQ configuration and ISR handling
  */

 void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
 void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
 void SPI_IRQHandling(SPI_Handle_t  *pHandle);

/*
 * other peripheral control APIs
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
void  SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
void  SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName);
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void SPI_CloseReception(SPI_Handle_t *pSPIHandle);


/*
 * Application Callback
 */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv);     //AppEv=Application event



#endif /* INC_STM32F407XX_SPI_DRIVER_H_ */
