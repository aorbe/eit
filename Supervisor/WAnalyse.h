#ifndef WANALYSE_H
#define WANALYSE_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/app.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#else
#include "wx_pch.h"
#endif
#include "IMain.h"

class WAnalyse : public wxFrame
{
    public:
        WAnalyse(IMain* main, wxFrame* frame, IStorage *dataSrc, const wxPoint& pos, long style);
        virtual ~WAnalyse();
    protected:
        void OnClose(wxCloseEvent&);
        void OnShow(wxShowEvent&);
        void OnStop(wxCommandEvent& event);
        void OnSave(wxCommandEvent& event);
    private:
        wxTimer updateTimer;
        wxButton *btnStop, *btnSave;
        wxFrame *parent;
        IMain* main;

        wxDECLARE_EVENT_TABLE();
};


#endif // WANALYSE_H
