/*
 * UsbPort.c
 *
 *  Created on: 14/12/2013
 *      Author: asantos
 */

#include "ChUsbPort.h"


bool ChUsbPort::opened = false;
bool ChUsbPort::transfering = false;
unsigned long ChUsbPort::local_rcvptr = 0;
libusb_device_handle *ChUsbPort::hPort = 0;
unsigned char ChUsbPort::localbuf[65536];
unsigned char ChUsbPort::local_rcvbuf[USB_BUFFER_SIZE];
pthread_mutex_t ChUsbPort::run_mutex;


ChUsbPort::ChUsbPort() {
	opened = false;
	hPort = NULL;
	local_rcvptr = 0;
	transfer = (libusb_transfer *)malloc(sizeof(libusb_transfer) * 100);
#ifdef _WINDOWS

#else

#endif
	strcpy(portname, "");
}

ChUsbPort::~ChUsbPort() {
	if(opened)
		close();
    free(transfer);
}

int ChUsbPort::open(const char* name)
{
	opened = false;
	uint32_t idVendor, idProduct;
	int r;

	strcpy(portname, name);

	if(2 != sscanf(portname, "%*s %04X:%04X", &idVendor, &idProduct))
	{
	    printf("Device Invalid Name\n");
	    return opened;
	}

#ifdef _WINDOWS


#else

    if (0 != libusb_init(&context))
    {
        printf("UsbOpen: Context Error\n");
        return 0;
    }
	printf("Openning Vendor %04X Product %04X\n", idVendor, idProduct);
    if ((hPort = libusb_open_device_with_vid_pid (context, idVendor, idProduct)) == NULL)
    {
        close();
        printf("UsbOpen: Read Open Error\n");
        return 0;
    }

    opened = true;

    if (libusb_set_auto_detach_kernel_driver(hPort, true) != LIBUSB_SUCCESS)
    {
        printf("Usb Driver Auto Detach Error\n");
    }

    if (libusb_kernel_driver_active(hPort, INTERFACE_NUMBER) == 1)
    {
        printf("Read Interface: Kernel Driver Active\n");
        int r = libusb_detach_kernel_driver(hPort, INTERFACE_NUMBER);
        printf("Read Interface: Return Detach Kernel %d\n", r);
        if (0 != r)
        {
            close();
            printf("Read UsbOpen: Detach Kernel Error\n");
            return opened;
        }
    }
    else
        printf("Main Interface: Kernel Driver Already Inactive\n");

    r = libusb_attach_kernel_driver(hPort, INTERFACE_NUMBER);
    if (0 != r)
    {
        printf("UsbOpen Main Interface Claim Error: %s\n", libusb_errmsg(r));
    }

    //if (int r = libusb_set_configuration(hPort, -1) !=0)
    //{
    //    printf("UsbOpen Set Configuration Error: %s\n", libusb_errmsg(r));
    //}
    r = libusb_claim_interface(hPort, INTERFACE_NUMBER);
    if (0 != r)
    {
        printf("UsbOpen Main Interface Claim Error: %s\n", libusb_errmsg(r));
    }


    if(INTERFACE_ALT_NUMBER != INTERFACE_NUMBER)
    {
        r = libusb_claim_interface(hPort, INTERFACE_ALT_NUMBER);
        if (0 != r)
        {
            printf("UsbOpen Alt Interface Claim Error: %s\n", libusb_errmsg(r));
        }
    }
    // Create reception thread
    pthread_mutex_init(&run_mutex, NULL);
    pthread_create(&sThread, NULL, run, this);


#endif
    return opened;
}

int ChUsbPort::configure(const char **params, const char **values)
{
	if(!opened)	return false;
	//int i = 0;
#ifdef _WINDOWS

#else

#endif
    return opened;
}

int ChUsbPort::cfg_default()
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
	sprintf(cfgvalue[0], "19200");
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

int ChUsbPort::isOpen()
{
	return opened;
}

int ChUsbPort::send(const unsigned char *buf, int size)
{

	if(!opened)	return -1;
	int n = 0;
	if(size > 1023)
        return -1;
    memcpy(localbuf, buf, size);
#ifdef _WINDOWS

#else
    int r = libusb_bulk_transfer(hPort, WRITE_ENDPOINT_NUMBER, localbuf, size, &n, 1000);
    if(0 != r)
    {
        printf("UsbPort Send: %s\n", libusb_errmsg(r));
        return -1;
    }
#endif
	if(n!=size)
		printf("Error %d\n", 0);
	return n;
}

//typedef void( * libusb_transfer_cb_fn)(struct libusb_transfer *transfer)
void ChUsbPort::recv_CallBack(struct libusb_transfer *transfer)
{
    printf("USB: Something received\n");
    local_rcvptr = local_rcvptr + transfer->actual_length;
    libusb_free_transfer(transfer);
    transfering = false;
}

int ChUsbPort::recv(unsigned char *buf, unsigned int max_size)
{
	if(!opened)
	{
	    printf("Recv Error: Interface not opened\n");
	    return -2;
	}

	pthread_yield();
	//int n = 0;
	if(max_size > local_rcvptr)
    {
        max_size = local_rcvptr;
        //return -1;
    }

	pthread_mutex_lock(&run_mutex);

    memcpy(buf, local_rcvbuf, max_size);

    for(unsigned long i=max_size; i<local_rcvptr; i++)
    {
        local_rcvbuf[i-max_size] = local_rcvbuf[i];
    }
    local_rcvptr = local_rcvptr - max_size;
    //printf("-Size %d\n", local_rcvptr);
    pthread_mutex_unlock(&run_mutex);
    return(max_size);
}

