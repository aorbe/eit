/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4_discovery.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t comm_data[SIZE_RCV_BUFFER][FRAME_SIZE];
extern volatile int32_t data[CAPTURE_LEN];	// Store each AD reading
extern volatile int16_t samples_cont;		// Counts each ADC pulse
extern volatile uint32_t comm_ctrl;		// Each bit related to one buffer
extern volatile uint8_t values[256][10];

volatile uint8_t last_idx;				// Last writing position on RX Buffer
volatile uint8_t rcv_idx;					// Writing position on RX Buffer
uint8_t buffer = 0;							// Number of the actual RX buffer

//volatile int16_t teste[CAPTURE_LEN];
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles External line 1 interrupt request.
 * @param  None
 * @retval None
 */
__attribute__( (section(".data#") ) ) void EXTI1_IRQHandler(void) {
	EXTI->PR = EXTI_Line1;						// Clear the EXTI line 1 pending bit
	if (samples_cont)							// Reach limit
	{
		data[--samples_cont] = ((int16_t)GPIOD->IDR) << 16 | ((int16_t)GPIOE->IDR);
		//data[--samples_cont] = teste[samples_cont];
	}
	else
	{
		//EXTI->IMR &= ~EXTI_Line1;	// hardcoded for faster execution?
		EXTI->IMR 	&= 0xFFFE; 					// Disable DAV Interrupt
	}
}

__attribute__( (section(".data#") ) ) void EXTI9_5_IRQHandler(void) {
	EXTI->PR = EXTI_Line6;
    if (TIM8->CNT)
    {
    	//GPIOD->ODR  ^= GPIO_Pin_13;
		TIM8->CNT = 0;
    }

}

#ifndef DMA_CAPTURE

/**
 * @brief  This function handles TIM8 Trigger interrupt request.
 * @param  None
 * @retval None
 */
__attribute__( (section(".data#") ) ) void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	TIM8->SR 	 = (uint16_t) ~(TIM_IT_Trigger);		// Clear Timer Trigger interrupt
	GPIOD->ODR  ^= GPIO_Pin_14;
	EXTI->IMR	|= EXTI_Line1;							// Enable DAV interrupt
}

#endif
/**
 * @brief  This function handles TIM3 Update interrupt request. TIMEOUT
 * @param  None
 * @retval None
 */
void TIM3_IRQHandler(void)
{
	uint16_t tmp;
	TIM3->SR			 = (uint16_t) ~(TIM_IT_Update);		// Clear Timer Trigger interrupt

	while(USART1->SR & USART_FLAG_RXNE)
		tmp = USART1->DR;
	USART1->CR3 |= (USART_DMAReq_Rx | USART_DMAReq_Tx);

	DMA2_Stream5->M0AR 	 = (uint32_t) comm_data[buffer];
	DMA2_Stream5->CR	|= (uint32_t) DMA_SxCR_EN;			// Recv
}

#ifdef DMA_RECEPTION

volatile uint32_t *from, *to;

//__attribute__( (section(".data#") ) )
void DMA2_Stream5_IRQHandler(void)
{
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;

	// Verify frame
	if ((comm_data[buffer][0] != 0xFF) ||
			(comm_data[buffer][8] != 0xEE) ||
			(comm_data[buffer][9] != 0x55)
			|| (comm_data[buffer][3] > SLAVE_QTY)
			)
	{
		TIM3->CR1			|= TIM_CR1_CEN;								// Start timer
		USART1->CR3 &= (uint16_t)~(USART_DMAReq_Rx | USART_DMAReq_Tx);
		return;
	}


	to = (uint32_t*) (&comm_data[buffer][comm_data[buffer][3]*10]);
	from   = (uint32_t*) (&values[comm_data[buffer][1]]);
	*(to++) = *from;
	*(from++) = 0xFFFFFFFF;
	*(to++) = *from;
	*(from++) = 0xFFFFFFFF;
	*((uint16_t*) to) = *((uint16_t*) from);
	*((uint16_t*) from) = 0xFFFF;

	comm_data[buffer][comm_data[buffer][3]*10+8]  = TIM8->CNT;

	comm_data[buffer][3]++;

	if (comm_data[buffer][1] == 0xFF)
		GPIOD->ODR			^= GPIO_Pin_15;

	//snd_state = buffer + 1;
	DMA2_Stream7->NDTR		= (uint32_t) FRAME_SIZE;
	DMA2_Stream7->M0AR		= (uint32_t) comm_data[buffer];
	DMA2_Stream7->CR		|= (uint32_t)DMA_SxCR_EN;		// Send

	buffer = 1 - buffer;
	DMA2_Stream5->M0AR = (uint32_t) comm_data[buffer];
	DMA2_Stream5->CR |= (uint32_t)DMA_SxCR_EN;		// Recv
}

#else

/*
 * Data reception - Starts timer and detects any useful command
 * Timeout?
 */
void USART1_IRQHandler(void)
{
	uint32_t *from, *to;
	comm_data[buffer][rcv_idx++] = USART1->DR;
	TIM3->CNT = 0;

	if (rcv_idx >= FRAME_SIZE) {

		// Verify frame
		if ((comm_data[buffer][0] != 0xFF) ||
				(comm_data[buffer][8] != 0xEE) ||
				(comm_data[buffer][9] != 0x55))
		{
			TIM3->CR1			|= TIM_CR1_CEN;								// Start timer
			USART1->CR1			&= ~USART_CR1_RE;
			GPIOD->ODR			^= GPIO_Pin_12;
			return;
		}


		if (comm_data[buffer][3] > SLAVE_QTY)
			comm_data[buffer][3] = SLAVE_QTY;

		to = (uint32_t*) (&comm_data[buffer][comm_data[buffer][3]*10]);
		from   = (uint32_t*) (&values[comm_data[buffer][1]]);
		*(to++) = *from;
		*(from++) = 0xFFFFFFFF;
		*(to++) = *from;
		*(from++) = 0xFFFFFFFF;
		*((uint16_t*) to) = *((uint16_t*) from);
		*((uint16_t*) from) = 0xFFFF;

		comm_data[buffer][3]++;

		if (comm_data[buffer][1] == 0xFF)
			GPIOD->ODR			^= GPIO_Pin_15;
		//snd_state = buffer + 1;
		DMA2_Stream7->NDTR		= (uint32_t) FRAME_SIZE;
		DMA2_Stream7->M0AR		= (uint32_t) comm_data[buffer];
		DMA2_Stream7->CR		|= (uint32_t)DMA_SxCR_EN;		// Send

		rcv_idx = 0;
		buffer = 1 - buffer;
	}


}

#endif

// DMA Send
__attribute__( (section(".data#") ) ) void DMA2_Stream7_IRQHandler(void) {
	DMA2->HIFCR = (uint32_t)DMA_IT_TCIF7;
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
 {
 }*/


/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void) {
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void) {
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1) {
		GPIOD->BSRRL = GPIO_Pin_12;
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void) {
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1) {

	}
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void) {
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1) {
		//GPIOD->BSRRL = GPIO_Pin_15;
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void) {
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1) {
		//GPIOD->BSRRL = GPIO_Pin_15;
	}
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void) {
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void) {
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void) {
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void) {
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
