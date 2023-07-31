#include "erroroverlay.h"

ErrorOverlay::ErrorOverlay(QWidget *parent) : QWidget(parent)
{
    parent_widget = static_cast<QWidget*>(parent);

    this->setStyleSheet("background-color: rgb(80, 80, 80);");
    this->setAutoFillBackground(true);
    QGraphicsOpacityEffect* newGraphicsEffect = new QGraphicsOpacityEffect(this);
    newGraphicsEffect->setOpacity(0.4f);
    this->setGraphicsEffect(newGraphicsEffect);
}

void ErrorOverlay::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    QPen newPen(p.pen());
    newPen.setWidth(6);
    newPen.setColor(Qt::white);
    p.setPen(newPen);

    QFont newFont(p.font());
    newFont.setPointSize(13);
    p.setFont(newFont);

    this->move(0, 0);
    this->resize(parent_widget->width(), parent_widget->height());

    // Draw the container
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // Draw message
    QFontMetrics fm(newFont);
    int messageWidth = fm.size(0, message).width();
    int messageHeight = fm.height() * 2 + 10;
    p.drawText((this->width() / 2) - (messageWidth / 2), (this->height() / 2) - (messageHeight / 2), messageWidth, messageHeight, Qt::AlignCenter, message);
}

void ErrorOverlay::SetMessage(QString message)
{
    this->message = message;
}
