#ifndef GATEWAYSERIAL_H_INCLUDED
#define GATEWAYSERIAL_H_INCLUDED


// For compilers that support precompilation, includes "wx/wx.h".
//#include <wx/app.h>
#include "WMain.h"

#ifdef _WINDOWS
    #include <windows.h>
#else
    #include<pthread.h>
    #include<unistd.h>
#endif
#include <sys/timeb.h>

//unsigned int stop = 0;

class MyApp : public wxApp
{
public:
	virtual bool OnInit()
	{
	    printf("Iniciando Janela\n");
		frame = new WMain( wxT("Supervisor"), wxPoint(50, 50), wxSize(450, 340), wxSYSTEM_MENU | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN);
		frame->Show( true );
		printf("Frame Showed\n");
		return true;
	}
	wxFrame* getMain()
	{
	    return frame;
	}
private:
	WMain *frame;
};

DECLARE_APP(MyApp)

#endif // GATEWAYSERIAL_H_INCLUDED
