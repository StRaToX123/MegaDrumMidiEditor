#ifndef ERROROVERLAY_H
#define ERROROVERLAY_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QStyleOption>

class ErrorOverlay : public QWidget
{
    Q_OBJECT

    public:
        explicit ErrorOverlay(QWidget *parent = nullptr);
        void paintEvent(QPaintEvent* event);
        void SetMessage(QString message);

    private:
        QWidget* parent_widget;
        QString message;

    signals:

};

#endif // ERROROVERLAY_H
