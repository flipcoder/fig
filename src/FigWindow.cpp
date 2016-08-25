#include "FigWindow.h"
using namespace std;

FigWindow :: FigWindow(FigApp* app):
    m_pApp(app)
{
    m_UI.setupUi(this);
    //m_UI.taskList->setShowGrid(false);
    //m_UI.taskList->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(
    //    m_UI.taskList,
    //    SIGNAL(customContextMenuRequested(const QPoint&)),
    //    app,
    //    SLOT(ctxMenu(const QPoint&))
    //);
    show();
}

FigWindow :: ~FigWindow()
{
    
}

void FigWindow :: closeEvent(QCloseEvent* ev)
{
    //if(m_pApp->tray()->isVisible())
    //{
    //    hide();
    //    ev->ignore();
    //}
}

