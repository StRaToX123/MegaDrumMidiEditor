#ifndef MIDINOTESCONTAINERS_H
#define MIDINOTESCONTAINERS_H

#include <QWidget>
#include <QScrollBar>
#include <QPalette>
//#include <QDebug>
#include "midinote.h"
#include "tab.h"
#include "ScrollBar/scrollbar.h"
#include "MidiRules/rulescontainer.h"
#include "erroroverlay.h"
#include "workerthread.h"
#include "UIScalingForMonitorThisAppIsOn.h"
#include <utility>

#define TABS_CONTAINER_LEFT_AND_RIGHT_MARGIN 7
#define TABS_INBETWEEN_ITEMS_SPACE 3
#define TABS_SPACE_BETWEEN_SCROLL_BUTTON_AND_CENTRE_WIDGET 3

//#define STARTER_TAB_CONTAINER_HEIGHT 27 // it will get initialized when the program is first painted
#define STARTER_TAB_SCROLL_BUTTON_WIDTH 15
#define STARTER_TAB_FONT_SIZE_BIG 7
#define STARTER_TAB_FONT_SIZE_SMALL 5
#define STARTER_TAB_CLOSE_BUTTON_SIZE 8

#define TABS_SCROLL_AMOUNT 3

class MidiNotesContainer : public QWidget
{
    Q_OBJECT

    public:
        explicit MidiNotesContainer(QWidget *parent = nullptr);
        ~MidiNotesContainer();
        void paintEvent(QPaintEvent* event);
        void CreateMidiNote(bool emitRepaint);
        void DisplayErrorMessage(QString message);
        void HideErrorMessage();
        void AddMessageToMidiNote(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage& midiMessage);
        void ActivateCurrentTab(bool tellWorkerThread, bool emitRepaint);
        // The constructor needs the tabsContainer in order to initialize
        // but we can't get the tabs containers at the time Qt would normaly initialize this class
        // which is at the start in mainwindow.cpp when it calls ui->setupUi(this);
        // because at that point ui->tabs hasn't been created yet
        // thats why the whole class initialization will be called afterwards via the Initialize function
        // where we can safely pass it the ui->tabs reference
        void Initialize(QWidget* tabsContainer);


        struct Entry
        {
            MidiNote* midi_note;
            RulesContainer* rules_container;
        };

        QWidget* tabs_container; // gets set from mainWindow
        QWidget* tabs_container_left;
        QWidget* tabs_container_centre;
        QWidget* tabs_container_right;
        int active_tab;
        int current_selected_tab;
        QList<Tab*> tabs;
        QList<QList<Entry*>> per_tab_entry_list; // has as many QList<Entry*> as much as there are tabs in the tabs list
        QString active_tab_stylesheet;
        QString inactive_selected_tab_stylesheet;
        QString inactive_unselected_tab_stylesheet;
        QPushButton* tabs_scroll_left;
        QPushButton* tabs_scroll_right;
        int tabs_scroll_offset;
        ScrollBar* scroll_bar;
        QFrame* vertical_line;
        RuleTypes* rule_types;
        int scroll_bar_width;
        bool do_once;
        ErrorOverlay* error_overlay;
        float monitor_scale_factor;

        // Tabs size variables
        float starter_tab_container_height;

    signals:
        void CreateRuleMatrixEntryOnWorkerThread(int tabIndex, int noteFamily, int noteName);
        void ModifyRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex, int newNoteFamily, int newNoteName);
        void RemoveRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex);
        void CreateRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule);
        void ModifyRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRuleParams);
        void RemoveRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int ruleIndex);
        void CreateTabWorkerThread();
        void RemoveTabWorkerThread(int tabIndex);
        void ActivateTabWorkerThread(int tabIndex);


    public slots:
        void TabClicked(int tabsIndex, bool emitRepaint);
        void CreateTab(bool emitRepaint);
        void RemoveTab(int tabIndex, bool applicationExiting);
        void TabScrollLeft();
        void TabScrollRight();
        void RemoveMidiNote(int tabIndex, int midiNoteIndex, bool tellWorkerThread, bool applicationExiting);
        void MidiNoteModified(int tabIndex, int entryListIndex); // called via a note when it's parameters get changed, we need to tell the workerThread to change it's parameters as well
        void AddRuleRow(int tabIndex, int midiNoteIndex);
        void RemoveRuleRow(int tabIndex, int midiNoteIndex);
        void RuleRowRuleCreated(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule);
        void RuleRowRuleModified(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* ruleParams);
        void RuleRowRuleRemoved(int tabIndex, int ruleMatrixIndex, int ruleIndex);
};

#endif // MIDINOTESCONTAINERS_H
