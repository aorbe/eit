#ifndef ISTORAGE_H
#define ISTORAGE_H

#include "Voltage.h"

class IStorage
{
    public:
        IStorage() {}
        virtual ~IStorage() {}
        virtual Voltage getData() = 0;
        virtual unsigned int getData(short[32][1280]) = 0;
    protected:
    private:
};

#endif // ISTORAGE_H
