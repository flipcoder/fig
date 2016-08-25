#include "FigApp.h"
#include "FigWindow.h"
#include <vector>
#include "kit/kit.h"
#include "kit/log/log.h"
using namespace std;
using kit::make_unique;

FigApp :: FigApp(int& argc, char* argv[]):
    QApplication(argc,argv),
    m_Args(argc, (const char**)argv),
    m_FigAppPath(argv[0]),
    m_pWindow(make_unique<FigWindow>(this))
{
}

FigApp :: ~FigApp()
{
}

bool FigApp :: event(QEvent* ev)
{
    return QApplication::event(ev);
}

void FigApp :: quit()
{
    QApplication::quit();
}

