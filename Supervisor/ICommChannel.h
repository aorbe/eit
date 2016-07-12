/*
 * ICommChannel.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef ICOMMCHANNEL_H_
#define ICOMMCHANNEL_H_

class ICommChannel {
public:
    ICommChannel() {};
	virtual ~ICommChannel() {};
	virtual int open(const char* name) = 0;
	virtual void close() = 0;
	virtual int configure(const char* params[], const char* values[]) = 0;
	virtual int send(const unsigned char *buf, int size) = 0;
	virtual int recv(unsigned char *buf, unsigned int max_size) = 0;
	virtual int isOpen() = 0;
	virtual void flush() = 0;
    virtual int hasData() = 0;
};

#endif /* ICOMMCHANNEL_H_ */
