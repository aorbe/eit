/*
 * Voltage.h
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#ifndef VOLTAGE_H_
#define VOLTAGE_H_
#include <complex.h>

class Voltage {
    public:
    int type;
    unsigned int id;
    complex values[32];
    short amplitude[32][1280];
};

#endif /* VOLTAGE_H_ */
