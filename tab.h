#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
//#include <QDebug>

#define LEFT_AND_RIGHT_MARGIN 8
#define TOP_AND_BOTTOM_MARGIN 3

class Tab : public QWidget
{
    Q_OBJECT

    public:
        explicit Tab(QWidget *parent = nullptr, int tabsContainerIndex = 0);
        float GetWidth();
        float GetHeight();
        void SetHeight(float height);
        void SetFontSize(float size, float monitorScaleFactor);
        void paintEvent(QPaintEvent* event);
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent (QMouseEvent* event);

        int tabs_container_index;
        QString name;
        float tab_width;
        float tab_height;
        float text_width;
        float text_height;
        float font_size;

        QPushButton* close_button;
        QFont default_font;

        // Parameters Dialog
        QLineEdit* line_edit_name;

    signals:
        void TabClicked(int tabsContainerIndex, bool emitRepaint);
        void RemoveTab(int tabsContainerIndex, bool applicationExiting);

    public slots:
        void CloseTab();

};

#endif // TAB_H
