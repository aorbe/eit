/*
 * SerialPort.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <stdio.h>
#include <stdlib.h>
#ifdef _WINDOWS
	#include <windows.h>
	#include <cstdio>
#else
	#include <errno.h>
	#include <termios.h>
	#include <unistd.h>
	#include <sys/types.h>
//	#include <stdio.h>
	#include <fcntl.h>
	#include <sys/signal.h>
	#include <sys/stat.h>
	#include <sys/ioctl.h>
	#include <dirent.h>
  #endif
//#include <stdlib.h>
#include <memory.h>
#include "ICommChannel.h"

#define DEFAULT_BAUDRATE	19200

class ChSerialPort : public ICommChannel {
public:
	ChSerialPort();
	~ChSerialPort();

	int open(const char *name);
	void close();
	int configure(const char *params[], const char *values[]);
	int send(const unsigned char *buf, int size);
	int recv(unsigned char *buf, unsigned int max_size);
	int isOpen();
	void flush();
	int hasData();
	static int listPorts(char** buffer, unsigned int max_size);

	// Dedicated functions
	/*
	bool isCTSActive();
	bool isDSRActive();
	void setDTR(bool state);
	void setRTS(bool state);
	*/
	int cfg_default();

private:
#ifdef _WINDOWS
	DCB port_settings;
	COMMTIMEOUTS Cptimeouts;

#else
	int GetLastError() { return errno;}
	struct termios port_settings;
#define HANDLE int
#define INVALID_HANDLE_VALUE (-1)
#endif
	HANDLE hPort;
	bool opened;
	char portname[255];

};

#endif /* SERIALPORT_H_ */
