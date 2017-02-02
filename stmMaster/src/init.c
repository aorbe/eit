/*
 * init.c
 *
 *  Created on: 08/06/2015
 *      Author: asantos
 *
 *
 * I/O
 * PD12/13/14/15 - LEDs
 * PA0 - Master sync clock input (125KHz from generator)
 * PC6 - Master sync clock output (1.6KHz for normal operation and for calibration)
 * PC8 - Master cycle synchronization (1 transition each 512 sync clock pulses)
 * PB6 - Tx | UART Communication in 10.5Mbps
 * PB7 - Rx |
 *
 */

#include "stm32f4_discovery.h"
#include "stm32f4xx_tim.h"
#include "main.h"
#include "init.h"

DMA_InitTypeDef  	DMA_SendStructure;
DMA_InitTypeDef  	DMA_RecvStructure;
DMA_InitTypeDef  	DMA_UsbStructure;

extern volatile uint8_t tx_data[FRAME_SIZE];			// send data. Fix format
extern volatile uint8_t USB_buffer[USB_NUM_BUFFERS][USB_BUFFER_SIZE];									// Reception buffer


void config(void)
{
	/* GPIOD, GPIOE and GPIOE - Enabling Clock */
	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_GPIOA |
							RCC_AHB1Periph_GPIOB |
							RCC_AHB1Periph_GPIOC |
							RCC_AHB1Periph_GPIOD |
							RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_USART1 | RCC_APB2Periph_SYSCFG, ENABLE);


	BasicIO_Config();			// All Pins

	InitUSART();

#ifdef _SIMULATION
	// Simulation has no UART communication
#else
	InitDMA();
	USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

	InitTIM3();		// USART Timeout
#endif
	InitSyncTimer();
}

// Config Blue Button and four LEDs
void BasicIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructureLEDs;
	GPIO_InitTypeDef GPIO_InitStructureINP;
	GPIO_InitTypeDef GPIO_InitStructureOC;
	GPIO_InitTypeDef GPIO_InitStructureUSART;

	/* Configure sync input */
	GPIO_StructInit(&GPIO_InitStructureINP);
	GPIO_InitStructureINP.GPIO_Pin		= GPIO_Pin_0;
	GPIO_InitStructureINP.GPIO_Mode		= GPIO_Mode_AF;
	GPIO_InitStructureINP.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructureINP.GPIO_PuPd		= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructureINP);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM8);

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode - LEDs */
	GPIO_InitStructureLEDs.GPIO_Pin		= GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructureLEDs.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructureLEDs.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructureLEDs.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructureLEDs.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructureLEDs);

	// Cycle - PB8
	GPIO_InitStructureOC.GPIO_Pin			= GPIO_Pin_8;
	GPIO_InitStructureOC.GPIO_Mode			= GPIO_Mode_AF;
	GPIO_InitStructureOC.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructureOC.GPIO_OType			= GPIO_OType_PP;
	GPIO_InitStructureOC.GPIO_PuPd			= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructureOC);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);

	// Sync Output (PC6)
	GPIO_InitStructureOC.GPIO_Pin			= GPIO_Pin_6;
	GPIO_InitStructureOC.GPIO_Mode			= GPIO_Mode_AF;
	GPIO_InitStructureOC.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructureOC.GPIO_OType			= GPIO_OType_PP;
	GPIO_InitStructureOC.GPIO_PuPd			= GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructureOC);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);

	// Tx Pin - PB6
	GPIO_InitStructureUSART.GPIO_Pin	= GPIO_Pin_6;
	GPIO_InitStructureUSART.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructureUSART.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructureUSART.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructureUSART.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructureUSART);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);

	// Rx Pin - PB7
	GPIO_InitStructureUSART.GPIO_Pin	= GPIO_Pin_7;
	GPIO_InitStructureUSART.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructureUSART.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructureUSART.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructureUSART.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructureUSART);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

}

