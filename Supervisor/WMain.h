/*
 * WMain.h
 *
 *  Created on: 16/03/2014
 *      Author: asantos
 */

#ifndef WMAIN_H_
#define WMAIN_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/app.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#else
#include "wx_pch.h"
#endif

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/listbox.h>
#include <wx/thread.h>

#include "ICommChannel.h"
#include "IMain.h"

#include "ChSerialPort.h"
#include "ChUsbPort.h"
#include "ChSimPort.h"

#include "WMonitor.h"
#include "WRun.h"

enum
{
ID_Hello = 1,
ID_ListPorts = 2,
ID_ChkBoxMode = 3,
ID_Connect = 4,
ID_ChkBoxConn = 5,
ID_ListComm = 6,
ID_Disconn = 7,
ID_Timer = 8,
ID_ChkBoxProto = 9,
ID_Run = 10,
ID_Stop = 11,
ID_Calib = 12,
ID_Analyse = 13,
ID_Config = 14,
ID_tMode = 15,
ID_tProto = 16,
ID_Reconnect = 17
};

wxDECLARE_EVENT(wxEVT_COMM_STOPPED, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_CALL_FRAME, wxCommandEvent);

//DECLARE_EVENT_TYPE(wxEVT_COMM_STOPPED, wxThreadEvent);
//DECLARE_EVENT_TYPE(wxEVT_CALL_FRAME, wxCommandEvent);

class WMain : public wxFrame, public IMain
{
public:
	WMain(const wxString& title, const wxPoint& pos, const wxSize& size, long style);
    wxListBox* Term;
    wxThread* run;

    wxString getConnectionInfo();
    void Stop(wxCommandEvent &event)
    {
        OnStop(event);
    }

protected:

    WMonitor* frmMonitor;
    WRun* frmRun;

private:
    void OnClose(wxCloseEvent&);

	void OnExit(wxCommandEvent& event);

    void OnMode(wxCommandEvent& event);
    void OnSelect(wxCommandEvent& event);

    void OnRun(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnCalibration(wxCommandEvent& event);
    void OnConfig(wxCommandEvent& event);
    void OnAnalyse(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

    void OnConnect(wxCommandEvent& event);
    void OnDisconnect(wxCommandEvent& event);

    void OnThreadStop(wxCommandEvent& event);

    void StartComm();
    void StopComm();

    void UpdatePortList(int type);

    void OnCallFrame(wxCommandEvent& event);


	wxChoice *opMode, *opConnection, *opProtocol;
	wxCheckBox *opReconnect;
	wxStaticText *tMode, *tPort, *tProto;
    wxButton *btnOpen, *btnClose, *btnRun, *btnStop, *btnClear, *btnConfig, *btnAnalyse;

    bool running, started;

    wxTimer retryTimer;

    ICommChannel* dev;

	wxDECLARE_EVENT_TABLE();

};


#endif /* WMAIN_H_ */
