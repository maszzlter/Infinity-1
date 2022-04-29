#include "playercontrols.h"
#include <QToolButton>

PlayerControls::PlayerControls(QWidget *parent)
    : QWidget(parent)
{
    preOne_button = new QToolButton(this);
    preOne_button->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    playStatus_button = new QToolButton(this);
    playStatus_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    nextOne_button = new QToolButton(this);
    nextOne_button->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    QBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(playStatus_button, 1);
    layout->addWidget(preOne_button, 1);
    layout->addWidget(nextOne_button, 1);
    setLayout(layout);
}
