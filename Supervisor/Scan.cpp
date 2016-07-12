#include "Scan.h"
#include "IProtocol.h"
#include "ProtBasicBus.h"

wxDEFINE_EVENT(wxEVT_CALL_FRAME, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_COMM_STOPPED, wxThreadEvent);

Scan::Scan(WMain *handler, ICommChannel *channel, wxString port, int type) : wxThread(wxTHREAD_JOINABLE)
{
    m_pHandler = handler;
    Set(channel, port, type);
}

Scan::~Scan()
{
    printf("Destroying Scan Main Loop\n");
}

void Scan::Set(ICommChannel *channel, wxString port, int type)
{
    this->channel = channel;
    this->port = new wxString(port);
    this->type = type;
}

void Scan::Send(int command)
{
    this->command = command;
}

void Scan::SetValue(long value)
{
    this->value = value;
}


/*
 * OPEN PORT
 * ATTACH TO DEFINED PROTOCOL
 * START MAIN THREAD LOOP
 * MAIN LOOP:
 *  VERIFY WETHER CHANNEL IS OPEN
 *  SEND ANY PENDING COMMAND
 *  SCAN DEVICE (READ)
*/
Scan::ExitCode Scan::Entry()
{
    int ret_value = 0;
    unsigned int id = 0xFFFF, type = 0;
    time_t last_success = 0;

    // Connect port and
    printf("Verifying channel %s\n", (const char*)port->ToAscii());
    if(!channel->open(port->ToAscii()))
    {
        printf("Channel open error\n");
    }

    // Protocol Creating
    switch(type)
    {
    case 0: // Basic Bus V.01
        proto = new ProtBasicBus(channel);
        break;
//    case 1:
//        proto = new Sniffer(new StoreData(), channel);
//        break;
    default:
        break;
    }

    printf("Entering Scan Main Loop\n");
    while(!TestDestroy())
    {

        if (!channel->isOpen())
        {
            printf("Channel Closed\n");
            //error = 1;
            break;
        }

        if (command)
        {
            switch(command)
            {
                case 1:
                    proto->enable(0);   // RUN
                    break;
                case 2:
                    proto->disable();
                    break;
//                case 3:
//                    proto->setScanTime(value);
//                    proto->config();
//                    break;
                case 4:
                    proto->enable(1);   // CALIBRATION
                    break;
                case 5:
                    proto->readConfig(NULL, NULL, 0);
                    break;
                case 6:
                    //proto->write()
                    break;
            }
            command = 0;
        }

        //wxLongLong StartTime = wxGetUTCTimeUSec();
        if( 0 < (ret_value = proto->read(&id)))
        {
            static int active_mode = 0;
            last_success = time(NULL);
            if ((ret_value == 2) && (active_mode != 2))
            {
                active_mode = 2;
                printf("Creating Monitor Frame\n");
                frmEvent = new wxCommandEvent(wxEVT_CALL_FRAME, GetId());
                frmEvent->SetString(wxT("MONITOR"));
                frmEvent->SetClientData(proto->getStorage());
                wxQueueEvent(m_pHandler, frmEvent);
            }
            if ((ret_value == 1) && (active_mode != 1))
            {
                active_mode = 1;
                printf("Creating Run Frame\n");
                frmEvent = new wxCommandEvent(wxEVT_CALL_FRAME, GetId());
                frmEvent->SetString(wxT("RUN"));
                frmEvent->SetClientData(proto->getStorage());
                wxQueueEvent(m_pHandler, frmEvent);
            }
            if ((ret_value != 1) && (ret_value != 2) && (ret_value > -200))
            {
                active_mode = 0;
            }
        }

        if (last_success < time(NULL) - 5 )
        {
            frmEvent = new wxCommandEvent(wxEVT_CALL_FRAME, GetId());
            frmEvent->SetString(wxT("MAIN"));
            frmEvent->SetClientData(proto->getStorage());
            wxQueueEvent(m_pHandler, frmEvent);
        }


        // Connection lost - By now give up
        if (ret_value < 0)
        {
            printf("SCAN Error %d\n", ret_value);
            this->Sleep(0);
            if (ret_value == -2)
            {
                this->Sleep(10);
                printf("Error -2\n");
                break;
            }
            if (ret_value == -4)
            {
                break;
            }
        }
    }
    //m_pHandler->FileData->SaveData();
    channel->close();
    // Destroy protocol
    if(proto)
    {
        delete proto;
    }

    wxQueueEvent(m_pHandler, new wxThreadEvent(wxEVT_COMM_STOPPED));
    printf("Leaving Scan Main Loop\n");
    return (wxThread::ExitCode) 0;
}
