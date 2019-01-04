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
 */

/*
 * Important files: main.c stm32f4xx_it.c comm_usb.c init.c
 * Initialization
 * main.c - tx_data is filled (Supervision protocol header - 10 bytes)
 * init.c - MCU pheripericals initialization
 * 			DMA2 Stream 5/7 for receive and send by UART
 * 			UART configuration
 * 			TIM8, TIM4 for synchronization
 * 			TIM2 for communication timeout
 * 			TIM6 for send period
 *
 * Operation
 * main.c - UART sending task is enable on running when synchronization counter reaches 251
 * stm32f4xx_it.c - Interrupt tasks
 * 			On UART reception
 * 				IF (FRAME ERROR)
 * 					Enable TIM2
 * 				ELSE
 * 					IF (MISSING PACKET)
 * 						Retransmit packet with missing ID
 * 					ELSE
 * 						Set next position for UART reception
 * 						IF (position reaches limit)
 * 							Fill header for USB transmission
 * 							Select next buffer
 * 						ENDIF
 * 					ENDIF
 * 				ENDIF
 * 			On TIM2 (UART Timeout)
 * 				Reset and enable UART reception
 * 			On TIM6 (Sending clock)
 * 				Send Acquisition Protocol packet
 * 	comm_usb.c - USB communication tasks
 * 			On USB reception
 * 				SWITCH (CODE)
 * 				CASE 0x01: Set running and enable DMA reception
 * 				CASE 0x02: Set running value to stop UART communication
 * 				CASE 0x03: If not running THEN set USB_Config
 * 				CASE 0x04:
 * 				ENDSWITCH
 * 			On USB transmission completed (two locations):
 * 				IF USB_Config THEN send parameters values packet
 * 				ELSE
 * 					Get next ready buffer/position and send it
 * 				ENDIF
 *
 *
 */


/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include "main.h"
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_usr.h"
#include "init.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_dev __ALIGN_END ;
USB_OTG_CORE_HANDLE     USB_OTG_dev;
#ifdef _STM_CDC
extern USBD_Class_cb_TypeDef  USBD_CDC_cb;
#else
extern USBD_Class_cb_TypeDef  USBD_HID_cb;
#endif

/* Private typedef -----------------------------------------------------------*/
DMA_InitTypeDef  	DMA_SendStructure;
DMA_InitTypeDef  	DMA_RecvStructure;

extern volatile uint8_t running;
volatile uint8_t tx_data[FRAME_SIZE];									// send data. Fix format

uint8_t contador				= 0;									// Temporary structure
uint16_t i, j, k;														// Auxiliary variables (mainly loops)

/* Private function prototypes -----------------------------------------------*/
void crc(uint8_t* buffer, uint16_t size);


/* Private function prototypes -----------------------------------------------*/
int main(void) {
	uint16_t count;
	uint32_t *from, *to;
	uint16_t* idx 	= (uint16_t *) &tx_data[1];							// Frame code -> idx

	unsigned int l1 = 0, l2=0;

	USBD_Init(&USB_OTG_dev,
			USB_OTG_FS_CORE_ID,
			&USR_desc,
#ifdef _STM_CDC
			&USBD_CDC_cb,
#else
			&USBD_HID_cb,
#endif
			&USR_cb);

	config();

	// Transmit packet formation
	memset(tx_data, 0x00, FRAME_SIZE);
	// Header
	tx_data[0] = 0xFF;
	tx_data[1] = 0xFF;	// ID High Byte
	tx_data[2] = 0xFF;	// ID Low Byte
	tx_data[3] = 0x01;	// Hop Number
	MODE	   = 0x00;	// Mode
	tx_data[5] = 0x00;	// Parameter
	tx_data[6] = 0x00;	// Value
	tx_data[7] = 0x00;	//
	tx_data[8] = 0xEE;  // CRC
	tx_data[9] = 0x55;  // CRC

	while (1)
	{
		// PENSAR
		if((tx_data[4] || (TIM3->CNT == 127))  && (running & 0x01))
		{
			tx_data[1] = 0x00;
			tx_data[2] = 0x00;
			running = 0x02;
			DMA2_Stream7->CR	|= (uint32_t)DMA_SxCR_EN;					// Start Sending DMA
			DMA2_Stream5->CR	|= (uint32_t)DMA_SxCR_EN;					// Enable Receiving DMA
		}

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

// CRC Calculation
void crc(uint8_t* buffer, uint16_t size)
{
	uint8_t z;
	uint16_t x, tmp = 0xFFFF; 	// initialize value
	for(x=0; x<size-2; x++)		// number of bytes to process
	{
		tmp ^= *(buffer+x);		// read through data array

		for(z=0; z<8; z++)
		{
			if(tmp & 0x01)
			{
				tmp = 0x0A001 ^ (tmp >> 1);
			}
			else
			{
				tmp >>= 1;
			}
		}
	}
	buffer[size-2] = tmp >> 8;
	buffer[size-1] = tmp & 0xFF;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
