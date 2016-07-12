/*
 * Modbus.cpp
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#include "ProtModbus.h"


ProtModbus::ProtModbus(ICommChannel *comm) : IProtocol::IProtocol(comm) {
	// TODO Auto-generated constructor stub
	sendData[0] = 0x01;
	sendData[1] = 0x03;
	sendData[2] = 0x00;
	sendData[3] = 0x00;
	sendData[4] = 0x00;
	sendData[5] = 0x10;
	unsigned short calc = crc(sendData, 6);
	sendData[6] = calc >> 8;
	sendData[7] = calc & 0xff;
}

ProtModbus::~ProtModbus() {
	// TODO Auto-generated destructor stub
}

int ProtModbus::read(unsigned int *id)
{
	int retries = 3;
	while(--retries)
	{
		if(_comm->send(sendData, 8) != 8)
		{
			printf("\n\nSending Error\n\n");
			_comm->flush();
			continue;
		}

		int r = _comm->recv(recvData, 21);

		if (r <= 0)
		{
			printf("\n\nRecption Error (No Data)\n\n");
			_comm->flush();
			continue;
		}

#ifdef _MODBUS_DEBUG
		printf("%03d <", r);
		for(int i=0; i<r; i++)
		{
			printf(" %02X", recvData[i]);
		}
		printf("\n");
#endif
		// Verify CRC
		unsigned short calc_crc = crc(recvData, r-2);
		unsigned short recv_crc = (recvData[r-2] << 8) + recvData[r-1];
		if (calc_crc != recv_crc)
		{
			printf("\n\nErro Checksum(%04X != %04X)\n\n", calc_crc, recv_crc);
			_comm->flush();
			continue;
		}
		if (r == 21)
		{
//		    Voltage data(7);
//			data.id = (recvData[3] << 8) + recvData[4];
//			printf("Recebido ID %04X\n", data.id);
//			for(int x=0; x<7; x++)
//			{
//				data.x[x] = recvData[5+x*2] * 256 + recvData[6+x*2];
//			}
			return 1;
		}
		else
		{
			printf("\n\nErro na Quantidade de Bytes Recebidos\n\n");
		}
	}
	return 0;
}
void ProtModbus::config()
{


}

unsigned short ProtModbus::crc(const unsigned char* buf, int size)
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

void ProtModbus::readConfig(const int* IDs, long* values, int size)
{

}

void ProtModbus::writeConfig(const int* IDs, long* values, int size)
{

}

void ProtModbus::descConfig(const int* id, string* desc, int qty)
{
}


IStorage *ProtModbus::getStorage()
{
    return NULL;
}
