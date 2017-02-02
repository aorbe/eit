/*
 * init.c
 *
 *  Created on: 23/04/2015
 *      Author: asantos
 */
#include "main.h"
#include "stm32f4xx.h"
#include "arm_math.h"
#include "init.h"

extern volatile int16_t data[CAPTURE_LEN];	// Store each AD reading
extern volatile uint8_t comm_data[SIZE_RCV_BUFFER][FRAME_SIZE];

int16_t erro = 20000;

DMA_InitTypeDef   	DMA_SendStructure;
DMA_InitTypeDef  	DMA_RecvStructure;
DMA_InitTypeDef  	DMA_DataStructure;

void config()
{
	NVIC_InitTypeDef NVIC_InitStructure2;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	BasicIO_Config();

	InitUSART();				// UART1 are used (PB6/PB7)
	InitDMA();					// Communication uses DMA
	InitTimeout();				// Communication timeout
	InitSyncTimer();			// Counting of cycles
	InitDataAvailableTimer();
	DMA1_Stream1->CR		|= DMA_SxCR_EN;		// Recv

}

/* Configure All GPIO channels
 */
void BasicIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode - LEDs	*/
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Configure Port E as input for 16 bit data from ADC					*/
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Configure PA08 (DAV) as TIM1_CH1  */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);

	/* Configure PA03 (OTR) as input  */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PA00 (Sync - TIM8 ETR) as input  */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	//GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM8);

	/* Configure PC06 (Cycle/Reset - TIM8_CH1) as input  */
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);

	// Tx Pin - PD08
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);

	// Rx Pin - PD09
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

}

/* Config TIM3
 *
 * Single interrupt after timer expired
 * Timer is enabled after sending
 * Shoud be 1/BaudRate * 10 * PacketSize * 1.02 (tolerance for jitter and processing delays)
 * */
void InitTimeout()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef   	NVIC_InitStructure;

	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	/*
	 * USART in 3.5Mbps, transfering 10 bytes (10 bits per byte sent, totalizing 100 bits or 35Kpackages/s)
	 *
	 */
	TIM_TimeBaseStructure.TIM_Period		= 180; // RCV_TIMEOUT - 1; // Counts until interrupt generation
	TIM_TimeBaseStructure.TIM_Prescaler		= 42 - 1; // 42 MHz Clock down to 1 MHz used above (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);

	//TIM_Cmd(TIM3, ENABLE);

}

/* Config TIM5
 * DMA1 Stream1 Channel 6
 */
