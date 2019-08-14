#ifndef FIGWINDOW_H_Q0YWXBJS
#define FIGWINDOW_H_Q0YWXBJS

#include <QDialog>
#include "ui_fig.h"
#include "kit/meta/meta.h"

class FigApp;

class FigWindow:
    public QDialog
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
        //Ui::Fig m_UI;

        //bool failed() const { return not m_bSuccess; }
        
    protected:
        
        void closeEvent(QCloseEvent* ev);

        //bool m_bSuccess = false;
};

#endif

