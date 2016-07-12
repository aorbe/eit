/*
 * WConfig.h
 *
 *  Created on: 29/09/2015
 *      Author: asantos
 */
#ifndef WCONFIG_H
#define WCONFIG_H

#include "WMain.h"

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

#include "Scan.h"
#include "ICommChannel.h"

enum
{
ID_Update = 1,
ID_Cancel = 2,
ID_CfgTimer = 3,
ID_TxtnElectrode    = 30,
ID_TxttCycle        = 31,
ID_TxtPattern       = 32,
ID_TxtnMeasure      = 33,
ID_TxttTimeout      = 34,
ID_TxtfSample       = 35,
ID_TxtnUnstable     = 36,
ID_TxtfExcitation   = 37,
ID_CfgnElectrode    = 50,
ID_CfgtCycle        = 51,
ID_CfgPattern       = 52,
ID_CfgnMeasure      = 53,
ID_CfgtTimeout      = 54,
ID_CfgfSample       = 55,
ID_CfgnUnstable     = 56,
ID_CfgfExcitation   = 57,
};

class WConfig : public wxFrame {
public:
	WConfig(WMain* main, const wxString& title, const wxPoint& pos, const wxSize& size, long style);

protected:

	// virtual ~WMain();
private:
    void OnTimer(wxTimerEvent& event);
    void OnUpdate(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);


    wxStaticText* T_nElectrode;
    wxStaticText* T_tCycle;
    wxStaticText* T_Pattern;
    wxStaticText* T_nMeasure;
    wxStaticText* T_tTimeout;
    wxStaticText* T_fSample;
    wxStaticText* T_nUnstable;
    wxStaticText* T_fExcitation;

    wxTextCtrl* P_nElectrode;
    wxTextCtrl* P_tCycle;
    wxTextCtrl* P_Pattern;
    wxTextCtrl* P_nMeasure;
    wxTextCtrl* P_tTimeout;
    wxTextCtrl* P_fSample;
    wxTextCtrl* P_nUnstable;
    wxTextCtrl* P_fExcitation;

    wxButton *btnUpdate, *btnCancel;
    wxTimer* update;

    Scan* run;

	wxDECLARE_EVENT_TABLE();

};

#endif // WCONFIG_H
