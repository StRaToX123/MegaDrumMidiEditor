#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
//#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

class TitleBar : public QWidget
{
    Q_OBJECT

    public:

        explicit TitleBar(QWidget *parent = nullptr);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *ev);


    private:

        bool start_mouse_tracking;
        int offset_X;
        int offset_Y;
        bool* main_window_is_maximized;




    signals:

};

#endif // TITLEBAR_H
