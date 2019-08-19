#include "FigWindow.h"
#include "FigApp.h"
using namespace std;

FigWindow :: FigWindow(FigApp* app):
    m_pApp(app)
{
}

FigWindow :: ~FigWindow()
{
}

void FigWindow :: closeEvent(QCloseEvent* ev)
{
    m_pApp->return_code(FigApp::RC_CLOSE);
}

