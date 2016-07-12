#ifndef IMAIN_INCLUDED
#define IMAIN_INCLUDED

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/app.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#else
#include "wx_pch.h"
#endif


class IMain
{
    public:
    virtual wxString getConnectionInfo() = 0;
    virtual void Stop(wxCommandEvent &event) = 0;

};


#endif // IMAIN_INCLUDED
