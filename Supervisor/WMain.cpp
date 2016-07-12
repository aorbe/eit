/*
 * WMain.cpp
 *
 *  Created on: 16/03/2014
 *      Author: asantos
 */
#include "WMain.h"
#include "WConfig.h"
#include "WMonitor.h"

//wxDECLARE_EVENT(wxEVT_COMM_STOPPED, wxThreadEvent);
//wxDECLARE_EVENT(wxEVT_CALL_FRAME, wxCommandEvent);

wxBEGIN_EVENT_TABLE(WMain, wxFrame)
EVT_CLOSE(WMain::OnClose)
EVT_CHOICE(ID_ChkBoxMode, WMain::OnMode)
EVT_CHOICE(ID_ChkBoxConn, WMain::OnSelect)
EVT_BUTTON(ID_Connect, WMain::OnConnect)
EVT_BUTTON(ID_Disconn, WMain::OnDisconnect)
EVT_BUTTON(ID_Run, WMain::OnRun)
EVT_BUTTON(ID_Stop, WMain::OnStop)
EVT_BUTTON(ID_Calib, WMain::OnCalibration)
EVT_BUTTON(ID_Config, WMain::OnConfig)
EVT_BUTTON(ID_Analyse, WMain::OnAnalyse)
EVT_COMMAND(wxID_ANY, wxEVT_COMM_STOPPED, WMain::OnThreadStop)
EVT_COMMAND(wxID_ANY, wxEVT_CALL_FRAME, WMain::OnCallFrame)
EVT_TIMER(ID_Timer, WMain::OnTimer)
wxEND_EVENT_TABLE()



WMain::WMain(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
	CreateStatusBar();
	SetStatusText( "SUPERVISOR" );

    wxPanel *panel = new wxPanel(this, wxID_ANY);

    tMode = new wxStaticText(panel, ID_tMode, _("Connection"), wxPoint(10,6), wxSize(100, 28));
    tPort = new wxStaticText(panel, ID_tProto, _("Port"), wxPoint(10,36), wxSize(110, 28));
    tProto = new wxStaticText(panel, ID_tProto, _("Protocol"), wxPoint(10,66), wxSize(100, 28));

    btnOpen = new wxButton(panel, ID_Connect, _("Conectar"), wxPoint(220,1), wxSize(100,28));
    btnClose = new wxButton(panel, ID_Disconn, _("Desconectar"), wxPoint(330,1), wxSize(100,28));
    btnRun = new wxButton(panel, ID_Run, _("Run"), wxPoint(220,91), wxSize(50,28));
    btnStop = new wxButton(panel, ID_Stop, _("Stop"), wxPoint(270,91), wxSize(50,28));
    btnClear = new wxButton(panel, ID_Calib, _("Calib"), wxPoint(320,91), wxSize(50,28));
    btnConfig = new wxButton(panel, ID_Config, _("Config"), wxPoint(10,91), wxSize(70,28));
    btnAnalyse = new wxButton(panel, ID_Analyse, _("Analyse"), wxPoint(370,91), wxSize(60,28));

    opConnection = new wxChoice(panel, ID_ChkBoxConn, wxPoint(100,31), wxSize(330,28));

    opProtocol = new wxChoice(panel, ID_ChkBoxProto, wxPoint(100,61), wxSize(330,28));
    opProtocol->Append(_("BasicBus V.01"));
    opProtocol->Append(_("Modbus"));
    opProtocol->SetSelection(0);

    wxString commModes[2] = {_("Serial"), _("USB")};
    opMode = new wxChoice(panel, ID_ChkBoxMode, wxPoint(100,1), wxSize(100,28), 2, commModes);
    opMode->SetSelection(1);

    Term = new wxListBox(panel, ID_ListComm, wxPoint(6,131), wxSize(420,150));

    retryTimer.SetOwner(this, ID_Timer);

    // Initial States
    btnOpen->Disable();
    btnClose->Disable();
    btnRun->Disable();
    btnStop->Disable();
    btnClear->Disable();
    btnConfig->Disable();
    opConnection->Enable();

    dev = NULL;
    run = NULL;
    //monitor = NULL;

    running = false;
    started = false;
    UpdatePortList(1);  // Type 1 is USB

    frmMonitor = NULL;
    frmRun = NULL;

}

void WMain::OnClose(wxCloseEvent&)
{
    wxMessageOutputDebug().Printf("MYFRAME: deleting thread");
    started = false;
    StopComm();
    while (running)
    {
        wxYield();
    }
    delete run;
    Destroy();
}

