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
    m_FigAppPath(argv[0])
{
    if(m_Args.size()>0)
    {
        m_SettingsFn = m_Args.get(m_Args.size()-1);
        m_SchemaFn = m_Args.value("schema");
    }

    try{
        m_Settings = make_shared<Meta>(m_SettingsFn);
    }catch(Error& e){
        QMessageBox::critical(m_pWindow.get(), "Error", "Could not load settings file.");
        quit();
        goto return_ctor;
    }
    
    try{
        m_Schema = make_shared<Schema>(m_SchemaFn);
    }catch(Error& e){
        QMessageBox::critical(m_pWindow.get(), "Error", "Could not load schema file.");
        quit();
        goto return_ctor;
    }
    
    m_pWindow = make_unique<FigWindow>(this);
    m_Success = true;
    
return_ctor: {}
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

