#include <memory>
#include <QtGui>
#include "FigApp.h"
using namespace std;

int main(int argc, char **argv)
{
    FigApp app(argc, argv);
    
    if(app.failed())
        return FigApp::RC_ERROR;
    
    int r = app.exec();

    int rc = app.return_code();
    if(rc != 0)
        return rc;

    return r;
}

