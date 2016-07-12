/*
 * SerialPort.cpp
 *
 *  Created on: 11/10/2013
 *      Author: asantos
 */

#include "ChSerialPort.h"


ChSerialPort::ChSerialPort() : ICommChannel::ICommChannel() {
	hPort = INVALID_HANDLE_VALUE;

#ifdef _WINDOWS
	memset(&port_settings, 0x00, sizeof(DCB));
	port_settings.DCBlength = sizeof(DCB);
	Cptimeouts.ReadIntervalTimeout         = 300;
	Cptimeouts.ReadTotalTimeoutMultiplier  = 20;
	Cptimeouts.ReadTotalTimeoutConstant    = 200;
	Cptimeouts.WriteTotalTimeoutMultiplier = 0;
	Cptimeouts.WriteTotalTimeoutConstant   = 0;
#else

#endif
	strcpy(portname, "");
}

ChSerialPort::~ChSerialPort() {
	close();
}


int ChSerialPort::open(const char* name)
{
    if(hPort != INVALID_HANDLE_VALUE)
    {
        opened = true;
        return opened;
    }
	opened = false;
#ifdef _WINDOWS
	hPort = CreateFileA(name,
			GENERIC_READ|GENERIC_WRITE,
            0,                          /* no share  */
            NULL,                       /* no security */
            OPEN_EXISTING,
            0,                          /* no threads */
            NULL);                      /* no templates */
#else
    printf("Opening %s\n", name);
	hPort = ::open(name, O_RDWR | O_NOCTTY);
#endif

	if (hPort == INVALID_HANDLE_VALUE) {
		printf("Port open error\n");
	    return GetLastError();
	}
	strcpy(portname, name);

#ifdef _WINDOWS
	if(!GetCommState(hPort, &port_settings))
	{
		CloseHandle(hPort);
		printf("Erro GetCommState %d\"\n", (int)GetLastError());
		return opened;
	}

	port_settings.BaudRate = DEFAULT_BAUDRATE;

	if(!SetCommState(hPort, &port_settings))
	{
		CloseHandle(hPort);
		printf("Erro portsettings\n");
		return opened;
	}
	if(!SetCommTimeouts(hPort, &Cptimeouts))
	{
		printf("Erro SetCommTimeouts\n");
		CloseHandle(hPort);
		return opened;
	}
#else
    if (tcgetattr(hPort, &port_settings) != 0)
    {
		close();
		printf("Erro GetCommState %d\"\n", (int)GetLastError());
		return opened;
    }


#endif
    opened = true;
    if (cfg_default() == false)
    {
		printf("Default Configuration Error (%d)\"\n", (int)GetLastError());
		close();
		opened = false;
		return opened;
    }
    printf("Port Openned.\n");
	return opened;
}

