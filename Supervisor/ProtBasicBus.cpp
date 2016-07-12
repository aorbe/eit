/*
 * BasicBusV01.cpp
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */
#include "ProtBasicBus.h"
#undef _MODBUS_DEBUG
#define IGNORE_CRC

#include "Supervisor.h"

#define MAX_DISPLAY 200

ProtBasicBus::ProtBasicBus(ICommChannel *comm) : IProtocol::IProtocol(comm)
{

	sendData[0] = 0x00;
	sendData[1] = 0x04;
	sendData[2] = 0x03;
	sendData[3] = 0x00;
	sendData[4] = 0x00;
	sendData[5] = 0x10;
	unsigned short calc = crc(sendData, 6);
	sendData[6] = calc >> 8;
	sendData[7] = calc & 0xff;

	char *p[2], *v[2];
	p[0] = new char[20];    sprintf(p[0], "%s", "BAUDRATE");
	v[0] = new char[20];    sprintf(v[0], "%s", "4000000");
	p[1] = new char[1];     sprintf(p[1], "%s", "");
	v[1] = new char[1];     sprintf(v[1], "%s", "");
    comm->configure((const char**)p, (const char**)v);

    rcv_size = 0;

    monitor = NULL;

    printf("BasicBus Initialized\n");
}

ProtBasicBus::~ProtBasicBus()
{
	// TODO Auto-generated destructor stub
}

void ProtBasicBus::enable(int mode)
{
    unsigned char ENABLE_CMD[5] = {0xF1, 0x01, (unsigned char) mode, 0xF2, 0x95};
    memcpy(sendData, ENABLE_CMD, 5);
    _comm->send(sendData, 5);
}

void ProtBasicBus::disable()
{
    unsigned char DISABLE_CMD[4] = {0xF1, 0x02, 0xA3, 0x2F};
    memcpy(sendData, DISABLE_CMD, 4);
    _comm->send(sendData, 4);
}

// Returned values
// -201 Invalid Size
// -202
// -203
// -204


int ProtBasicBus::read(unsigned int *id)
{
    int r, i, state = 0;
    static unsigned int count = 0;
    unsigned int expected_size = 20488;

    r = _comm->recv(&recvData[rcv_size], expected_size - rcv_size);

    if (r > 0)
    {
        rcv_size = rcv_size + r;
        #ifdef _MODBUS_DEBUG
        printf("Received Size: %d bytes\n", rcv_size);
        #endif
    }
    else
    {
        if (r != 0)
        {
            printf("Error: Device Not Opened\n");
        }
        return r;
    }

    if  (rcv_size < 4)
    {
        //printf("Missing data in first reception (%d bytes)\n", rcv_size);
        return -201;
    }
    // Protocol
    // First byte is always 0xFF
    // Second byte define packet meaning:
    // 0x01 - Acquisition start (not valid in reception)
    // 0x02 - Acquisition stop (not valid in reception)
    // 0x03 - Configuration read
    // 0x04 - Configuration write
        // 0x05 - Electrode data transfer
    if (recvData[0] != 0xFF)
    {
        memShift(0);
        printf("Invalid Packet Start\n");
        return -202;
    }

    switch (recvData[1])
    {
    case 0x03:
        expected_size = 18;
        break;
    case 0x04:
        expected_size = 4;
        break;
    case 0x05:
        expected_size = 20488;
        break;
    default:
        memShift(0);
        printf("Invalid Packet Code\n");
        return -203;
    }

    int retries = 10;
    while((retries-- > 0) && (rcv_size < expected_size))
    {
        if ((r = _comm->recv(&recvData[rcv_size], expected_size - rcv_size)) >= 0)
        {
            rcv_size += r;
        }
    }
    if (rcv_size < expected_size)
    {
        return -204;
    }
#ifdef _MODBUS_DEBUG
    printf("\n");
    for(int i=0; i<rcv_size && i <MAX_DISPLAY; i++)
    {
        printf(" %02X", recvData[i]);
        if (i%32 == 31)
            printf("\n");
    }
    printf(" <<< %03d\n", rcv_size);

#endif

    // Verify CRC
#ifndef IGNORE_CRC
    unsigned short calc_crc = crc(recvData, rcv_size-2);
    unsigned short recv_crc = (recvData[rcv_size-2] << 8) + recvData[rcv_size-1];
    if (calc_crc != recv_crc)
    {
        printf("\n\nErro Checksum(%04X != %04X)\n\n", calc_crc, recv_crc);
        memShift(0);
        _comm->flush();
        return -1;
    }
#endif
    switch (recvData[1])
    {
    case 0x03:
        printf("Received config information\n");
        // TODO: Avaiod segmentation fault
        break;
        unsigned int id[8];
        double value[8];
        for(i=0;i<8;i++)    id[i] = i;
        value[0] = recvData[2];
        value[1] = *((short*) &recvData[3]);
        value[2] = recvData[5];
        value[3] = *((short*) &recvData[6]);
        value[4] = *((short*) &recvData[8]);
        value[5] = *((short*) &recvData[10]) * 1000.0;
        value[6] = *((short*) &recvData[12]);
        value[7] = *((short*) &recvData[14]) * 100.0;
        storage.InsertConfig(id, value, 8);
        state = 0x03;
        break;
    case 0x04:
        state = 0x04;
        break;
    case 0x05:
        if (recvData[2] == 0x00)
        {
            // Normal Mode
            storage.InsertData(recvData, expected_size, 16, id, 0x01);

            count = count + (expected_size - 8)/160;

            if (count > 4095)
            {
                count = 0;
                static wxLongLong start = 0;

                if (start == 0)
                {
                    start = wxGetLocalTimeMillis();
                }
                else
                {
                    wxDouble speed = 0, elapsed = 0, frames = 0;

                    elapsed = (wxGetLocalTimeMillis() - start).ToDouble();
                    start = wxGetLocalTimeMillis();
                    speed = (4096.0 * 160.0) / elapsed;
                    frames = (4096.0 * 1000.0/ 32.0) / elapsed;
                    printf("\n%f ms Speed:%5.2f Kb/s %5.2f frames/s\n", elapsed, speed, frames);
                    //wxString resumo;
                    //resumo << elapsed << _("ms ") << id << _(" (speed ")<< speed <<_(" Kb/s ou ");
                    //resumo << frames <<_("frames/s) Execs ") << count;
                    //m_pHandler->Term->Insert(resumo, 0);
                    //printf("%f ms %03d (speed %f Kb/s ou %f frames/s) Execs %d\n", elapsed , id, speed, frames, count);
                }

            }

            state = 0x01;
        }
        else
        {
            // Debug Mode
            storage.InsertData(recvData, expected_size, 16, id, 0x02);
            //printf("ID %04X\n", ((unsigned int)recvData[3])+((unsigned int)recvData[4])*256);
//            if(monitor == NULL)
//            {
//                monitor = new WMonitor(NULL, "SCATIE - Monitor", wxPoint(1,1), wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
//                monitor->Show();
//                printf("Monitor Created\n");
//            }
//            monitor->UpdatePlot(recvData[3], &recvData[6], expected_size-8);
            state = 0x02;
        }

        break;
    }
    if (rcv_size > expected_size)
    {
        memShift(expected_size);
        printf("Shift Expected Size\n");
    }
    else
        rcv_size = 0;
    return state;
}

