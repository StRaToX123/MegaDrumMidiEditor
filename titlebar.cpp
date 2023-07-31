#include "titlebar.h"

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    start_mouse_tracking = false;
}


void TitleBar::mousePressEvent(QMouseEvent *event)
{ 
    if(window()->isMaximized() == false)
    {
        if(event->button() == Qt::LeftButton)
        {
            start_mouse_tracking = true;
            offset_X = QCursor::pos().x() - window()->pos().x();
            offset_Y = QCursor::pos().y() - window()->pos().y();
        }
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if(start_mouse_tracking == true)
    {
       window()->move(QCursor::pos().x() - offset_X,
                            QCursor::pos().y() - offset_Y);
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        start_mouse_tracking = false;
    }
}

void TitleBar::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    QFont font = p.font() ;

    /* twice the size than the current font size */
    font.setPointSize(font.pointSize() * 1.3);
    p.setPen(Qt::white);

    /* set the modified font to the painter */
    p.setFont(font);

    p.drawText((this->width() / 2) - 100, this->height() / 2 - 10, 200, this->height(), Qt::AlignHCenter, "midiDrum by PAULIO");

    return;
}

