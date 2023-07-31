#ifndef MIDINOTE_H
#define MIDINOTE_H

#include <QWidget>
//#include <QDebug>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QRadialGradient>
#include <QLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>
#include "workerthread.h"
#include "globals.h"

#define NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS 4.0f
#define TOP_WIDGET_MIN_AND_MAX_HEIGHT 50

class MidiNote : public QWidget
{
    Q_OBJECT

    public:
        explicit MidiNote(QWidget *parent = nullptr);
        void paintEvent(QPaintEvent* event);
        void SetMinimumAndMaximumHorizontalVertical(int newHorizontal, int newVertical);
        QDialog* GetMidiNoteParametersDialog();
        void AddNoteToDisplay(MidiMessage& midiMessage);
        void DrawOnScreenNotes();

        int midi_notes_container_entry_list_tab_index;
        int midi_notes_container_entry_list_note_index; // which note in a row is this for the midiContainer

        QLabel* label_display_name;
        QFont font_label_display_name;
        QString display_name; // exists because midiNotesContainer will need to access the display_name variable in a function call which can't accept rvalues from label_display_name.text()
        int number_of_notes_on_display;
        int note_family;
        int note_name;
        QColor color;
        QColor color_copy; // used so the color picker window doesnt automatically change color, because the user can still click cancel on the parameters dialog


        // Parameters dialog
        QLineEdit* line_edit_note_family;
        QLineEdit* line_edit_note_name;
        QLineEdit* line_edit_display_name;
        QPushButton* push_button_color_picker;

        // Notes Display
        QWidget* bottom_widget;
        QList<OnScreenNote*> on_screen_notes;



    signals:
        void AddRuleRow(int tabIndex, int midiNoteIndex);
        void RemoveRuleRow(int tabIndex, int midiNoteIndex);
        void RemoveMidiNote(int tabIndex, int midiNoteIndex, bool tellWorkerThread, bool applicationExiting);
        void MidiNoteModified(int tabIndex, int entryListIndex);

    public slots:
        void CatchOnClickRemoveMidiNote();
        void CatchOnClickedAddRuleRow();
        void CatchOnClickedRemoveRuleRow();
        void CatchOnClickedConfigurationButton();
        void OpenColorPicker();


    private:
        QWidget* parent_widget;

};

#endif // MIDINOTE_H
