#ifndef SIMPORT_H
#define SIMPORT_H

#include "ICommChannel.h"


class ChSimPort : public ICommChannel
{
    public:
        ChSimPort();
        virtual ~ChSimPort();
        int open(const char* name);
        void close();
        int configure(const char* params[], const char* values[]);
        int send(const unsigned char *buf, int size);
        int recv(unsigned char *buf, unsigned int max_size);
        int isOpen();
        void flush();
        int hasData();

    protected:
    private:
        char buffer[65536];
        int state;
        unsigned short idx;
        union
        {
            unsigned char c[4];
            float f;
        } V;
};

#endif // SIMPORT_H