unsigned short ProtBasicBus::crc(const unsigned char* buf, int size)
{
	unsigned short CRC = 0xFFFF; 	// initialize value
	for(int x=0; x<size; x++)		// number of bytes to process
	{
		CRC ^= buf[x];		// read through data array
		for(int z=0; z<8; z++)
		{
			if(CRC & 0x01)
			{
				CRC = 0x0A001 ^ (CRC >> 1);
			}
			else
			{
				CRC >>= 1;
			}
		}
	}
	return CRC;
}

void ProtBasicBus::memShift(int shift)
{
    if (rcv_size < 2)
        return;
    if (shift == 0)
    {
        for(unsigned int i=1; i<rcv_size-1; i++)
        {
           if ((recvData[i] == 0xFF) && (recvData[i+1] <= 0x05) && (recvData[i+1] >= 0x03))
           {
               shift = i;
               break;
           }
        }
    }
    if(shift == 0)
    {
        rcv_size = 0;
        return;
    }
    char tmp_buf[rcv_size];
    memcpy(tmp_buf, &recvData[shift], rcv_size - shift);
    memcpy(recvData, tmp_buf, rcv_size - shift);
    rcv_size -= shift;
}

/******************************************************************************************
        CONFIGURATION
*******************************************************************************************/
void ProtBasicBus::readConfig(const int*, long*, int size)
{
    if (!size)
    {
        sendData[0] = 0xF1;
        sendData[1] = 0x03;
        sendData[2] = 0x93;
        sendData[3] = 0xB9;
        _comm->send(sendData, 4);
        printf("BasicBus: readConfig()\n");
    }
    else
    {

    }

}

void ProtBasicBus::writeConfig(const int*, long*, int)
{

}

void ProtBasicBus::descConfig(const int* id, string* desc, int qty)
{

    for(int i=0; i<qty; i++)
    {
        switch(id[i])
        {
        case CFG_nElectrode:
            desc[i] = "Number of electrodes";
            break;
        case CFG_tCycle:
            desc[i] = "Number of electrodes";
            break;
        case CFG_Pattern:
            desc[i] = "Number of electrodes";
            break;
        case CFG_nMeasure:
            desc[i] = "Number of electrodes";
            break;
        case CFG_tTimeout:
            desc[i] = "Number of electrodes";
            break;
        case CFG_fSample:
            desc[i] = "Number of electrodes";
            break;
        case CFG_nUnstable:
            desc[i] = "Number of electrodes";
            break;
        case CFG_fExcitation:
            desc[i] = "Number of electrodes";
            break;
        }
    }
}

IStorage *ProtBasicBus::getStorage()
{
    return &storage;
}

