#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QWidget>
#include <QBoxLayout>
#include <QMediaPlayer>
#include <QStyle>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractSlider;
class QComboBox;
QT_END_NAMESPACE

class PlayerControls : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerControls(QWidget *parent = nullptr);
    void playStatus_clicked();

signals:
    void play();

private:
    QAbstractButton *playStatus_button = nullptr; //控制播放状态
    QAbstractButton *preOne_button = nullptr; //控制上一首
    QAbstractButton *nextOne_button = nullptr;    //控制下一首
};

#endif // PLAYERCONTROLS_H
