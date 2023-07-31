#ifndef GLOBALS_H
#define GLOBALS_H

#include <QWidget>
#include <QLabel>

class OnScreenNote : public QWidget
{
    Q_OBJECT

    public:
        OnScreenNote(QWidget* parent = nullptr) : QWidget(parent)
        {
            processed = false;
            top_level_labels_deleted = false;
            bottom_level_labels_deleted = false;
        };
        ~OnScreenNote(){};

        QWidget* top_level_widget;
        QWidget* bottom_level_widget;
        QWidget* spacer_widget;

        // These variables are here because we will be deleteing some of these labels later on
        QLabel* top_level_widget_label_note_family;
        QLabel* top_level_widget_label_note_name;
        QLabel* top_level_widget_label_note_value;
        QLabel* bottom_level_widget_label_note_family;
        QLabel* bottom_level_widget_label_note_name;
        QLabel* bottom_level_widget_label_note_value;

        bool processed;
        bool top_level_labels_deleted;
        bool bottom_level_labels_deleted;

        int note_value;
};


#endif // GLOBALS_H
