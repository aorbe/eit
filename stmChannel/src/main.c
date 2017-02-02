/*
 * All IO configuration is done in this routine
 * Master has:
 * 4 outputs (LED) and 1 input (pushbutton) for debug
 * 1 output (PC8) for sync
 * USART port with TX in DMA mode for networking
 * USB port for PC communication
 *
 * Slave has:
 * 4 outputs (LED PD12-15) and 1 input (pushbutton PA1) for debug
 * USART2 port  with TX in DMA mode and RX in Interrupt mode for networking (PA2-3)
 * EXTI for detection of ADC data ready DAV (PC13)
 * EXTI for detection of Sync Pulse ()
 * 16 inputs for data (PE0-15)
 * 2 input for ADC status (NOT USED)
 * 1 output for ADC control (NOT USED)
 *
 * USB PROCESSING
 * Data received from UART are directly stored in USB buffer. Higher array position
 * of USB Buffer are filled first so, USB data sent has higher ID first.
 * When 20480 values are received then HEADER are added. The buffer are exchange
 * and when one is filled others is sent. The ID put in the header represents is
 * the last received, so the higher.
 *
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4_discovery.h"
//#include "arm_math.h"
#include "init.h"

/* Private typedef -----------------------------------------------------------*/



/* Variables -----------------------------------------------------------------*/
volatile int16_t data[CAPTURE_LEN];			// Store each AD reading
volatile uint8_t comm_data[SIZE_RCV_BUFFER][FRAME_SIZE] __attribute__ ((aligned (8)));
volatile uint8_t values[256][10] __attribute__ ((aligned (8)));



/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	uint16_t x = 0;

	/*
	 * TIM1 - DAV capture
	 * TIM3 - Communication Timeout
	 * TIM8 - Synchronization Timer ???
	 * USART3 - Communication Bus
	 *
	 */

	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_DMA1   |
							RCC_AHB1Periph_DMA2   |
							RCC_AHB1Periph_GPIOA  |
							RCC_AHB1Periph_GPIOB  |
							RCC_AHB1Periph_GPIOC  |
							RCC_AHB1Periph_GPIOD  |
							RCC_AHB1Periph_GPIOE, ENABLE);

	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM3   |
							RCC_APB1Periph_USART3 , ENABLE);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM1   |
							RCC_APB2Periph_SYSCFG |
							RCC_APB2Periph_TIM8, ENABLE);


	for(x=0; x<SIZE_RCV_BUFFER * FRAME_SIZE; x++)
		comm_data[x/FRAME_SIZE][x%FRAME_SIZE] = 0;

	for(x=0; x<2560; x++)
		values[x/10][x%10] = 0xFF;

	config();

	while (1)
	{
	    /*
	    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3))
	    	GPIOD->BSRRH = GPIO_Pin_13;
	    else
	    	GPIOD->BSRRL = GPIO_Pin_13;
	    */

	}
	return 0;
}


#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif
