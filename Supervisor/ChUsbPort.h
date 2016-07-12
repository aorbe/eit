/*
 * UsbPort.h
 *
 *  Created on: 14/12/2013
 *      Author: asantos
 */

#ifndef USBPORT_H_
#define USBPORT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <exception>
#ifdef _WINDOWS
	#include <windows.h>

#else
	#include <libusb-1.0/libusb.h>
    #include <pthread.h>
#endif


#include "ICommChannel.h"

#define DEFAULT_BAUDRATE	19200
#define USB_BUFFER_SIZE     900000

class ChUsbPort : public ICommChannel {
public:
	ChUsbPort();
	~ChUsbPort();

	int open(const char *name);
	void close();
	int configure(const char **params, const char **values);
	int send(const unsigned char *buf, int size);
	int recv(unsigned char *buf, unsigned int max_size);
	int isOpen();
	int hasData();
	void flush();
	static int listPorts(char** buffer, unsigned int max_size);


	// Dedicated functions
	/*
	bool isCTSActive();
	bool isDSRActive();
	void setDTR(bool state);
	void setRTS(bool state);
	*/
	int cfg_default();
	static void recv_CallBack(struct libusb_transfer *transfer);

private:
#ifdef _WINDOWS

#else
    libusb_context* context;
    static libusb_device_handle *hPort;
    static unsigned char localbuf[65536];
    static unsigned char local_rcvbuf[USB_BUFFER_SIZE];
    static unsigned long local_rcvptr;
    static bool transfering;
    struct libusb_transfer *transfer;
    #define INTERFACE_NUMBER        0x00
    #define INTERFACE_ALT_NUMBER    0x01
    #define READ_ENDPOINT_NUMBER    0x81
    #define WRITE_ENDPOINT_NUMBER   0x01
#define HANDLE int
#endif
	static bool opened;
	char portname[255];
	const char* libusb_errmsg(int err);
	static void *run(void* arg);
	static pthread_mutex_t run_mutex;
	pthread_t sThread;
};


#endif /* USBPORT_H_ */