void ChUsbPort::flush()
{
#ifdef _WINDOWS

#else

#endif
}

void ChUsbPort::close()
{
    if (!opened)
        return;
    opened = false;
#ifdef _WINDOWS

#else

    if(NULL != hPort)
    {
        pthread_join(sThread, NULL);
        libusb_release_interface(hPort, INTERFACE_NUMBER);
        libusb_attach_kernel_driver(hPort, INTERFACE_NUMBER);
        libusb_release_interface(hPort, INTERFACE_ALT_NUMBER);
        libusb_attach_kernel_driver(hPort, INTERFACE_ALT_NUMBER);
        libusb_close(hPort);
        hPort = NULL;
    }
    libusb_exit(context);

#endif

}

int ChUsbPort::listPorts(char** buffer, unsigned int max_size)
{
    int size = 0;
    libusb_context* context;
    libusb_device** list;
    if (0 != libusb_init(&context))
    {
        printf("ListPorts: Context Error\n");
        return 0;
    }

    int qty = libusb_get_device_list(context, &list);
    libusb_device_descriptor desc;
    libusb_device_handle *dev;
    unsigned char st_desc[255];
    printf("Found %d devices\n", qty);
    for(int i=0; i<qty; i++)
    {
        if(0 == libusb_get_device_descriptor(list[i], &desc))
        {
            if (0 == libusb_open(list[i], &dev))
            {
                libusb_get_string_descriptor_ascii(dev, 1, st_desc, 255);
                switch(desc.bDeviceClass)
                {
                    case LIBUSB_CLASS_PER_INTERFACE:
                        sprintf(buffer[size++], "Interf %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                        break;
                    case LIBUSB_CLASS_AUDIO:
                        sprintf(buffer[size++], "Audio %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    case LIBUSB_CLASS_COMM:
                        sprintf(buffer[size++], "Comm %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    case LIBUSB_CLASS_HID:
                        sprintf(buffer[size++], "HID %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    case LIBUSB_CLASS_PRINTER:
                        sprintf(buffer[size++], "Printer %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
                        sprintf(buffer[size++], "HealthCare %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    case LIBUSB_CLASS_VENDOR_SPEC:
                        sprintf(buffer[size++], "VendorSpec %04X:%04X %s", desc.idVendor, desc.idProduct, st_desc);
                            break;
                    default:
                        break;
                }
                libusb_close(dev);
            }
            else
                printf("Device Open Error\n");
        }


    }
    libusb_free_device_list(list, 0);

    libusb_exit(context);
    return size;
}

const char* ChUsbPort::libusb_errmsg(int err)
{
    switch(err)
    {
        case LIBUSB_SUCCESS:
            return("Success");
        case LIBUSB_ERROR_IO:
            return("IO Error");
        case LIBUSB_ERROR_INVALID_PARAM:
            return("Invalid Parameter");
        case LIBUSB_ERROR_ACCESS:
            return("Error Access");
        case LIBUSB_ERROR_NO_DEVICE:
            return("No Device");
        case LIBUSB_ERROR_NOT_FOUND:
            return("Nof Found");
        case LIBUSB_ERROR_BUSY:
            return("Busy");
        case LIBUSB_ERROR_TIMEOUT:
            return("Timeout");
        case LIBUSB_ERROR_OVERFLOW:
            return("Overflow");
        case LIBUSB_ERROR_PIPE:
            return("Error Pipe");
        case LIBUSB_ERROR_INTERRUPTED:
            return("Interrupted");
        case LIBUSB_ERROR_NO_MEM:
            return("No Memory");
        case LIBUSB_ERROR_NOT_SUPPORTED:
            return("Not Supported");
        case LIBUSB_ERROR_OTHER:
            return("Other");
    }
    return "Not Registered";
}

int ChUsbPort::hasData()
{
    return local_rcvptr;
}

void *ChUsbPort::run(void* arg)
{
    int max;
    int r = 0;
    int received = 0;
	/* added the void pointer cast, otherwise gcc will complain about */
	/* "warning: dereferencing type-punned pointer will break strict aliasing rules" */
#ifdef _WINDOWS

#else
    while(opened)
    {
        pthread_yield();

        max = USB_BUFFER_SIZE - local_rcvptr - 1;
        if (max >= 2568)
        {
            usleep(10000);
            pthread_mutex_lock(&run_mutex);
            r = libusb_bulk_transfer(hPort, READ_ENDPOINT_NUMBER, &local_rcvbuf[local_rcvptr], max, &received, 150);
            if ((r == 0) || ((r == -7) && (received > 0)))
            {
                local_rcvptr += received;
            }
            pthread_mutex_unlock(&run_mutex);
            if ((r < 0) && (r != -7 /*TIMEOUT*/))
            {
//                printf("+Size %d (%d)\n", local_rcvptr, r);
                ((ChUsbPort*)arg)->close();
                return 0;
            }
        }


    }
#endif
    return 0;
}
