#include "Record.h"
#include <complex.h>

Record::Record(unsigned int max_size, unsigned int num_ele)
{
    //  printf("Record Initializing %d %d\n", max_size, num_ele);
    this->max_size = max_size;
    timestamp = new time_t[max_size];
    id = new unsigned int[max_size];
    data = new RECV*[max_size];
    for(unsigned int i=0; i<max_size; i++)
        data[i] = new RECV[num_ele];
    ele = num_ele;
    pos = 0;
}

Record::~Record()
{
    //printf("Destroying record\n");
    delete timestamp;
    delete id;
    for(unsigned int i=0; i<max_size; i++)
        delete data[i];
    delete data;
}

unsigned int Record::getSize()
{
    return pos;
}

bool Record::isFull()
{
    return (pos >= max_size);
}

Voltage Record::addData(unsigned char *recvData, unsigned int num_electrode, unsigned int recv_size, unsigned int *idx)
{
    Voltage measure;

    *idx = ((unsigned int)recvData[3])+((unsigned int)recvData[4])*256;
    measure.id = *idx;

    for(unsigned int y=0; y<((recv_size-8)/10/num_electrode);y++)
    {
        if (pos >= max_size)
        {
            printf("**** BUFFER OVERFLOW ****\n");
            pos--;
        }

        timestamp[pos] = time(NULL);
        id[pos] = *idx;
        for(unsigned int x=0; x<num_electrode; x++)
        {
            memcpy((void*)  &data[pos][x].b[0], (void*) &recvData[6+(y*num_electrode+x)*10], 10);
            /*
            if (data[pos][x].b[0] == 0xFF && data[pos][x].b[1] == 0xFF && data[pos][x].b[2] == 0xFF && data[pos][x].b[3] == 0xFF)
            {
                printf("Erro %04X Chan%02d\n", *idx, x);
            }
            */
        }
//        printf("Amplitude %06d %f %f mV\n", *idx, sqrt(pow(data[pos][0].value.real,2) + (pow(data[pos][0].value.imag,2)))*2000/32768.0/1020,
//               sqrt(pow(data[pos][1].value.real,2) + (pow(data[pos][1].value.imag,2)))*2000/32768.0/1020);
        if (data[pos][0].b[0] == 0xFF && data[pos][0].b[1] == 0xFF && data[pos][0].b[2] == 0xFF && data[pos][0].b[3] == 0xFF)
        {
            printf("Erro %04X Channel 0\n", *idx);
        }
        pos++;
        *idx = (*idx + 1) % 65536;
    }

    measure.type = 1;
    for(unsigned int z=0; z<num_electrode; z++)
    {
        measure.values[z] = data[pos-1][z].value.real + I * data[pos-1][z].value.imag;
    }
    return measure;
}

int Record::getData(time_t* ts, unsigned int* id, float* real, float* imag, unsigned short* q, unsigned int itemsize)
{
    pos--;
    *ts = timestamp[pos];
    *id = this->id[pos];
    for(unsigned int i=0; i<ele && i<itemsize; i++)
    {
        real[i] = data[pos][i].value.real;
        imag[i] = data[pos][i].value.imag;
        q[i] = data[pos][i].value.q;
    }

    return 0;
}
