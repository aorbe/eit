#ifndef GRAPH_H
#define GRAPH_H

#include <wx/wx.h>
#include <wx/glcanvas.h>

#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
   #include <GL/glut.h>
#endif

#ifndef WIN32
#include <unistd.h> // FIXME: ¿This work/necessary in Windows?
                    //Not necessary, but if it was, it needs to be replaced by process.h AND io.h
#endif

class Graph: public wxGLCanvas {
        void Render();
public:
    Graph(wxFrame* parent);
    void Paintit(wxPaintEvent& event);
protected:
    DECLARE_EVENT_TABLE()
};


#endif // GRAPH_H
