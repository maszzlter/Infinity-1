#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QWidget>
#include <QBoxLayout>
#include <QMediaPlayer>
#include <QStyle>
#include <QTimer>


QT_BEGIN_NAMESPACE
class QToolButton;
class QSlider;
class QComboBox;
QT_END_NAMESPACE

class PlayerControls : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerControls(QWidget *parent = nullptr);
    void playStatus_clicked();

signals:
    void playStatus_signal();
    void preOne_signal();
    void nextOne_signal();

public:
    QToolButton *playStatus_button = nullptr; //控制播放状态
    QToolButton *preOne_button = nullptr; //控制上一首
    QToolButton *nextOne_button = nullptr;    //控制下一首

};

#endif // PLAYERCONTROLS_H
