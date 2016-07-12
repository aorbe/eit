#include "ChSimPort.h"

ChSimPort::ChSimPort()
{
    for(int i=0; i<65536; i++)
        buffer[i] = 0;
    state = 0;
    idx = 0xFFFF;
}

ChSimPort::~ChSimPort()
{
    //dtor
}

int ChSimPort::open(const char* name)
{
    return true;
}

void ChSimPort::close()
{
    return;
}

int ChSimPort::configure(const char* params[], const char* values[])
{
    return true;
}

int ChSimPort::send(const unsigned char *buf, int size)
{
    if (buf[0] != 0xF1)
        return 0;

    if ( (size == 5) && (buf[1] == 0x01))
    {
        state = buf[2] + 1;
        //buf[3] == 0xF2 && buf[4] == 0x95
    }

    if ( (size == 4) && (buf[1] == 0x02))
    {
        state = 0;
        //buf[2] == 0xA3 && buf[3] == 0x2F
    }

    if ( (size == 4) && (buf[1] == 0x03))
    {
        state = 0;
        //buf[2] == 0x93 && buf[3] == 0xB9
    }

    if ( (size == 5) && (buf[1] == 0x01))
    {
        state = buf[2] + 1;
    }
    return size;
}

int ChSimPort::recv(unsigned char *buf, unsigned int max_size)
{

    if (state > 0)
    {
        if (idx >= 0xFF80)
            idx = 0xFFFF - idx + 0x80;
        else
            idx += 0x80;
        buf[0] = 0xFF;
        buf[1] = 0x05;
        buf[2] = 0x00;
        buf[3] = idx & 0xFF;
        buf[4] = idx / 0x100;
        buf[5] = 0xFF;
        V.f = 1.0;
        for(int i=0; i<2048; i++)
        {
            for(int j=0; j<4; j++)
            {
                buf[j + (i*10 + 6)] = V.c[j];
                buf[j + 4 + (i*10 + 6)] = V.c[j];
            }
        }
        return 20488>max_size?max_size:20488;

    }
    return 0;
}

int ChSimPort::isOpen()
{
    return true;
}

void ChSimPort::flush()
{
    return;
}

int ChSimPort::hasData()
{
    if (state > 0)
        return 20488;
    return 0;
}
