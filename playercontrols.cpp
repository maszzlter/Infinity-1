#include "playercontrols.h"
#include <QToolButton>

PlayerControls::PlayerControls(QWidget *parent)
    : QWidget(parent)
{
    preOne_button = new QToolButton(this);
    preOne_button->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    playStatus_button = new QToolButton(this);
    playStatus_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    nextOne_button = new QToolButton(this);
    nextOne_button->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    QBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(preOne_button, 1);
    layout->addWidget(playStatus_button, 1);
    layout->addWidget(nextOne_button, 1);
    setLayout(layout);
    //掐换播放状态
    connect(playStatus_button, &QToolButton::clicked, this, [=] {
        emit playStatus_signal();
    });
    //上一首
    connect(preOne_button, &QToolButton::clicked, this, [=] {
        emit preOne_signal();
    });
    connect(nextOne_button, &QToolButton::clicked, this, [=] {
        emit nextOne_signal();
    });
}
