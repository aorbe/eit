/*
 * init.h
 *
 *  Created on: 23/04/2015
 *      Author: asantos
 */

#ifndef INIT_H_
#define INIT_H_


void config(void);			// External Use

void BasicIO_Config(void);
void InitUSART(void);
void InitDMA(void);
void InitExtInterrupt(void);
void InitSyncTimer(void);
void InitTimeout(void);
void InitDataAvailableTimer(void);

#endif /* INIT_H_ */
