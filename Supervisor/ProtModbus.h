/*
 * Modbus.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#undef _MODBUS_DEBUG
#include <stdio.h>
#include "IProtocol.h"

class ProtModbus: public IProtocol {
public:
	ProtModbus(ICommChannel *comm);
	~ProtModbus();

	int read(unsigned int *id);
	void config();
    void readConfig(const int* id, long* value, int qty);
	void writeConfig(const int* id, long* value, int qty);
	void descConfig(const int* id, string* desc, int qty);

	IStorage *getStorage();

private:
	unsigned short crc(const unsigned char* buf, int size);
	unsigned char sendData[30];
	unsigned char recvData[30];
};

#endif /* MODBUS_H_ */
