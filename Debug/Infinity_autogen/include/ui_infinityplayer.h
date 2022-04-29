/********************************************************************************
** Form generated from reading UI file 'infinityplayer.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INFINITYPLAYER_H
#define UI_INFINITYPLAYER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InfinityPlayer
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *InfinityPlayer)
    {
        if (InfinityPlayer->objectName().isEmpty())
            InfinityPlayer->setObjectName(QString::fromUtf8("InfinityPlayer"));
        InfinityPlayer->resize(800, 600);
        centralwidget = new QWidget(InfinityPlayer);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        InfinityPlayer->setCentralWidget(centralwidget);
        menubar = new QMenuBar(InfinityPlayer);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        InfinityPlayer->setMenuBar(menubar);
        statusbar = new QStatusBar(InfinityPlayer);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        InfinityPlayer->setStatusBar(statusbar);

        retranslateUi(InfinityPlayer);

        QMetaObject::connectSlotsByName(InfinityPlayer);
    } // setupUi

    void retranslateUi(QMainWindow *InfinityPlayer)
    {
        InfinityPlayer->setWindowTitle(QCoreApplication::translate("InfinityPlayer", "InfinityPlayer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfinityPlayer: public Ui_InfinityPlayer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INFINITYPLAYER_H