// Config Timer 2
// Single interrupt after timer expired
// Timer is enabled after sending
// Shoud be 1/BaudRate * 10 * PacketSize * 1.02 (tolerance for jitter and processing delays)
void InitTIM3()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef   		NVIC_InitStructure;

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period 						= RCV_TIMEOUT - 1; // Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler						= 84 - 1; // 42 MHz Clock down to 1 MHz used above (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision					= 0;
	TIM_TimeBaseStructure.TIM_CounterMode					= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

// Config Timer 3 and 1
// TIM8 generates main clock signal
// TIM4 generates sync clock signal
void InitSyncTimer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef		TIM_InitStructureOC;
	TIM_BDTRInitTypeDef		TIM_InitStructureBDTR;
	NVIC_InitTypeDef   		NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
	TIM_Cmd(TIM5, DISABLE);
	TIM_Cmd(TIM8, DISABLE);

	/* TIM5 Update - Communication Trigger */
	NVIC_InitStructure.NVIC_IRQChannel 						= TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM8 - Receive clock from main generator */
	// TESTING:
	TIM_TimeBaseStructure.TIM_Period 		= 12;				// Counts until interrupt generation 125/1.6 / 2 (Toogle) -> 38 (Run Mode)
	//TIM_TimeBaseStructure.TIM_Period 		= 17499;				// Counts until interrupt generation 125/1.6 / 2 (Toogle) -> 38 (Run Mode)
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;				// This is changed for calibration from 0 to 1023
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
	// TESTING:
	TIM_ETRClockMode1Config(TIM8, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
	TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_OC1);
	TIM_SelectMasterSlaveMode(TIM8, TIM_MasterSlaveMode_Enable);

	/* TIM5 - Communication Cycle */
	TIM_TimeBaseStructure.TIM_Period 		= 5;				// Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;				//
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	// TIM5 is slave of TIM8
	TIM_SelectInputTrigger(TIM5, TIM_TS_ITR3);
	TIM_SelectSlaveMode(TIM5, TIM_SlaveMode_External1);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

	/* TIM4 - Main Sync Pulse Output */
	TIM_TimeBaseStructure.TIM_Period 		= 2;
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	// TIM4 is slave of TIM8
	TIM_SelectInputTrigger(TIM4, TIM_TS_ITR3);
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_External1);
	// TIM4 outputs in PC8
	TIM_OCStructInit(&TIM_InitStructureOC);
	TIM_InitStructureOC.TIM_OCMode			= TIM_OCMode_Toggle;
	TIM_InitStructureOC.TIM_OutputState		= TIM_OutputState_Enable;
	TIM_InitStructureOC.TIM_Pulse			= 1;
	TIM_InitStructureOC.TIM_OCPolarity		= TIM_OCPolarity_Low;
	TIM_InitStructureOC.TIM_OCIdleState		= TIM_OCIdleState_Reset;
	TIM_OC3Init(TIM4, &TIM_InitStructureOC);
	TIM_CCxCmd(TIM4, TIM_Channel_3, TIM_CCx_Enable);
	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);

	/* TIM3 - Cycle Pulse Output */
	TIM_TimeBaseStructure.TIM_Period 		= 255;				// Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;				//
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	// TIM3 is slave of TIM4
	TIM_SelectInputTrigger(TIM3, TIM_TS_ITR3);
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_External1);
	// TIM3
	TIM_OCStructInit(&TIM_InitStructureOC);
	TIM_InitStructureOC.TIM_OCMode			= TIM_OCMode_Toggle;
	TIM_InitStructureOC.TIM_OutputState		= TIM_OutputState_Enable;
	TIM_InitStructureOC.TIM_Pulse			= 127;
	TIM_InitStructureOC.TIM_OCPolarity		= TIM_OCPolarity_Low;
	TIM_OC1Init(TIM3, &TIM_InitStructureOC);
	TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Enable);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);



	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
	TIM_Cmd(TIM8, ENABLE);
}

