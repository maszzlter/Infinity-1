#ifndef DURATIONSLIDER_H
#define DURATIONSLIDER_H

#include <QSlider>
#include <QWidget>
#include <QMouseEvent>
#include <QCoreApplication>

class DurationSlider : public QSlider
{
    Q_OBJECT
public:
    DurationSlider(QWidget * parent = 0);
    ~DurationSlider();

signals:
    void sliderReleasedAt(int);//当鼠标释放时，抛出包含鼠标X坐标位置信息的信号
    void sliderMoved(int);

public:  //mouse
    void mousePressEvent(QMouseEvent *event);  //单击
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    // virtual bool event(QEvent *event) override;
    bool m_isMoving;
    int m_value;
    bool m_mousePress;
};

#endif // DURATIONSLIDER_H
