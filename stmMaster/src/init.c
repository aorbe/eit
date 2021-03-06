/*
 * init.c
 *
 *  Created on: 08/06/2015
 *      Author: asantos
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

	InitTIM2();		// USART Timeout
#endif
	InitSyncTimer();
	InitTIM6();
}

// Config Blue Button and four LEDs
void BasicIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructureLEDs;
//	GPIO_InitTypeDef GPIO_InitStructureBTN;
	GPIO_InitTypeDef GPIO_InitStructureOC;
	GPIO_InitTypeDef GPIO_InitStructureUSART;

	/* Configure user button as input */
//	GPIO_InitStructureBTN.GPIO_Pin		= GPIO_Pin_0;
//	GPIO_InitStructureBTN.GPIO_Mode		= GPIO_Mode_IN;
//	GPIO_InitStructureBTN.GPIO_Speed	= GPIO_Speed_2MHz;
//	GPIO_InitStructureBTN.GPIO_PuPd		= GPIO_PuPd_NOPULL;
//	GPIO_Init(GPIOA, &GPIO_InitStructureBTN);

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode - LEDs */
	GPIO_InitStructureLEDs.GPIO_Pin		= GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructureLEDs.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructureLEDs.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructureLEDs.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructureLEDs.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructureLEDs);

	// Cycle
	GPIO_InitStructureOC.GPIO_Pin			= GPIO_Pin_8;
	//GPIO_InitStructureOC.GPIO_Mode			= GPIO_Mode_AF;
	GPIO_InitStructureOC.GPIO_Mode			= GPIO_Mode_OUT;
	GPIO_InitStructureOC.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructureOC.GPIO_OType			= GPIO_OType_PP;
	GPIO_InitStructureOC.GPIO_PuPd			= GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructureOC);
	//GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM3);

	// SYNC
	GPIO_InitStructureOC.GPIO_Pin			= GPIO_Pin_8;
	GPIO_InitStructureOC.GPIO_Mode			= GPIO_Mode_AF;
	GPIO_InitStructureOC.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructureOC.GPIO_OType			= GPIO_OType_PP;
	GPIO_InitStructureOC.GPIO_PuPd			= GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructureOC);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);

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
void InitTIM2()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef   		NVIC_InitStructure;

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM2 clock enable */
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
// TIM1 generates main clock signal
// TIM3 generates sync clock signal
void InitSyncTimer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef		TIM_InitStructureOC;
	TIM_BDTRInitTypeDef		TIM_InitStructureBDTR;
	NVIC_InitTypeDef   		NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM1, DISABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period 		= CAPTURE_SCAN - 1;		// Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler		= 84 - 1;				// 84 MHz Clock down to 1 MHz used above (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period 		= 511;				// Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;				//
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/*
	TIM_InitStructureOC.TIM_OCMode			= TIM_OCMode_Toggle;
	TIM_InitStructureOC.TIM_OutputState		= TIM_OutputState_Enable;
	TIM_InitStructureOC.TIM_Pulse			= 25;
	TIM_InitStructureOC.TIM_OCPolarity		= TIM_OCPolarity_Low;
	TIM_InitStructureOC.TIM_OCIdleState		= TIM_OCIdleState_Reset;
	TIM_OC3Init(TIM3, &TIM_InitStructureOC);
	TIM_CCxCmd(TIM3, TIM_Channel_3, TIM_CCx_Enable);
	*/

	TIM_OCStructInit(&TIM_InitStructureOC);
	TIM_InitStructureOC.TIM_OCMode			= TIM_OCMode_Toggle;
	TIM_InitStructureOC.TIM_OutputState		= TIM_OutputState_Enable;
	TIM_InitStructureOC.TIM_Pulse			= 250;
	TIM_InitStructureOC.TIM_OCPolarity		= TIM_OCPolarity_Low;
	TIM_OC1Init(TIM1, &TIM_InitStructureOC);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);

	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
	TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);

	TIM_SelectInputTrigger(TIM3, TIM_TS_ITR0);
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_External1);

	TIM_Cmd(TIM3, ENABLE);
	TIM_Cmd(TIM1, ENABLE);
}

// Config Timer 6
// Cyclic interrupt after timer expired
// SCAN_PERIOD (in us)
void InitTIM6()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef   		NVIC_InitStructure;

	/* Enable the TIM6 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel 						= TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM6 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period 			= SCAN_PERIOD - 1; 	// Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler 		= 84 - 1; 	// 42 MHz Clock down to 1 MHz used above (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision 	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	TIM_SelectOnePulseMode(TIM6, TIM_OPMode_Repetitive);

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
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

