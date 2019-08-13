#include <memory>
#include <QtGui>
#include "FigApp.h"
using namespace std;

int main(int argc, char **argv)
{
    FigApp app(argc, argv);

    if(app.failed())
        return 1;
    
    return app.exec();
}

