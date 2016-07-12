/*
 * WConfig.cpp
 *
 *  Created on: 29/09/2015
 *      Author: asantos
 */
#include "WConfig.h"


wxBEGIN_EVENT_TABLE(WConfig, wxFrame)
EVT_BUTTON(ID_Update, WConfig::OnUpdate)
EVT_BUTTON(ID_Cancel, WConfig::OnCancel)
EVT_TIMER(ID_CfgTimer, WConfig::OnTimer)
wxEND_EVENT_TABLE()


WConfig::WConfig(WMain* main, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
    //this->SetSize(size);
    wxPanel *panel = new wxPanel(this, wxID_ANY);

    T_nElectrode = new wxStaticText(panel, ID_TxtnElectrode, _T("Número de Eletrodos"), wxPoint(10,10), wxSize(220, 28));
    T_tCycle = new wxStaticText(panel, ID_TxttCycle, _T("Tempo de Ciclo (us)"), wxPoint(10,40), wxSize(220, 28));
    T_Pattern = new wxStaticText(panel, ID_TxtPattern, _T("Padrão (Pula)"), wxPoint(10,70), wxSize(220, 28));
    T_nMeasure = new wxStaticText(panel, ID_TxtnMeasure, _T("Número de Medidas"), wxPoint(10,100), wxSize(220, 28));
    T_tTimeout = new wxStaticText(panel, ID_TxttTimeout, _T("Tempo de Timeout (us)"), wxPoint(10,130), wxSize(220, 28));
    T_fSample = new wxStaticText(panel, ID_TxtfSample, _T("Frequência de Amostragem (kHz)"), wxPoint(10,160), wxSize(220, 28));
    T_nUnstable = new wxStaticText(panel, ID_TxtnUnstable, _T("Medidas para Estabilização"), wxPoint(10,190), wxSize(220, 28));
    T_fExcitation = new wxStaticText(panel, ID_TxtfExcitation, _T("Frequência de Excitação (kHz)"), wxPoint(10,220), wxSize(220, 28));

    P_nElectrode = new wxTextCtrl(panel, ID_CfgnElectrode, _T("???"), wxPoint(240,5), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_tCycle = new wxTextCtrl(panel, ID_CfgtCycle, _T("???"), wxPoint(240,35), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_Pattern = new wxTextCtrl(panel, ID_CfgPattern, _T("???"), wxPoint(240,65), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_nMeasure = new wxTextCtrl(panel, ID_CfgnMeasure, _T("???"), wxPoint(240,95), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_tTimeout = new wxTextCtrl(panel, ID_CfgtTimeout, _T("???"), wxPoint(240,125), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_fSample = new wxTextCtrl(panel, ID_CfgfSample, _T("???"), wxPoint(240,155), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_nUnstable = new wxTextCtrl(panel, ID_CfgnUnstable, _T("???"), wxPoint(240,185), wxSize(80,28), wxTE_PROCESS_ENTER);
    P_fExcitation = new wxTextCtrl(panel, ID_CfgfExcitation, _T("???"), wxPoint(240,215), wxSize(80,28), wxTE_PROCESS_ENTER);


    btnUpdate = new wxButton(panel, ID_Update, "OK", wxPoint(160,250), wxSize(70,28));
    btnCancel = new wxButton(panel, ID_Cancel, "Cancelar", wxPoint(240,250), wxSize(70,28));
    btnUpdate->Disable();

    //update = new wxTimer(this, ID_CfgTimer);
    //update->Start(1000);


    //opProtocol = new wxChoice(panel, ID_ChkBoxProto, wxPoint(100,61), wxSize(330,28));
    //opProtocol->Append("BasicBus V.01");
    //opProtocol->Append("Sniffer");
    //opProtocol->SetSelection(0);

    //opMode = new wxChoice(panel, ID_ChkBoxMode, wxPoint(100,1), wxSize(100,28), 2, commModes);
    //opMode->SetSelection(1);
	CreateStatusBar();
	SetStatusText("Atualizando Dados...");

	//SetSize(520,380);
    //run = (Scan*)main->run;
    //FileData = main->FileData;
	//run->Send(5);

}

void WConfig::OnTimer(wxTimerEvent& event)
{
    unsigned int ids[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double values[8];
    // Verify Database
    //ANDRE FileData->GetConfig(ids, values, 8);
    P_nElectrode->SetValue(wxString::FromCDouble(values[0]));
    P_tCycle->SetValue(wxString::FromCDouble(values[1]));
    P_Pattern->SetValue(wxString::FromCDouble(values[2]));
    P_nMeasure->SetValue(wxString::FromCDouble(values[3]));
    P_tTimeout->SetValue(wxString::FromCDouble(values[4]));
    P_fSample->SetValue(wxString::FromCDouble(values[5]));
    P_nUnstable->SetValue(wxString::FromCDouble(values[6]));
    P_fExcitation->SetValue(wxString::FromCDouble(values[7]));
}

void WConfig::OnUpdate(wxCommandEvent& event)
{

}

void WConfig::OnCancel(wxCommandEvent& event)
{

}
