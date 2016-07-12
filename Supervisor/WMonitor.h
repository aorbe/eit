#ifndef WMONITOR_H
#define WMONITOR_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/app.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#else
#include "wx_pch.h"
#endif
#include "math/mathplot.h"
#include "IStorage.h"
#include "IMain.h"

enum
{
    ID_Plot = 101,
    ID_MonTimer = 102,
    ID_MonStop = 103,
    ID_MonSave = 104
};


class WMonitor : public wxFrame
{
    public:
        WMonitor(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style);
        virtual ~WMonitor();
    protected:
        class WGraph : public mpFX
        {
            public:
            WGraph(int channel) : mpFX(wxString::Format(wxT("CH%02d"), channel), mpALIGN_LEFT)
            {
                for(unsigned int x=0; x<1280; x++)
                    values[x] = 0.0;
                SetContinuity(true);
                m_drawOutsideMargins = false;
                m_pen = wxPen(*wxBLUE, 1, wxSOLID);
                //m_type = mpLAYER_PLOT;
            }
            virtual double GetMinX() { return 1020; }
            virtual double GetMaxX() { return 0; }
            virtual double GetMinY() { return -2.0; }
            virtual double GetMaxY() { return +2.0; }
            virtual double GetY(double x)
            {
                unsigned int pos = x;
                if (pos>=0 && pos<1280)
                {
                    return (double) values[pos]/16384.0;
                }
                else
                    return 0.0;
            }
            int values[1280];
        };
        void OnClose(wxCloseEvent&);
        void OnShow(wxShowEvent&);
        void OnStop(wxCommandEvent& event);
        void OnSave(wxCommandEvent& event);
    private:
        wxTimer updateTimer;
        wxButton *btnStop, *btnSave;
        wxFrame *parent;
        IMain* main;

        float f_s[1020], f_c[1020];
        wxString Demodula(int* sinal, unsigned int num);

        IStorage *src;

        void OnTimer(wxTimerEvent &event);

        std::vector<double> x, y;
        bool update, step;
        WGraph* fx[32];
        mpWindow *plot[32];

        int values[32][1280];

        wxDECLARE_EVENT_TABLE();
};

#endif // WMONITOR_H