// Config USART1
// Clock is 84 MHz and /8 is used
// 8 data, 1 stop, no parity, 1 start = 10 bits
// TX = PB6  RX = PB7
void InitUSART()
{
	USART_InitTypeDef	USART_InitStructure;

	USART_OverSampling8Cmd(USART1, ENABLE);		// Normally is 16
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate		= SERIAL_BAUD;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

// Config Sending DMA
// Receiving UART uses DMA2 Stream5 Channel 4
// Transmiting UART uses DMA2 Stream7 Channel 4
void InitDMA()
{
	NVIC_InitTypeDef   	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel					 = DMA2_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd				 = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
	// NVIC_InitStructure.NVIC_IRQChannel					 = DMA2_Stream7_IRQn;
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority		 = 0;
	// NVIC_InitStructure.NVIC_IRQChannelCmd				 = ENABLE;
	// NVIC_Init(&NVIC_InitStructure);

	DMA_SendStructure.DMA_Channel				 = DMA_Channel_4;
	DMA_SendStructure.DMA_DIR					 = DMA_DIR_MemoryToPeripheral; // Sending
	DMA_SendStructure.DMA_Memory0BaseAddr		 = (uint32_t)tx_data;
	DMA_SendStructure.DMA_MemoryInc				 = DMA_MemoryInc_Enable;
	DMA_SendStructure.DMA_BufferSize			 = FRAME_SIZE;
	DMA_SendStructure.DMA_PeripheralBaseAddr	 = (uint32_t)&USART1->DR;
	DMA_SendStructure.DMA_PeripheralInc			 = DMA_PeripheralInc_Disable;

	DMA_SendStructure.DMA_PeripheralDataSize	 = DMA_PeripheralDataSize_Byte;
	DMA_SendStructure.DMA_MemoryDataSize		 = DMA_MemoryDataSize_Byte;

	DMA_SendStructure.DMA_Mode					 = DMA_Mode_Normal;
	DMA_SendStructure.DMA_Priority				 = DMA_Priority_VeryHigh;
	DMA_SendStructure.DMA_FIFOMode				 = DMA_FIFOMode_Disable;
	DMA_SendStructure.DMA_FIFOThreshold			 = DMA_FIFOThreshold_Full;
	DMA_SendStructure.DMA_MemoryBurst			 = DMA_MemoryBurst_Single;
	DMA_SendStructure.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;

	DMA_RecvStructure.DMA_Channel				 = DMA_Channel_4;
	DMA_RecvStructure.DMA_DIR					 = DMA_DIR_PeripheralToMemory; // Receive
	DMA_RecvStructure.DMA_Memory0BaseAddr		 = (uint32_t)&USB_buffer[0][USB_BUFFER_INIT];
	DMA_RecvStructure.DMA_MemoryInc				 = DMA_MemoryInc_Enable;
	DMA_RecvStructure.DMA_BufferSize		 	 = FRAME_SIZE;
	DMA_RecvStructure.DMA_PeripheralBaseAddr	 = (uint32_t)&USART1->DR;
	DMA_RecvStructure.DMA_PeripheralInc			 = DMA_PeripheralInc_Disable;

	DMA_RecvStructure.DMA_PeripheralDataSize	 = DMA_PeripheralDataSize_Byte;
	DMA_RecvStructure.DMA_MemoryDataSize		 = DMA_MemoryDataSize_Byte;

	DMA_RecvStructure.DMA_Mode					 = DMA_Mode_Normal;
	DMA_RecvStructure.DMA_Priority				 = DMA_Priority_VeryHigh;
	DMA_RecvStructure.DMA_FIFOMode				 = DMA_FIFOMode_Disable;
	DMA_RecvStructure.DMA_FIFOThreshold			 = DMA_FIFOThreshold_Full;
	DMA_RecvStructure.DMA_MemoryBurst			 = DMA_MemoryBurst_Single;
	DMA_RecvStructure.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;

	DMA_DeInit(DMA2_Stream7);
	DMA_Init(DMA2_Stream7, &DMA_SendStructure);
	// VERIFY: Using DMA2_Stream7 IF to verify transmission completed
	// DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

	DMA_DeInit(DMA2_Stream5);
	DMA_Init(DMA2_Stream5, &DMA_RecvStructure);
	DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
}

