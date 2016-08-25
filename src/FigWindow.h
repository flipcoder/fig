#ifndef FIGWINDOW_H_Q0YWXBJS
#define FIGWINDOW_H_Q0YWXBJS

#include <QMainWindow>
#include "ui_fig.h"
#include "kit/meta/meta.h"

class FigApp;

class FigWindow:
    public QMainWindow
{
    Q_OBJECT
        
    public:

        FigWindow(FigApp* app);
        virtual ~FigWindow();

        FigWindow(const FigWindow&) = default;
        FigWindow(FigWindow&&) = default;
        FigWindow& operator=(const FigWindow&) = default;
        FigWindow& operator=(FigWindow&&) = default;

        FigApp* m_pApp;
        Ui::Fig m_UI;
        
    protected:
        
        void closeEvent(QCloseEvent* ev);
};

#endif

