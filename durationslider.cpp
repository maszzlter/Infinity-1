#include "durationslider.h"
#include <QDebug>
#include <QEvent>

void DurationSlider::mousePressEvent(QMouseEvent *event){
    //    this.x:控件原点到界面边缘的x轴距离；
    //    globalPos.x：鼠标点击位置到屏幕边缘的x轴距离；
    //    pos.x：鼠标点击位置到本控件边缘的距离；
    //    this.width:本控件的宽度;
    //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
    QSlider::mousePressEvent(event);
    m_isMoving = false;
    m_mousePress = true;
    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
    double pos = event->pos().x() / (double)width();
    double value = pos * (maximum() - minimum()) + minimum();
    //value + 0.5 四舍五入
    if(value>maximum()){
        value=maximum();
    }
    if(value<minimum()){
        value=minimum();
    }
    m_value=value+0.5;
    setValue(m_value);
    emit sliderMoved(m_value);
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));

    QCoreApplication::sendEvent(parentWidget(), &evEvent);
}
void DurationSlider::mouseMoveEvent(QMouseEvent *event){
    QSlider::mouseMoveEvent(event);
    double pos = event->pos().x() / (double)width();
    double value = pos * (maximum() - minimum()) + minimum();
    if(value>maximum()){
        value=maximum();
    }
    if(value<minimum()){
        value=minimum();
    }
    //value + 0.5 四舍五入
    if(m_mousePress){
        m_value=value + 0.5;
        m_isMoving=true;
        emit sliderMoved(m_value);
    }
    setValue(value + 0.5);
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);
}

void DurationSlider::mouseReleaseEvent(QMouseEvent *event){
    QSlider::mouseReleaseEvent(event);
    m_mousePress = false;
    m_isMoving=false;
    emit sliderReleasedAt(m_value);//抛出有用信号
}

DurationSlider::DurationSlider(QWidget *parent)
{
    m_value=0;
    m_mousePress = false;
    m_isMoving=false;
}

DurationSlider::~DurationSlider()
{

}


