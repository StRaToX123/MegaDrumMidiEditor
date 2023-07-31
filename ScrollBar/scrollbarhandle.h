#ifndef SCROLLBARHANDLE_H
#define SCROLLBARHANDLE_H

#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QtMath>
//#include <QDebug>
#include "scrollbarparams.h"

class ScrollBarHandle : public QWidget
{
    Q_OBJECT

    public:
        explicit ScrollBarHandle(QWidget *parent = nullptr, ScrollBarParams* scrollBarParams = nullptr);
        void paintEvent(QPaintEvent* event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        int handle_position; // relative to parent pixel position (TOP LEFT CORNER)
        int handle_size; // relative to parent pixel size (HEIGHT)

        ScrollBarParams* scroll_bar_params;

    private:
        QWidget* parent_widget;
        bool mouse_pressed;

        int onClick_mouse_y_position;


    signals:
        void moveItems();
};

#endif // SCROLLBARHANDLE_H
