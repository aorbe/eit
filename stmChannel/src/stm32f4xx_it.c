/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4_discovery.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t comm_data[SIZE_RCV_BUFFER][FRAME_SIZE];
extern volatile int16_t data[CAPTURE_LEN];	// Store each AD reading
extern volatile uint8_t values[256][10];

// DMA1
uint8_t buffer = 0;							// Number of the actual RX buffer

// DMA2
uint16_t count = 0;
struct FP
{
	union {
		float f;
		uint32_t l;
		uint16_t i[2];
		uint8_t b[4];
	};
} sSin, sCos;
int32_t *sDst;
int16_t *sSrc;
int32_t total[20];
uint16_t node;
uint8_t samples_idx = 0;

const float Sin[] = {
		     0.000000000000000,
			 0.309016994374947,
			 0.587785252292473,
			 0.809016994374947,
			 0.951056516295154,
			 1.000000000000000,
			 0.951056516295154,
			 0.809016994374947,
			 0.587785252292473,
			 0.309016994374948,
			 0.000000000000000,
			-0.309016994374947,
			-0.587785252292473,
			-0.809016994374947,
			-0.951056516295154,
			-1.000000000000000,
			-0.951056516295154,
			-0.809016994374948,
			-0.587785252292473,
			-0.309016994374948
};

const float Cos[] = {
		 1.000000000000000,
		 0.951056516295154,
		 0.809016994374947,
		 0.587785252292473,
		 0.309016994374948,
		 0.000000000000000,
		-0.309016994374947,
		-0.587785252292473,
		-0.809016994374947,
		-0.951056516295154,
		-1.000000000000000,
		-0.951056516295154,
		-0.809016994374948,
		-0.587785252292473,
		-0.309016994374948,
		 0.000000000000000,
		 0.309016994374947,
		 0.587785252292473,
		 0.809016994374947,
		 0.951056516295154
};

/**
 * @brief  Monitor cycle signal
 * @param  None
 * @retval None
 */
//__attribute__( (section(".data#") ) )
void EXTI9_5_IRQHandler(void) {
	EXTI->PR = EXTI_Line6;
    if (TIM8->CNT)
    {
    	//GPIOD->ODR  ^= GPIO_Pin_13;
		//TIM8->CNT = 0;
    }
}

/**
 * @brief  This function handles TIM8 Trigger interrupt request.
 * @param  None
 * @retval None
 */
//__attribute__( (section(".data#") ) )
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	TIM8->SR 	 = (uint16_t) ~(TIM_IT_Trigger);		// Clear Timer Trigger interrupt

	GPIOD->BSRRL = GPIO_Pin_13;
	DMA2_Stream1->NDTR   = CAPTURE_LEN;
	DMA2_Stream1->M0AR 	 = (uint32_t)&data[0];
	DMA2_Stream1->CR	|= (uint32_t) DMA_SxCR_EN;		// Enable DAV capture
	//TIM1->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief  Handles timeout on communication bus (probably synchronism error) reinitializing DMA transfer
 * @param  None
 * @retval None
 */
void TIM3_IRQHandler(void)
{
	uint16_t tmp;
	TIM3->SR			 = (uint16_t) ~(TIM_IT_Update);		// Clear Timer Trigger interrupt

	while(USART3->SR & USART_FLAG_RXNE)
		tmp = USART3->DR;
	USART3->CR3 |= (USART_DMAReq_Rx | USART_DMAReq_Tx);

	DMA1_Stream1->M0AR 	 = (uint32_t) comm_data[buffer];
	DMA1_Stream1->CR	|= (uint32_t) DMA_SxCR_EN;			// Recv

}


/**
 * @brief  After AD capture, data is demodulated
 * @param  DMA2 data
 * @retval values
 */
void DMA2_Stream1_IRQHandler(void)
{
	DMA2->LIFCR = DMA_LIFCR_CTCIF1;

   	GPIOD->BSRRH = GPIO_Pin_13;
   	GPIOD->BSRRL = GPIO_Pin_14;

	if (comm_data[0][4])	// MODE
	{
		// Calibration Mode : Data without any treatment
		for(count=0; count<CAPTURE_LEN; count++)
		{
			((int16_t*)values)[count] = data[count];
		}
	}
	else
	{
		// Normal Mode : Amplitude/Phase/Quality for each data set (CAPTURE_LEN)
		// TIM8 identifies each capture (communication is assynchronous)
		samples_idx = TIM8->CNT;

		// Amplitude = (S^2 + C^2)^(.5)
		// Phase = atan(C/S)
		// S = SUM (sin(wt) .* capture)
		// C = SUM (cos(wt) .* capture)
		sSin.f = 0;
		sCos.f = 0;

		// As frequency are multiple and floating point multiplication is very slow
		// First all sum is done and after only 20 multiplications
		count = 20;
		sDst = total;
		sSrc = data;
		while(count > 0u)
		{
			count--;
			*(sDst++) = *(sSrc++);
		}

		// I dont known whether if or % are faster
		node = 0;
		for(count=20; count<CAPTURE_LEN; count++)
		{
			total[node] = total[node] +  data[count];
			node++;
			if (node >= 20)
				node = 0;
		}
		for(count=0; count<20; count++)
		{
			sSin.f = sSin.f + (Sin[count] * total[count]);
			sCos.f = sCos.f + (Cos[count] * total[count]);
		}

		// TODO: Test diferent implementations
		for(count=0; count<4; count++) {
			values[samples_idx][count]   = sSin.b[count];
			values[samples_idx][count+4] = sCos.b[count];
		}
		values[samples_idx][8] = samples_idx;
		values[samples_idx][9] = 0x80;

	}
	GPIOD->BSRRH = GPIO_Pin_14;
}

volatile uint32_t *from, *to;
void DMA1_Stream1_IRQHandler(void)
{
	DMA1->LIFCR = DMA_LIFCR_CTCIF1;

	// Verify frame
	if (	(comm_data[buffer][0] != 0xFF) ||
			(comm_data[buffer][8] != 0xEE) ||
			(comm_data[buffer][9] != 0x55) ||
			(comm_data[buffer][3] > SLAVE_QTY)
			)
	{
		TIM3->CR1			|= TIM_CR1_CEN;								// Start timer
		USART3->CR3 &= (uint16_t)~(USART_DMAReq_Rx | USART_DMAReq_Tx);
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
	DMA1->HIFCR = (uint32_t)DMA_HIFCR_CTCIF4;

	DMA1_Stream4->M0AR		= (uint32_t) comm_data[buffer];
	DMA1_Stream4->CR		|= (uint32_t)DMA_SxCR_EN;		// Send

	buffer = 1 - buffer;
	DMA1_Stream1->M0AR = (uint32_t) comm_data[buffer];
	DMA1_Stream1->CR |= (uint32_t)DMA_SxCR_EN;		// Recv
}

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
