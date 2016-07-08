/*
 * init.h
 *
 *  Created on: 08/06/2015
 *      Author: asantos
 */

#ifndef INIT_H_
#define INIT_H_

void config(void);				// External Use

// Configuration Routines
void BasicIO_Config(void);		// I/O Initialization
void InitUSART(void);			// USART Initialization
void InitDMA(void);				// DMA Reception Initialization
void InitTIM2(void);			// TIM2 Initialization
void InitSyncTimer(void);		// TIM3 and TIM8 - Capture Clock Sync and Cycle
void InitTIM6(void);			// TIM6 - cyclic

#endif /* INIT_H_ */