int ChSerialPort::configure(const char *params[], const char *values[])
{
	if(!opened)	return false;
	int i = 0;
#ifdef _WINDOWS
	while(strcmp(params[i], ""))
	{
		if(!strcmp(params[i], "BAUDRATE"))
		{
			port_settings.BaudRate = atoi(values[i]);
			printf("Baudrate = %d\n", (int)port_settings.BaudRate);
		}
		if(!strcmp(params[i], "DATABITS"))
		{
			port_settings.ByteSize = atoi(values[i]);
			printf("Data Bits = %d\n", port_settings.ByteSize);
		}
		if(!strcmp(params[i], "STOPBITS"))
		{
			if(!strcmp(values[i], "1"))
			{
				port_settings.StopBits = ONESTOPBIT;
				printf("Stop Bits = 1\n");
			}
			if(!strcmp(values[i], "1.5"))
			{
				port_settings.StopBits = ONE5STOPBITS;
			}
			if(!strcmp(values[i], "2"))
			{
				port_settings.StopBits = TWOSTOPBITS;
			}
		}
		if(!strcmp(params[i], "PARITY"))
		{
			if(!strcmp(values[i], "NONE"))
			{
				port_settings.Parity = NOPARITY;
				printf("Parity = NONE\n");
			}
			if(!strcmp(values[i], "EVEN"))
			{
				port_settings.Parity = EVENPARITY;
			}
			if(!strcmp(values[i], "ODD"))
			{
				port_settings.Parity = ODDPARITY;
			}
			if(!strcmp(values[i], "MARK"))
			{
				port_settings.Parity = MARKPARITY;
			}
			if(!strcmp(values[i], "SPACE"))
			{
				port_settings.Parity = SPACEPARITY;
			}
		}
		i++;
	}
	if(!SetCommState(hPort, &port_settings))
	{
		opened = false;
		CloseHandle(hPort);
		return opened;
	}
	/*
	 *
	 if(!SetCommTimeouts(hPort, &Cptimeouts))
	{
		opened = false;
		CloseHandle(hPort);
		return opened;
	}
	*/
#else
	while(strcmp(params[i], ""))
	{
		if(!strcmp(params[i], "BAUDRATE"))
		{
			speed_t speed;
			printf("Baudrate = %d\n",  atoi(values[i]));
			switch (atoi(values[i]))
			{
			case 19200:
				speed = B19200;
				break;
			case 38400:
				speed = B38400;
				break;
			case 57600:
				speed = B57600;
				break;
			case 115200:
				speed = B115200;
				break;
			case 230400:
				speed = B230400;
				break;
			case 460800:
				speed = B460800;
				break;
			case 500000:
				speed = B500000;
				break;
			case 576000:
				speed = B576000;
				break;
			case 921600:
				speed = B921600;
				break;
			case 1000000:
				speed = B1000000;
				break;
			case 1152000:
				speed = B1152000;
				break;
			case 1500000:
				speed = B1500000;
				break;
			case 2000000:
				speed = B2000000;
				break;
			case 2500000:
				speed = B2500000;
				break;
			case 3000000:
				speed = B3000000;
				break;
			case 3500000:
				speed = B3500000;
				break;
			case 4000000:
				speed = B4000000;
				break;
			default:
				printf("\n\nInvalid Baudrate\n\n");
				speed = B19200;
				break;
			}
		    cfsetospeed (&port_settings, speed);
		    cfsetispeed (&port_settings, speed);
		}
		if(!strcmp(params[i], "DATABITS"))
		{
			int bits = atoi(values[i]);
			printf("Data Bits = %d\n", bits);
			switch (bits)
			{
			case 5:
				port_settings.c_cflag = (port_settings.c_cflag & ~CSIZE) | CS5;
				break;
			case 6:
				port_settings.c_cflag = (port_settings.c_cflag & ~CSIZE) | CS6;
				break;
			case 7:
				port_settings.c_cflag = (port_settings.c_cflag & ~CSIZE) | CS7;
				break;
			case 8:
				port_settings.c_cflag = (port_settings.c_cflag & ~CSIZE) | CS8;
				break;
			default:
				printf("\n\nData Bits INVALID\n\n");
			}
		}
		if(!strcmp(params[i], "STOPBITS"))
		{
			if(!strcmp(values[i], "1"))
			{
				port_settings.c_cflag = (port_settings.c_cflag & ~CSTOPB);
				printf("Stop Bits = 1\n");
			}
			if(!strcmp(values[i], "2"))
			{
				port_settings.c_cflag = (port_settings.c_cflag | CSTOPB);
				printf("Stop Bits = 2\n");
			}
		}
		if(!strcmp(params[i], "PARITY"))
		{
			if(!strcmp(values[i], "NONE"))
			{
				port_settings.c_cflag = (port_settings.c_cflag & ~PARENB);
				printf("Parity = NONE\n");
			}
			if(!strcmp(values[i], "EVEN"))
			{
				port_settings.c_cflag = (port_settings.c_cflag & ~PARODD) | PARENB;
				printf("Parity = EVEN\n");
			}
			if(!strcmp(values[i], "ODD"))
			{
				port_settings.c_cflag = (port_settings.c_cflag | PARODD) | PARENB;
				printf("Parity = ODD\n");
			}
		}
		i++;

	}
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    port_settings.c_iflag &= ~IGNBRK;         // ignore break signal
    port_settings.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    port_settings.c_oflag = 0;                // no remapping, no delays
    port_settings.c_cc[VMIN]  = 0;            // read doesn't block
    //port_settings.c_cc[VTIME] = 20;            // 0.5 seconds read timeout

    port_settings.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    port_settings.c_cflag |= (CLOCAL | CREAD);			// ignore modem controls,
 													// enable reading
    port_settings.c_cflag &= ~CRTSCTS;

    if (tcsetattr (hPort, TCSANOW, &port_settings) != 0)
    {
		printf("error %d from tcsetattr", errno);
		opened = false;
		return opened;
    }
#endif
    return opened;
}


int ChSerialPort::cfg_default()
{
	char **cfgname, **cfgvalue, CFG_SIZE=5;
	cfgname = new char*[CFG_SIZE];
	cfgvalue = new char*[CFG_SIZE];
	for (int k=0; k<CFG_SIZE; k++)
	{
		cfgname[k] = new char[12];
		cfgvalue[k] = new char[8];
	}
	sprintf(cfgname[0], "BAUDRATE");
	sprintf(cfgvalue[0], "115200");
	sprintf(cfgname[1], "DATABITS");
	sprintf(cfgvalue[1], "8");
	sprintf(cfgname[2], "STOPBITS");
	sprintf(cfgvalue[2], "1");
	sprintf(cfgname[3], "PARITY");
	sprintf(cfgvalue[3], "NONE");
	cfgname[4][0] = 0x00;
	cfgvalue[4][0] = 0x00;
	int result = configure((const char**)cfgname, (const char**)cfgvalue);
	for(int j=0; j<CFG_SIZE; j++)
	{
		delete cfgname[j];
		delete cfgvalue[j];
	}
	delete cfgname;
	delete cfgvalue;
	return result;

}

