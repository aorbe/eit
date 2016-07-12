#include "WMonitor.h"
#include "Voltage.h"
#include <fstream>
#include <math.h>

wxBEGIN_EVENT_TABLE(WMonitor, wxFrame)
EVT_TIMER(ID_MonTimer, WMonitor::OnTimer)
EVT_CLOSE(WMonitor::OnClose)
EVT_SHOW(WMonitor::OnShow)
EVT_BUTTON(ID_MonStop, WMonitor::OnStop)
EVT_BUTTON(ID_MonSave, WMonitor::OnSave)
wxEND_EVENT_TABLE()


WMonitor::WMonitor(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style)
: wxFrame(frame, wxID_ANY, wxT("SCATIE - Calibration"), pos, wxSize(800,600), style)
{
    parent = frame;
    this->main = main;
    CreateStatusBar();
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    for(unsigned int i=0; i<8; i++)
    {
        fx[i] = new WGraph(i+1);

        plot[i] = new mpWindow(panel, ID_Plot, wxPoint(400*(i%2),128*((int)i/2)), wxSize(395,125), wxBORDER_SIMPLE);
        plot[i]->AddLayer(fx[i], true);

        plot[i]->AddLayer(new mpScaleY(wxT("V"), mpALIGN_BORDER_LEFT));
        plot[i]->Fit();
    }
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
    short values[32][1280];
    //Voltage measures = src->getData();
    unsigned int id;
    id = src->getData(values);
    printf("Reading Completed %04X\n", id);

    for(unsigned int i=0; i<8; i++)
    {
        for(unsigned int x=0; x<1280; x++)
        {
            fx[i]->values[x] = values[i][x];
        }
        fx[i]->SetName(Demodula(fx[i]->values, i));
        plot[i]->UpdateAll();
    }
    SetStatusText(wxString::Format(wxT("Reading %05d"), id));
    update = true;
}


wxString WMonitor::Demodula(int* sinal, unsigned int num)
{
    float x =0, y=0;
    for(unsigned int n  = 0; n<1020; n++)
    {
        x = x + (f_s[n] * sinal[n]);
        y = y + (f_c[n] * sinal[n]);
    }
    x = x / 1020 * 2 / 32.768;
    y = y / 1020 * 2 / 32.768;
    float amplitude = sqrt((x*x) + (y*y));
    float fase = atan2(y,x);
    return wxString::Format(wxT("Ch%02d (%5.2fmV %3.0f)"), num, amplitude, fase*180.0/M_PI);
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
