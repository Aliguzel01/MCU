/*
 * stm32f407xx_gpio_driver.h
 *
 *  Created on: Aug 16, 2020
 *      Author: HP
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include<stdint.h>
#include "stm32f407xx.h"

/*
 * GPIO pini için konfigurasyon yapısı
 */
typedef struct
{
	uint8_t GPIO_PinNumber;   /* <possible value from @GPIO_PIN_NUMBERS >*/
	uint8_t GPIO_PinMode;    /* <possible value from @GPIO_PIN_MODES >*/
	uint8_t GPIO_PinSpeed;           /* <possible value from @GPIO_PIN_SPEEDS >*/
	uint8_t GPIO_PinPuPdControl;      /* <possible value from @GPIO_PIN_PUPDCONTROL >*/
	uint8_t GPIO_PinOPType;           /* <possible value from @GPIO_PIN_OPTYPES >*/
	uint8_t GPIO_PinAltFunMode;
}GPIO_PinConfig_t;


/*
 * GPIO PİNİ İÇİN TUTAMAÇ YAPISI
 */

typedef struct
{
	//GPIO cevre biriminin temel adresini tutan işaretçi
	GPIO_RegDef_t *pGPIOx;     /*bu pinin ait oldugu GPIO portunun temel adresini tutar*/
	GPIO_PinConfig_t GPIO_PinConfig;   /*bu degişken GPIO pin yapılandırma ayarlarını tutar*/

}GPIO_Handle_t;


/*
 * @GPIO_PIN_NUMBERS
 * GPIO pin numaraları
 */

#define GPIO_PIN_NO_0                  0
#define GPIO_PIN_NO_1                  1
#define GPIO_PIN_NO_2                  2
#define GPIO_PIN_NO_3                  3
#define GPIO_PIN_NO_4                  4
#define GPIO_PIN_NO_5                  5
#define GPIO_PIN_NO_6                  6
#define GPIO_PIN_NO_7                  7
#define GPIO_PIN_NO_8                  8
#define GPIO_PIN_NO_9                  9
#define GPIO_PIN_NO_10                 10
#define GPIO_PIN_NO_11                 11
#define GPIO_PIN_NO_12                 12
#define GPIO_PIN_NO_13                 13
#define GPIO_PIN_NO_14                 14
#define GPIO_PIN_NO_15                 15






/*
 * @GPIO_PIN_MODES
 * GPIO pin possible modes
 */
#define GPIO_MODE_IN                       0
#define GPIO_MODE_OUT                      1
#define GPIO_MODE_ALTFN                    2
#define GPIO_MODE_ANALOG                   3
#define GPIO_MODE_IT_FT                    4
#define GPIO_MODE_IT_RT                    5
#define GPIO_MODE_IT_RFT                   6


/*
 * @GPIO_PIN_OPTYPES
 * GPIO pin possible output type
 */

#define GPIO_OP_TYPE_PP                    0
#define GPIO_OP_TYPE_OD                    1

/*
 * @GPIO_PIN_SPEEDS
 * GPIO pin output SPEEDS
 */
#define  GPIO_SPEED_LOW                      0
#define  GPIO_SPEED_MEDIUM                   1
#define  GPIO_SPEED_FAST                     2
#define  GPIO_SPEED_HIGH                     3

/*
 * @GPIO_PIN_PUPDCONTROL
 * GPIO pin PUPD(PULL-UP AND PULL-DOWN) yapılandırma makroları
 */
#define GPIO_NO_PUPD                       0
#define GPIO_PIN_PU                        1
#define GPIO_PIN_PD                        2





/*********************APIs*********************/
/*
 * GPIO sürücü API lerinin fonksiyon protittip tanımları
 */
/*******************************************************************************************/

/*
 * peripheral clock setup
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnorDi); /*EnorDi enable yada disable demek */


/*
 * Init and DeInit
 */

void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

/*
 * data read and write
 */


uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);


/*
 * IRQ yapılandırması ve ISR işlemesi
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void GPIO_IRQHandling(uint8_t PinNumber);



#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