void InitDataAvailableTimer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef 		TIM_ICInitStruct;
	NVIC_InitTypeDef   	NVIC_InitStructure;

	/* Time base configuration */
	/* External clock counter
	 */
	TIM_TimeBaseStructure.TIM_Period		= CAPTURE_LEN; //
	TIM_TimeBaseStructure.TIM_Prescaler		= 0; //
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICFilter = 0x00;

	TIM_ICInit(TIM1, &TIM_ICInitStruct);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);

	TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);
	TIM_TIxExternalClockConfig(TIM1, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0x00);

	/* Enable DMA2 Stream 1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_DataStructure.DMA_Channel					= DMA_Channel_6;
	DMA_DataStructure.DMA_DIR						= DMA_DIR_PeripheralToMemory; // Reveiving
	DMA_DataStructure.DMA_Memory0BaseAddr			= (uint32_t)&data[0];
	DMA_DataStructure.DMA_MemoryInc					= DMA_MemoryInc_Enable;
	DMA_DataStructure.DMA_BufferSize				= CAPTURE_LEN;
	DMA_DataStructure.DMA_PeripheralBaseAddr		= (uint32_t)&GPIOE->IDR;
	DMA_DataStructure.DMA_PeripheralInc				= DMA_PeripheralInc_Disable;

	DMA_DataStructure.DMA_PeripheralDataSize		= DMA_PeripheralDataSize_HalfWord;
	DMA_DataStructure.DMA_MemoryDataSize			= DMA_MemoryDataSize_HalfWord;

	DMA_DataStructure.DMA_Mode						= DMA_Mode_Normal;
	DMA_DataStructure.DMA_Priority					= DMA_Priority_High;
	DMA_DataStructure.DMA_FIFOMode					= DMA_FIFOMode_Enable;
	DMA_DataStructure.DMA_FIFOThreshold				= DMA_FIFOThreshold_Full;
	DMA_DataStructure.DMA_MemoryBurst				= DMA_MemoryBurst_INC8;
	DMA_DataStructure.DMA_PeripheralBurst			= DMA_PeripheralBurst_Single;

	DMA_DeInit(DMA2_Stream1);
	DMA_Init(DMA2_Stream1, &DMA_DataStructure);
	DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);

	TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

}


// SyncTimer is TIM8
// which counter value (0-255) is used as sample identifier
void InitSyncTimer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef		TIM_ICInitStructure;
	NVIC_InitTypeDef   		NVIC_InitStructure;
	EXTI_InitTypeDef		EXTI_InitStructure;

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource6);

	/* Configure EXTI Line6 for cycle syncronization */
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line1 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Increase each sync pulse (1.6kHz in RUN mode) */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period		= 255;
	TIM_TimeBaseStructure.TIM_Prescaler		= 0;
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
	TIM_SelectOnePulseMode(TIM8, TIM_OPMode_Repetitive);
	//
	TIM_ETRClockMode1Config(TIM8, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);


	/* Enable the TIM8 trigger Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel						= TIM8_TRG_COM_TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM8, TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM8, ENABLE);

}

// Config USART3
// Clock is 42 MHz and /8 is used
// 8 data, 1 stop, no parity, 1 start = 10 bits
// TX = PD8  RX = PD9
void InitUSART()
{
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef   	NVIC_InitStructure;

	USART_OverSampling8Cmd(USART3, ENABLE);		// Normally is 16
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = SERIAL_BAUD;
	USART_Init(USART3, &USART_InitStructure);

	USART_Cmd(USART3, ENABLE);
}

// DMA Channel and number are related to internal peripheral (UART3) used
// DMA1 Stream1 Channel 4 - USART3_RX
// DMA1 Stream4 Channel 7 - USART3_TX
void InitDMA()
{
	NVIC_InitTypeDef   	NVIC_InitStructure;

	/* Enable DMA1 Stream 1 Interrupt - Reception*/
	NVIC_InitStructure.NVIC_IRQChannel						= DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_SendStructure.DMA_Channel					= DMA_Channel_7;
	DMA_SendStructure.DMA_DIR						= DMA_DIR_MemoryToPeripheral; // Sending
	DMA_SendStructure.DMA_Memory0BaseAddr			= (uint32_t)comm_data[0];
	DMA_SendStructure.DMA_MemoryInc					= DMA_MemoryInc_Enable;
	DMA_SendStructure.DMA_BufferSize				= FRAME_SIZE;
	DMA_SendStructure.DMA_PeripheralBaseAddr		= (uint32_t)&USART3->DR;
	DMA_SendStructure.DMA_PeripheralInc				= DMA_PeripheralInc_Disable;

	DMA_SendStructure.DMA_PeripheralDataSize		= DMA_PeripheralDataSize_Byte;
	DMA_SendStructure.DMA_MemoryDataSize			= DMA_MemoryDataSize_Byte;

	DMA_SendStructure.DMA_Mode						= DMA_Mode_Normal;
	DMA_SendStructure.DMA_Priority					= DMA_Priority_High;
	DMA_SendStructure.DMA_FIFOMode					= DMA_FIFOMode_Disable;
	DMA_SendStructure.DMA_FIFOThreshold				= DMA_FIFOThreshold_Full;
	DMA_SendStructure.DMA_MemoryBurst				= DMA_MemoryBurst_Single;
	DMA_SendStructure.DMA_PeripheralBurst			= DMA_PeripheralBurst_Single;

	DMA_RecvStructure.DMA_Channel					= DMA_Channel_4;
	DMA_RecvStructure.DMA_DIR						= DMA_DIR_PeripheralToMemory; // Reveiving
	DMA_RecvStructure.DMA_Memory0BaseAddr			= (uint32_t)comm_data[0];
	DMA_RecvStructure.DMA_MemoryInc					= DMA_MemoryInc_Enable;
	DMA_RecvStructure.DMA_BufferSize				= FRAME_SIZE;
	DMA_RecvStructure.DMA_PeripheralBaseAddr		= (uint32_t)&USART3->DR;
	DMA_RecvStructure.DMA_PeripheralInc				= DMA_PeripheralInc_Disable;

	DMA_RecvStructure.DMA_PeripheralDataSize		= DMA_PeripheralDataSize_Byte;
	DMA_RecvStructure.DMA_MemoryDataSize			= DMA_MemoryDataSize_Byte;

	DMA_RecvStructure.DMA_Mode						= DMA_Mode_Normal;
	DMA_RecvStructure.DMA_Priority					= DMA_Priority_High;
	DMA_RecvStructure.DMA_FIFOMode					= DMA_FIFOMode_Disable;
	DMA_RecvStructure.DMA_FIFOThreshold				= DMA_FIFOThreshold_Full;
	DMA_RecvStructure.DMA_MemoryBurst				= DMA_MemoryBurst_Single;
	DMA_RecvStructure.DMA_PeripheralBurst			= DMA_PeripheralBurst_Single;

	DMA_DeInit(DMA1_Stream4);
	DMA_Init(DMA1_Stream4, &DMA_SendStructure);

	DMA_DeInit(DMA1_Stream1);
	DMA_Init(DMA1_Stream1, &DMA_RecvStructure);
	DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);

	USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

	DMA_Cmd(DMA1_Stream4, DISABLE);

}

