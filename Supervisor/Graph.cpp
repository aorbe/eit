#include "Graph.h"


BEGIN_EVENT_TABLE(Graph, wxGLCanvas)
    EVT_PAINT    (Graph::Paintit)
END_EVENT_TABLE()

Graph::Graph(wxFrame *parent)
:wxGLCanvas(parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas")){
    int argc = 1;
    char* argv[1] = { wxString((wxTheApp->argv)[0]).char_str() };

/*
NOTE: this example uses GLUT in order to have a free teapot model
to display, to show 3D capabilities. GLUT, however, seems to cause problems
on some systems. If you meet problems, first try commenting out glutInit(),
then try comeenting out all glut code
*/
    glutInit(&argc, argv);
}


void Graph::Paintit(wxPaintEvent& WXUNUSED(event)){
    Render();
}

void Graph::Render()
{
//    SetCurrent();
//    wxPaintDC(this);
//    glClearColor(0.0, 0.0, 0.0, 0.0);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);
//
//    glBegin(GL_POLYGON);
//        glColor3f(1.0, 1.0, 1.0);
//        glVertex2f(-0.5, -0.5);
//        glVertex2f(-0.5, 0.5);
//        glVertex2f(0.5, 0.5);
//        glVertex2f(0.5, -0.5);
//        glColor3f(0.4, 0.5, 0.4);
//        glVertex2f(0.0, -0.8);
//    glEnd();
//
//    glBegin(GL_POLYGON);
//        glColor3f(1.0, 0.0, 0.0);
//        glVertex2f(0.1, 0.1);
//        glVertex2f(-0.1, 0.1);
//        glVertex2f(-0.1, -0.1);
//        glVertex2f(0.1, -0.1);
//    glEnd();
//
//// using a little of glut
//    glColor4f(0,0,1,1);
//    glutWireTeapot(0.4);
//
//    glLoadIdentity();
//    glColor4f(2,0,1,1);
//    glutWireTeapot(0.6);
//// done using glut
//
//    glFlush();
//    SwapBuffers();
}



