#include "WRun.h"

wxBEGIN_EVENT_TABLE(WRun, wxFrame)
EVT_CLOSE(WRun::OnClose)
EVT_SHOW(WRun::OnShow)
EVT_BUTTON(ID_RunStop, WRun::OnStop)
EVT_TIMER(ID_RunTimer, WRun::OnTimer)
wxEND_EVENT_TABLE()


WRun::WRun(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style)
: wxFrame(frame, wxID_ANY, wxT("SCATIE - Capturing"), pos, wxSize(500,400), style)
{
    parent = frame;

    wxPanel *panel = new wxPanel(this, wxID_ANY);
    this->main = main;
    for(unsigned int i=0; i<32; i++)
    {
        amp[i] = new wxTextCtrl(panel, ID_TextAmp, wxT("---"), wxPoint((i%4)*120+10, (i/4)*30+10), wxSize(100,25));
        amp[i]->SetEditable(false);
    }

    btnStop = new wxButton(panel, ID_RunStop, _("Stop"), wxPoint(440,260), wxSize(50,28));
    txtInfo = new wxStaticText(panel, ID_RunInfo, main->getConnectionInfo(), wxPoint(10,150), wxSize(100,28));
    CreateStatusBar();
	SetStatusText(wxT("Running"));


    src = dataSrc;

    updateTimer.SetOwner(this, ID_RunTimer);

}

WRun::~WRun()
{
    //dtor
}

void WRun::OnStop(wxCommandEvent& event)
{
    main->Stop(event);
}

void WRun::OnClose(wxCloseEvent&)
{
    Close();
}

void WRun::OnTimer(wxTimerEvent &event)
{
    for(unsigned x=0; x<8; x++)
    {
        amp[x*2]->SetValue(wxString::Format(wxT("%5.2f mV"), 0.92115*2000.0/32768.0/1020.0*cabs(src->getData().values[x])));
        amp[x*2+1]->SetValue(wxString::Format(wxT("%5.2f rad"), carg(src->getData().values[x])));

    }
    SetStatusText(wxString::Format(wxT("Reading %05d"), src->getData().id));
}

 void WRun::OnShow(wxShowEvent& event)
{
    if (event.IsShown())
        updateTimer.Start(500);
    else
        updateTimer.Stop();
}