int ChSerialPort::isOpen()
{
    if (hPort == INVALID_HANDLE_VALUE)
        return false;
    return true;
}

int ChSerialPort::send(const unsigned char *buf, int size)
{
	if(!opened)	return -1;
	int n;
#ifdef _WINDOWS
	WriteFile(hPort, buf, size, (LPDWORD)((void *)&n), NULL);
#else
	n = write(hPort, buf, size);
#endif
	if(n!=size)
		printf("Error %d\n", (int)GetLastError());
	return n;
}

int ChSerialPort::recv(unsigned char *buf, unsigned int max_size)
{
	if(!opened)	return -1;
	int n = 0;
	if(max_size>4096)  max_size = 4096;
	/* added the void pointer cast, otherwise gcc will complain about */
	/* "warning: dereferencing type-punned pointer will break strict aliasing rules" */
#ifdef _WINDOWS
	ReadFile(hPort, buf, max_size, (LPDWORD)((void *)&n), NULL);
#else
	//int res;
	//struct timeval Timeout;
	//fd_set readfs;
	/* set timeout value within input loop */
	//Timeout.tv_usec = 0;  /* milliseconds */
	//Timeout.tv_sec  = 3;  /* seconds */
	//FD_SET(hPort, &readfs);
	//res = select(1, &readfs, NULL, NULL, &Timeout);

	//printf("Sei La %d\n", res);
	//if (res!=0)
	//{
		/* number of file descriptors with input = 0, timeout occurred. */
		n = read(hPort, buf, max_size);
	//}
#endif
	return(n);

}

void ChSerialPort::flush()
{
#ifdef _WINDOWS
	PurgeComm(hPort, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
#else
	tcflush(hPort, TCIOFLUSH);
#endif
}

void ChSerialPort::close()
{
#ifdef _WINDOWS
	CloseHandle(hPort);
#else
    if(hPort != INVALID_HANDLE_VALUE)
    {
        hPort = INVALID_HANDLE_VALUE;
        ::close(hPort);
        printf("Port Closed\n");
    }
#endif
}

/*
bool SerialPort::isCTSActive()
{
	if(!opened)	return -1;
	int status;

	GetCommModemStatus(hPort, (LPDWORD)((void *)&status));
	if(status & MS_CTS_ON)
		return true;
	else
		return false;
}

bool SerialPort::isDSRActive()
{
	if(!opened)	return -1;
	int status;
	GetCommModemStatus(hPort, (LPDWORD)((void *)&status));
	if(status & MS_DSR_ON)
		return(true);
	else
		return(false);
}

void SerialPort::setDTR(BOOL state)
{
	if(!opened)	return;
	if (state)
		EscapeCommFunction(hPort, SETDTR);
	else
		EscapeCommFunction(hPort, CLRDTR);
}

void SerialPort::setRTS(BOOL state)
{
	if(!opened)	return;
	if(state)
		EscapeCommFunction(hPort, SETRTS);
	else
		EscapeCommFunction(hPort, CLRRTS);
}
*/
int ChSerialPort::listPorts(char** buffer, unsigned int max_size)
{
    DIR           *d;
    struct dirent *dir;
    struct stat   st;
    unsigned int i = 0;
    printf("Buscando Portas\n");
    d = opendir("/dev/");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if((strlen(dir->d_name) > 4) && (0 == strncmp(dir->d_name, "ttyS", 4)) && stat(dir->d_name, &st))
            {
                sprintf(buffer[i++], "/dev/%s", dir->d_name);
                printf("Adding /dev/%s\n", dir->d_name);
            }
            if((strlen(dir->d_name) > 6) && (0 == strncmp(dir->d_name, "ttyACM", 6)) && stat(dir->d_name, &st))
            {
                sprintf(buffer[i++], "/dev/%s", dir->d_name);
                printf("Adding /dev/%s\n", dir->d_name);
            }
            if((strlen(dir->d_name) > 6) && (0 == strncmp(dir->d_name, "ttyUSB", 6)) && stat(dir->d_name, &st))
            {
                sprintf(buffer[i++], "/dev/%s", dir->d_name);
                printf("Adding /dev/%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
    return i;
}

int ChSerialPort::hasData()
{
    return 0;
}
