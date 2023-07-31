#ifndef MIDIMONITOR_H
#define MIDIMONITOR_H

#include <QDialog>
//#include <qDebug>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include "globals.h"

#define BOTTOM_WIDGET_HORIZONTAL_INDENT 20

// this one line edit had to be separated from MidiNoteMonitorDialog because Qt doesnt see signals and slots from parent non QObject classes
// and we cant make MidiMonitorDialog a QObject and a QDialog at the same time since Qobject is alrady inside of QDialog
class MidiMonitor : public QObject
{
    Q_OBJECT

    public:
        MidiMonitor(QObject* parent = nullptr);
        ~MidiMonitor();
        QLineEdit* number_of_notes_allowed_on_display;
        int number_of_notes_allowed_on_display_value;

    signals:
        void MidiMonitorClosed(); // gets emited from midiMonitor destructor

    public slots:
        void NumberOfNotesAllowedOnDisplayChanged();
};


class MidiMonitorDialog : public QDialog
{
    public:
        MidiMonitorDialog(QWidget* parent = nullptr);
        ~MidiMonitorDialog();
        void paintEvent(QPaintEvent* event);
        void AddKnownNoteToMonitor(int noteFamily, QString& noteName, int noteValue, QColor& color);
        void AddUnknownNoteToMonitor(int noteFamily, int noteName, int noteValue);
        void DrawOnScreenNotes();

        QWidget* bottom_widget;
        QList<OnScreenNote*> on_screen_midi_notes;

        float font_size;

        MidiMonitor* midi_monitor;
};

#endif // MIDIMONITOR_H
