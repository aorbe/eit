/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"

extern volatile int16_t data[1020];	// Store each AD reading
extern volatile int16_t samples_cont;		// Counts each ADC pulse


inline void EXTI1_IRQHandler(void) {
	EXTI->PR = 0x0002;						// Clear the EXTI line 1 pending bit
	if (samples_cont)							// Reach limit
		data[--samples_cont] = GPIOE->IDR;
	else
	{
		//EXTI->IMR &= ~EXTI_Line1;	// hardcoded for faster execution?
		EXTI->IMR 	&= 0xFFFE; 					// Disable DAV Interrupt
	}
}

