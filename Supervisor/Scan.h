#ifndef SCAN_H
#define SCAN_H

#include "WMain.h"
#include "WMonitor.h"
#include "WRun.h"
#include "IProtocol.h"
#include <wx/thread.h>
#include <wx/event.h>


class Scan : public wxThread
{
public:
    Scan(WMain *handler, ICommChannel *channel, wxString port, int type);
    ~Scan();
    void Set(ICommChannel *channel, wxString port, int type);
    void Send(int command);
    void SetValue(long value);
protected:
    virtual ExitCode Entry();
    WMain *m_pHandler;
    IProtocol *proto;
    ICommChannel *channel;
    wxString* port;
    int type;
    int command;
    long value;
    wxCommandEvent* frmEvent;
private:
};

#endif // SCAN_H
