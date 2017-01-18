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
struct FP
{
	union {
		float f;
		uint32_t l;
		uint16_t i[2];
		uint8_t b[4];
	};
} sSin, sCos;


/* Function ------------------------------------------------------------------*/
void InitSinCosValues(void);


/* Variables -----------------------------------------------------------------*/
volatile int16_t samples_cont = CAPTURE_LEN;		// Counts each ADC pulse
volatile int32_t data[CAPTURE_LEN];			// Store each AD reading
volatile int16_t *sdata;
volatile uint32_t comm_ctrl = 0;		// Each bit related to one buffer
volatile uint8_t comm_data[SIZE_RCV_BUFFER][FRAME_SIZE] __attribute__ ((aligned (8)));

float Sin[20], Cos[20];
float sAvg = 0;

uint16_t x=0;
uint16_t node;

int32_t total[20];

int32_t *sDst;
int32_t *sSrc;
volatile uint8_t values[256][10] __attribute__ ((aligned (8)));

uint8_t samples_idx = 0;

__attribute__( (section(".data#") ) ) void demodula()
{

}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_DMA2 |
							RCC_AHB1Periph_GPIOA  |
							RCC_AHB1Periph_GPIOB  |
							RCC_AHB1Periph_GPIOC  |
							RCC_AHB1Periph_GPIOD  |
							RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG | RCC_APB2Periph_TIM8 | RCC_APB2Periph_USART1, ENABLE);

	sdata = data;

	for(x=0; x<SIZE_RCV_BUFFER * FRAME_SIZE; x++)
		comm_data[x/FRAME_SIZE][x%FRAME_SIZE] = 0;

	for(x=0; x<2560; x++)
		values[x/10][x%10] = 0xFF;

	config();
	InitSinCosValues();

	while (1)
	{
	    if(!samples_cont)
	    {
	    	if (comm_data[0][4])	// MODE
	    	{
	    		// Calibration Mode : Data without any treatment
	    		for(x=0;x<CAPTURE_LEN;x++)
	    		{
	    			((int16_t*)values)[x] = data[x];
	    		}
	    	}
	    	else
	    	{
	    		//demodula();
	    		// Normal Mode : Amplitude/Phase/Quality for each data set (CAPTURE_LEN)
	    		// TIM8 identifies each capture (communication is assynchronous)
	    		samples_idx = TIM8->CNT;
	    		//if (samples_idx == 0x12)
	    		//	GPIOD->ODR			^= GPIO_Pin_12;

	    		// Amplitude = (S^2 + C^2)^(.5)
	    		// Phase = atan(C/S)
	    		// S = SUM (sin(wt) .* capture)
	    		// C = SUM (cos(wt) .* capture)
	    		sSin.f = 0;
	    		sCos.f = 0;

	    		// As frequency are multiple and floating point multiplication is very slow
	    		// First all sum is done and after only 20 multiplications
	    		x = 20;
	    		sDst = total;
	    		sSrc = data;
	    		while(x > 0u)
	    		{
	    			x--;
	    			*(sDst++) = *(sSrc++);
	    		}

	    		// I dont known whether if or % are faster
	    		node = 0;
	    		for(x=20; x<CAPTURE_LEN; x++)
	    		{
	    			total[node] = total[node] +  data[x];
	    			node++;
	    			if (node >= 20)
	    				node = 0;
	    		}
	    		for(x=0; x<20; x++)
	    		{
	    			sSin.f = sSin.f + (Sin[x] * total[x]);
	    			sCos.f = sCos.f + (Cos[x] * total[x]);
	    		}

	    		// TODO: Test diferent implementations
	    	/*
	    		for(x=0;x<CAPTURE_LEN;x++)
	    		{
	    			sSin = sSin + Sin[x%20] * data[x];
	    			sCos = sCos + Cos[x%20] * data[x];
	    		}
	    	*/
	    		for(x=0; x<4; x++) {
	    			values[samples_idx][x]   = sSin.b[x];
	    			values[samples_idx][x+4] = sCos.b[x];
	    		}
	    		values[samples_idx][8] = samples_idx;
	    		values[samples_idx][9] = 0x80;

	    	}
			//sSin = 0;
			//sCos = 0;
			samples_cont = CAPTURE_LEN * 2;
	    }



	    /*
	    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3))
	    	GPIOD->BSRRH = GPIO_Pin_13;
	    else
	    	GPIOD->BSRRL = GPIO_Pin_13;
	    */

	}
	return 0;
}

//extern volatile int16_t teste[CAPTURE_LEN];

void InitSinCosValues()
{
	uint16_t t_i;
	// Sine and Cossine data
	Sin[ 0] = 0.000000000000000;
	Sin[ 1] = 0.309016994374947;
	Sin[ 2] = 0.587785252292473;
	Sin[ 3] = 0.809016994374947;
	Sin[ 4] = 0.951056516295154;
	Sin[ 5] = 1.000000000000000;
	Sin[ 6] = 0.951056516295154;
	Sin[ 7] = 0.809016994374947;
	Sin[ 8] = 0.587785252292473;
	Sin[ 9] = 0.309016994374948;
	Sin[10] = 0.000000000000000;
	Sin[11] =-0.309016994374947;
	Sin[12] =-0.587785252292473;
	Sin[13] =-0.809016994374947;
	Sin[14] =-0.951056516295154;
	Sin[15] =-1.000000000000000;
	Sin[16] =-0.951056516295154;
	Sin[17] =-0.809016994374948;
	Sin[18] =-0.587785252292473;
	Sin[19] =-0.309016994374948;

	Cos[ 0] = 1.000000000000000;
	Cos[ 1] = 0.951056516295154;
	Cos[ 2] = 0.809016994374947;
	Cos[ 3] = 0.587785252292473;
	Cos[ 4] = 0.309016994374948;
	Cos[ 5] = 0.000000000000000;
	Cos[ 6] =-0.309016994374947;
	Cos[ 7] =-0.587785252292473;
	Cos[ 8] =-0.809016994374947;
	Cos[ 9] =-0.951056516295154;
	Cos[10] =-1.000000000000000;
	Cos[11] =-0.951056516295154;
	Cos[12] =-0.809016994374948;
	Cos[13] =-0.587785252292473;
	Cos[14] =-0.309016994374948;
	Cos[15] = 0.000000000000000;
	Cos[16] = 0.309016994374947;
	Cos[17] = 0.587785252292473;
	Cos[18] = 0.809016994374947;
	Cos[19] = 0.951056516295154;

	//for(t_i=0; t_i<CAPTURE_LEN; t_i++)
	//{
	//	teste[t_i] = Cos[t_i%20]*1000;
	//}
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
