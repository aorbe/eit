/*
 * IProtocol.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef IPROTOCOL_H_
#define IPROTOCOL_H_

#include <string>
#include "Graph.h"
#include "IStorage.h"
#include "ICommChannel.h"
#include "Record.h"
using namespace std;

class IProtocol {
public:
	IProtocol(ICommChannel *comm)
	{
	    _comm = comm;
	}
	virtual ~IProtocol()
	{
	    _comm = 0;
	}
	void setScanTime(int value) {
        cfg.scan_time = value;
	}

    struct IData
    {
        int type;
        union
        {
            complex e[32];
            short m[32][256*5];
        };

    };
	// Interface Methods
	virtual int read(unsigned int* id) = 0;
	virtual void enable(int mode) = 0;
	virtual void disable() = 0;

    // Configuration manipulation
	virtual void readConfig(const int*, long*, int) = 0;
	virtual void writeConfig(const int*, long*, int) = 0;
    virtual void descConfig(const int*, string* desc, int) = 0;

    virtual IStorage *getStorage() = 0;

protected:
	class CONFIG {
    public:
        int scan_time;
	};
	CONFIG cfg;
	ICommChannel *_comm;
};

#endif /* IPROTOCOL_H_ */
