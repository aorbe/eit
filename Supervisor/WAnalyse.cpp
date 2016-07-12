#include "WAnalyse.h"

wxBEGIN_EVENT_TABLE(WMonitor, wxFrame)
EVT_TIMER(ID_MonTimer, WMonitor::OnTimer)
EVT_CLOSE(WMonitor::OnClose)
EVT_SHOW(WMonitor::OnShow)
EVT_BUTTON(ID_MonStop, WMonitor::OnStop)
EVT_BUTTON(ID_MonSave, WMonitor::OnSave)
wxEND_EVENT_TABLE()


WAnalyse::WAnalyse(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style)
: wxFrame(frame, wxID_ANY, wxT("SCATIE - Data Analyse"), pos, wxSize(800,600), style)
{
    parent = frame;
    this->main = main;
    CreateStatusBar();
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    updateTimer.SetOwner(this, ID_MonTimer);


    btnStop = new wxButton(panel, ID_MonStop, _("Stop"), wxPoint(720,520), wxSize(70,28));
    btnSave = new wxButton(panel, ID_MonSave, _("Save"), wxPoint(640,520), wxSize(70,28));

    for(unsigned int n=0; n<1020; n++)
    {
        f_s[n] = sin(0.1*M_PI*n);
        f_c[n] = cos(0.1*M_PI*n);
    }


    src = dataSrc;
}

WMonitor::~WMonitor()
{
    //dtor
}

void WMonitor::OnTimer(wxTimerEvent &event)
{
    SetStatusText(wxString::Format(wxT("Reading %05d"), id));
}


void WMonitor::OnClose(wxCloseEvent&)
{
    Close();
}

void WMonitor::OnStop(wxCommandEvent& event)
{
    main->Stop(event);
}

void WMonitor::OnSave(wxCommandEvent& event)
{
    char filename[22];
    for(unsigned int i =0; i<8; i++)
    {
        sprintf(filename, "RAW%04d%02d%02d%02d%02d%02d_%02d", wxDateTime::Now().GetYear(), wxDateTime::Now().GetMonth()+1, wxDateTime::Now().GetDay(), wxDateTime::Now().GetHour(),
                wxDateTime::Now().GetMinute(), wxDateTime::Now().GetSecond(), i);
        std::ofstream fRaw (filename, std::ofstream::out);
        for(unsigned int x=0; x<1280; x++)
        {
            fRaw << fx[i]->values[x] << "\n";
        }
        fRaw.close();
    }
}


void WMonitor::OnShow(wxShowEvent& event)
{
    if (event.IsShown())
        updateTimer.Start(2000);
    else
        updateTimer.Stop();
}
