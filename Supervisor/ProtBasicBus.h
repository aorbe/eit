/*
 * BasicBusV01.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef BASICBUSV01_H_
#define BASICBUSV01_H_

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#undef _BASICBUSV01_DEBUG
#include "IProtocol.h"
#include "StDatabase.h"
#include "WMonitor.h"


class ProtBasicBus: public IProtocol {
public:
	ProtBasicBus(ICommChannel *comm);
	~ProtBasicBus();

    static const int CFG_nElectrode    = 1001;
    static const int CFG_tCycle        = 1002;
    static const int CFG_Pattern       = 1003;
    static const int CFG_nMeasure      = 1004;
    static const int CFG_tTimeout      = 1005;
    static const int CFG_fSample       = 1006;
    static const int CFG_nUnstable     = 1007;
    static const int CFG_fExcitation   = 1008;

	int read(unsigned int *id);
	void enable(int mode);
	void disable();
	void readConfig(const int*, long*, int);
	void writeConfig(const int*, long*, int);
	void descConfig(const int* id, string* desc, int qty);

	IStorage *getStorage();

private:
	unsigned short crc(const unsigned char* buf, int size);
	unsigned char sendData[32768];
	unsigned char recvData[32768];
	unsigned int rcv_size;
	void memShift(int size);
	StDatabase storage;
	//wxGLCanvas *MyGLCanvas;
	WMonitor* monitor;

	bool graphics;
};

#endif /* BASICBUSV01_H_ */
