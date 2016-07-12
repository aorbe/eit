#ifndef RECORD_H
#define RECORD_H

#include "Voltage.h"
#include <time.h>
#include<pthread.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>

class Record
{
    public:
        Record(unsigned int max_size = 256, unsigned int num_ele = 16);
        virtual ~Record();
        unsigned int getSize();
        union RECV {
            unsigned char b[10];
            struct {
                float real;
                float imag;
                unsigned char q;
                unsigned char code;
            } value;
        };
        Voltage addData(unsigned char *recvData, unsigned int num_electrode, unsigned int recv_size, unsigned int* idx);
        int getData(time_t* ts, unsigned int* id, float* real, float* imag, unsigned short* q, unsigned int);
        bool isFull();
    protected:
    private:
        time_t* timestamp;
        unsigned int *id;
        RECV **data;
        unsigned int pos;
        unsigned int max_size;
        unsigned int ele;
        pthread_mutex_t mtx;
};

#endif // RECORD_H
