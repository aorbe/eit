#include "stm32f4xx_it.h"
#include "main.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "usb_conf.h"
#include "usbd_hid_core.h"

volatile uint8_t USB_buffer[USB_NUM_BUFFERS][USB_BUFFER_SIZE];								// Reception buffer
volatile int32_t control = USB_BUFFER_INIT;
volatile uint32_t UsbControl 	= 0;									// 32 bits mapped indication of USB_buffer utilization

extern volatile uint8_t tx_data[FRAME_SIZE];			// send data. Fix format
volatile uint8_t running 		= 0;
volatile uint8_t next_bank 		= 0;									// bank been used
volatile uint8_t override_1		= 0;									// byte overwrite by CRC
volatile uint8_t override_2		= 0;									// byte overwrite by CRC

uint16_t next_idx = 0;
// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
//uint8_t tx_state = 0;												// UART send DMA indication

// 12 more because will store crc (2) and header(10) in reception

extern USB_OTG_CORE_HANDLE     USB_OTG_dev;


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
}


/**
  * @brief  This function confirms UART sending.
  * @param  None
  * @retval None
  */
// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
// void DMA2_Stream7_IRQHandler(void)
// {
//	DMA2->HIFCR			= DMA_HIFCR_CTCIF7;								// Clear Interrupt Flag
//	tx_state			= 0;											// Enable next transfer
//}

/**
  * @brief  This function verify UART reception and prepare USB package.
  * @param  None
  * @retval None
  */
void DMA2_Stream5_IRQHandler(void)
{
	DMA2->HIFCR 		 = DMA_HIFCR_CTCIF5;							// Clear Interrupt Flag
	uint16_t* idx;
//	TIM2->CNT			 = 0;											// Reset timeout

	// Verify frame
	if ((USB_buffer[next_bank][control] != 0xFF) ||
			(USB_buffer[next_bank][control+4] != MODE) ||
			(USB_buffer[next_bank][control+8] != 0xEE) ||
			(USB_buffer[next_bank][control+9] != 0x55))
	{
		TIM2->CR1			|= TIM_CR1_CEN;										// Start timer
		USART1->CR1			&= ~USART_CR1_RE;
		GPIOD->BSRRL		= GPIO_Pin_14;								// Blink LED 14 (Invalid Reception)
		return;
	}
	idx 	= (uint16_t *) &USB_buffer[next_bank][control+1];			// Frame code -> idx
	if (*idx == next_idx)												// Packet OK
	{
		GPIOD->BSRRH		= GPIO_Pin_14;
		if (*idx != 0xFFFF)
			next_idx = (*idx) + 1;
		else
			next_idx = 0;
		//USB_buffer[next_bank][control + USB_DATA_SIZE] 		= override_1;	// Replacing CRC with real data
		//USB_buffer[next_bank][control + USB_DATA_SIZE + 1]  = override_2;
		control 			 = control - USB_DATA_SIZE;						// Next storage for UART reception
		if (control < 0)													// USB packet filled?
		{
			USB_buffer[next_bank][4]  = 0xFF;								// Destination
			USB_buffer[next_bank][5]  = 0x05;								// Source
			USB_buffer[next_bank][6]  = MODE;								// Mode
			USB_buffer[next_bank][7]  = USB_buffer[next_bank][1];			// IDH
			USB_buffer[next_bank][8]  = USB_buffer[next_bank][2];			// IDL
			USB_buffer[next_bank][9]  = next_bank;							// test
			UsbControl 		|= ((uint32_t)0x0001 << next_bank);				// USB buffer ready to be sent
			control			 = USB_BUFFER_INIT;								// First storage position for UART reception
			next_bank = 0;													// Find free bank
			if(UsbControl != USB_FULL_BUFFER)								// Is there more space?
			{
				while(UsbControl & ((uint32_t)0x1 << next_bank))			// Find next cleared bit
				{
					next_bank++;
				}
			}
			GPIOD->ODR			^= GPIO_Pin_12;								// Blink LED 12
		}
		else
		{
			//override_1 = USB_buffer[next_bank][control + USB_DATA_SIZE];	// Storing data on CRC position
			//override_2 = USB_buffer[next_bank][control + USB_DATA_SIZE + 1];
		}
	}
	else
	{
		// Retransmission
		GPIOD->BSRRL		= GPIO_Pin_14;								// Blink LED 14 (Reception Error)
		idx = (uint16_t *) &tx_data[1];							// Frame code -> idx
		if (next_idx)
			*idx = next_idx - 1;
		else
			*idx = 0xFFFF;
	}
	DMA2_Stream5->M0AR	 = (uint32_t)&USB_buffer[next_bank][control];		// Define buffer for reception
	DMA2_Stream5->CR	|= (uint32_t)DMA_SxCR_EN;							// Enable Receiving DMA
}

