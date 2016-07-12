#ifndef WRUN_H
#define WRUN_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/app.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#else
#include "wx_pch.h"
#endif

#include "IMain.h"
#include "IStorage.h"

enum
{
    ID_TextAmp = 201,
    ID_RunStop = 202,
    ID_RunInfo = 203,
    ID_RunTimer = 204
};


class WRun : public wxFrame
{
    public:
        WRun(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style);
        virtual ~WRun();
        void OnStop(wxCommandEvent& event);

    protected:
        void OnClose(wxCloseEvent&);
        void OnTimer(wxTimerEvent &event);
        void OnShow(wxShowEvent& event);
    private:

        wxFrame* parent;
        wxTextCtrl *amp[32];
        wxStaticText *txtInfo;
        wxButton *btnStop;
        IMain *main;

        IStorage *src;

        wxTimer updateTimer;


        DECLARE_EVENT_TABLE();
};

#endif // WRUN_H
