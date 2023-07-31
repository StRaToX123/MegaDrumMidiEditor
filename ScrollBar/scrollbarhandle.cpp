#include "scrollbarhandle.h"

////////////////////////////////////
/////// ScrollBar Handle ///////////
////////////////////////////////////

ScrollBarHandle::ScrollBarHandle(QWidget *parent, ScrollBarParams* scrollBarParams) : QWidget(parent)
{
    mouse_pressed = false;
    scroll_bar_params = scrollBarParams;
    parent_widget = static_cast<QWidget*>(parent);
    handle_position = 0.0f;

    QString handleStyleSheet = "background-color : rgb(";
    handleStyleSheet += QString::number(scroll_bar_params->color_handle_r);
    handleStyleSheet += ", ";
    handleStyleSheet += QString::number(scroll_bar_params->color_handle_g);
    handleStyleSheet += ", ";
    handleStyleSheet += QString::number(scroll_bar_params->color_handle_b);
    handleStyleSheet += ");";
    handleStyleSheet += "border-radius: ";
    handleStyleSheet += QString::number(scroll_bar_params->handle_border_radius);
    handleStyleSheet += "px;";


    this->setStyleSheet(handleStyleSheet);
}

void ScrollBarHandle::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
         mouse_pressed = true;
         onClick_mouse_y_position = QCursor::pos().y();
    }
}

void ScrollBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    if(mouse_pressed == true)
    {
        int mousedelta = QCursor::pos().y() - onClick_mouse_y_position; // in pixels
        onClick_mouse_y_position = QCursor::pos().y();
        handle_position += mousedelta;

        if(handle_position < 0)
        {
            handle_position = 0;
        }

        if((handle_position + handle_size) > scroll_bar_params->background_height)
        {
            handle_position = scroll_bar_params->background_height - handle_size;
        }

        emit moveItems();
    }
}

void ScrollBarHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mouse_pressed = false;
    }
}


void ScrollBarHandle::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);

    QPainter p(this);

    int handleWidth = scroll_bar_params->background_width * scroll_bar_params->handle_width_percentage_01;
    int handlePositionX = scroll_bar_params->position_x + ((scroll_bar_params->background_width - handleWidth) / 2);
    this->move(handlePositionX, handle_position);
    this->resize(handleWidth, handle_size);

    // Draw the note
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);


    QWidget::paintEvent(event);
}
