/********************************************************************************
** Form generated from reading UI file 'fig.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FIG_H
#define UI_FIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Fig
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Fig)
    {
        if (Fig->objectName().isEmpty())
            Fig->setObjectName(QStringLiteral("Fig"));
        Fig->resize(800, 600);
        centralwidget = new QWidget(Fig);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        Fig->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Fig);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        Fig->setMenuBar(menubar);
        statusbar = new QStatusBar(Fig);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        Fig->setStatusBar(statusbar);

        retranslateUi(Fig);

        QMetaObject::connectSlotsByName(Fig);
    } // setupUi

    void retranslateUi(QMainWindow *Fig)
    {
        Fig->setWindowTitle(QApplication::translate("Fig", "Fig", 0));
    } // retranslateUi

};

namespace Ui {
    class Fig: public Ui_Fig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FIG_H