/**
  * @brief  This function handles receiving timeout.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
	TIM2->SR 			 = (uint16_t)~TIM_IT_Update;						// Clear interrupt
	USART1->CR1			|= USART_CR1_RE;
	DMA2_Stream5->CR	&= ~(uint32_t)DMA_SxCR_EN;						// Disable Receiving DMA
	DMA2_Stream5->M0AR	 = (uint32_t)&USB_buffer[next_bank][control];		// Define buffer for reception
	DMA2_Stream5->CR	|= (uint32_t)DMA_SxCR_EN;						// Enable Receiving DMA
}

/**
  * @brief  This function handles UART scanning.
  * @param  None
  * @retval None
  */
void TIM6_DAC_IRQHandler(void)
{
	TIM6->SR		= (uint16_t)~TIM_IT_Update;							// Clear interrupt
	uint16_t* idx 	= (uint16_t *) &tx_data[1];							// Frame code -> idx
#ifdef _SIMULATION
	// Verify frame
	USB_buffer[next_bank][control] 		= 0xFF;
	USB_buffer[next_bank][control+4] 	= 0x01;	//MODE
	USB_buffer[next_bank][control+8] 	= 0xEE;
	USB_buffer[next_bank][control+9] 	= 0x55;
	// 2.5
	USB_buffer[next_bank][control+10]	= 0x00;
	USB_buffer[next_bank][control+11]	= 0xe8;
	USB_buffer[next_bank][control+12]	= 0xfd;
	USB_buffer[next_bank][control+13]	= 0x47;
	USB_buffer[next_bank][control+14]	= 0x00;
	USB_buffer[next_bank][control+15]	= 0xe8;
	USB_buffer[next_bank][control+16]	= 0xfd;
	USB_buffer[next_bank][control+17]	= 0x47;
	USB_buffer[next_bank][control+18]	= 0x01;
	USB_buffer[next_bank][control+19]	= 0x02;

	*((uint16_t *) &USB_buffer[next_bank][control+1]) = *idx;		// idx auto increment
	if( ((*idx) & 0xFF) == 0xFF)									// Max code value ?
	{
		GPIOD->ODR 			^= GPIO_Pin_15;							// LED 15 blinking
	}
	if (*idx != 0xFFFF)
		(*idx) = (*idx) + 1;
	else
	{
		if (!(running & 0x1))										// Capture process stopped ?
		{
			TIM6->CR1 &= (uint16_t)~TIM_CR1_CEN;					// Stop timer
			running = 0;
			return;
		}
		*idx = 0;
	}
	control 			 = control - USB_DATA_SIZE;						// Next storage for UART reception
	if (control < 0)													// USB packet filled?
	{
		USB_buffer[next_bank][4]  = 0xFF;								// Destination
		USB_buffer[next_bank][5]  = 0x05;								// Source
		USB_buffer[next_bank][6]  = MODE;								// Mode
		USB_buffer[next_bank][7]  = USB_buffer[next_bank][1];			// IDH
		USB_buffer[next_bank][8]  = USB_buffer[next_bank][2];			// IDL
		USB_buffer[next_bank][9]  = next_bank;							// test
		UsbControl 		|= ((uint32_t)0x0001 << next_bank);				// USB buffer ready to be sent
		control			 = USB_BUFFER_INIT;								// First storage position for UART reception
		next_bank = 0;													// Find free bank
		if(UsbControl != USB_FULL_BUFFER)								// Is there more space?
		{
			while(UsbControl & ((uint32_t)0x1 << next_bank))			// Find next cleared bit
			{
				next_bank++;
			}
		}
	}
#else
	// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
	if (DMA2->HISR & DMA_HISR_TCIF7)									// Transmission in progress ?
	// if (!tx_state)													// Transmission in progress ?
	{
		DMA2->HIFCR			=  DMA_HIFCR_CTCIF7;							// Transmission clear flag
		if( ((*idx) & 0xFF) == 0xFF)									// Max code value ?
		{
			GPIOD->ODR 			^= GPIO_Pin_15;							// LED 15 blinking
		}
		if((*idx) >= 0xFFFF)											// Max code value ?
		{
			if (!(running & 0x1))										// Capture process stopped ?
			{
				TIM6->CR1 &= (uint16_t)~TIM_CR1_CEN;					// Stop timer
				GPIOC->BSRRH = GPIO_Pin_8;
				running = 0;
				return;
			}
			(*idx) = 0;
		}
		else
			(*idx)++;													// Increment code value
		DMA2_Stream7->CR	|= (uint32_t)DMA_SxCR_EN;					// Start Sending DMA
		// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
		//tx_state			= 1;										// Enable transmission
	}
#endif
}



/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