// List available ports on system
void WMain::UpdatePortList(int type)
{
    char** portList = new char*[255];
    int q = 0;
    for(int i=0; i<255; i++)    portList[i] = new char[255];
    switch(type)
    {
    case 0: // Serial
        q = ChSerialPort::listPorts(portList, 255);
        opConnection->Enable();
        opConnection->Clear();
        for(int i=0; i<q; i++)      opConnection->Append(wxString::FromUTF8(portList[i]));
        for(int i=0; i<255; i++)    delete portList[i];
        delete portList;
        if(dev)
            delete dev;
        dev = new ChSerialPort();
        btnClose->Disable();
        break;

    case 1: // USB
        q = ChUsbPort::listPorts(portList, 255);
        opConnection->Enable();
        opConnection->Clear();
        for(int i=0; i<q; i++)      opConnection->Append(wxString::FromUTF8(portList[i]));
        for(int i=0; i<255; i++)    delete portList[i];
        delete portList;
        if(dev)
        {
            delete dev;
        }
        dev = new ChUsbPort();
        btnClose->Disable();
        break;

    case 2:
        dev = new ChSimPort();
        btnClose->Disable();
        break;
    }
}

void WMain::OnThreadStop(wxCommandEvent& event)
{
    printf("SCAN finish\n");
    running = false;
    if (started)
    {
        retryTimer.StartOnce(5000);
    }
}

void WMain::StartComm()
{
    if (started)
    {
        if (!running)
        {
            running = true;
            if (run != NULL)
            {
                delete run;
            }
            run = new Scan(this, dev, opConnection->GetStringSelection(), opProtocol->GetSelection());
            run->Run();
        }
    }
}

void WMain::StopComm()
{
    if (running)
    {
        run->Delete();
    }
}

void WMain::OnTimer(wxTimerEvent &event)
{
    printf("Retrying...");
    StartComm();
}

wxString WMain::getConnectionInfo()
{
    if (running)
    {
        //return wxString::Format("%s %s", )
    }
    return wxT("");

}

/******************************************************************************************
        CHOICE COMMANDS
*******************************************************************************************/
void WMain::OnMode(wxCommandEvent& event)
{
    UpdatePortList(event.GetSelection());
}

void WMain::OnSelect(wxCommandEvent& event)
{
    if(event.GetSelection() >= 0)
    {
        btnOpen->Enable();
        //btnClose->Enable();
    }
    else
    {
        btnOpen->Disable();
        btnClose->Disable();
    }
}

/******************************************************************************************
        BUTTONS COMMANDS
*******************************************************************************************/
void WMain::OnDisconnect(wxCommandEvent &event)
{
    started = false;
    if (running)
    {
        run->Delete();
    }
    // Screen formatting
    opConnection->Enable();
    opMode->Enable();
    opProtocol->Enable();
    btnRun->Disable();
    btnStop->Disable();
    btnClose->Disable();
    btnOpen->Enable();
    //StopComm();
}

void WMain::OnConnect(wxCommandEvent &event)
{
    started = true;
    opConnection->Disable();
    opMode->Disable();
    opProtocol->Disable();
    btnClose->Enable();
    btnOpen->Disable();
    btnRun->Enable();
    btnStop->Enable();
    btnClear->Enable();
    StartComm();
}

void WMain::OnRun(wxCommandEvent &event)
{
    if (running)
    {
        ((Scan*) run)->Send(1);

        Term->Insert(_("Start Cycle"),0);
    }
}

void WMain::OnStop(wxCommandEvent &event)
{
    if (running)
    {
        ((Scan*) run)->Send(2);
        Term->Insert(_("Stop Cycle"),0);
    }
}

void WMain::OnCalibration(wxCommandEvent &event)
{
    if (running)
    {
        ((Scan*) run)->Send(4);
        printf("Command Sent\n");
        Term->Insert(_("Start Calibration Cycle"),0);
    }
}

void WMain::OnConfig(wxCommandEvent &event)
{
    WConfig *config = new WConfig(this, "SCATIE - Parâmetros", wxPoint(1,1), wxSize(340,380), wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
    config->Show();
}

void WMain::OnCallFrame(wxCommandEvent &event)
{
    if (event.GetString().IsSameAs(wxT("RUN")))
    {
        if (frmRun == NULL)
        {
            frmRun =  new WRun(this, this, (IStorage*) event.GetClientData(), wxPoint(0,0), wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
        }
        if (!frmRun->IsVisible())
        {
            frmRun->Show();
            if (IsVisible())
                Hide();
            if ((frmMonitor != NULL) && (frmMonitor->IsVisible()))
                frmMonitor->Hide();
        }
    }
    if (event.GetString().IsSameAs(wxT("MONITOR")))
    {

        if (frmMonitor == NULL)
        {
            frmMonitor =  new WMonitor(this, this, (IStorage*) event.GetClientData(), wxPoint(0,0), wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);
        }
        if (!frmMonitor->IsVisible())
        {
            frmMonitor->Show();
            if (IsVisible())
                Hide();
            if ((frmRun != NULL) && (frmRun->IsVisible()))
                frmRun->Hide();
        }
    }
    if (event.GetString().IsSameAs(wxT("MAIN")))
    {

        if (!IsVisible())
            Show();
        if((frmRun != NULL) && (frmRun->IsVisible()))
        {
            frmRun->Hide();
        }
        if((frmMonitor != NULL) && (frmMonitor->IsVisible()))
        {
            frmMonitor->Hide();
        }

    }
}

void WMain::OnAnalyse(wxCommandEvent &event)
{

}

