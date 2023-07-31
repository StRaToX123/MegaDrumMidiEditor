#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define MAIN_WINDOW_BACKGROUD_COLOR "#3B3B3B"

#include <QMainWindow>
#include <QPixmap>
#include <QMenuBar>
#include <QFrame>
#include <QStyle>
#include <QStatusBar>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include "workerthread.h"
#include "midimonitor.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:

        MainWindow(QWidget *parent = nullptr);
        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);
        void keyPressEvent(QKeyEvent* event); // used to set/reset worker_thread_not_initialized
        ~MainWindow();

    private:
        QRegion RoundedRect(const QRect& rect, int r);
        Ui::MainWindow *ui;

        QSize before_maximize_size;
        int before_maximize_pos_X;
        int before_maximize_pos_Y;

        WorkerThread* worker_thread;
        bool worker_thread_initialized; // used to know if we need to display
        WorkerThreadHiHatHardCode* worker_thread_hihat_harcode;
        MidiMonitorDialog* midi_monitor_dialog;

        QAction* open_close_mega_drum_action; // we need to keep a reference to this QAction because we need to switch the Find MegaDrum to Close MegaDrum and the other way around

    public slots:
        void InitializationError(int errorCode); // workerThread delivers initialization status messages including a success message through here
        void ExitApp();
        void MinimizeApp();
        void MaximizeApp();
        void Load();
        void Save();
        void CreateTab();
        void ActivateCurrentTab();
        void CreateNewMidiNote();
        void OpenMidiMonitor();
        void MidiMonitorClosed(); // gets called when the midi monitor closes. It will set midi_monitor_dialog to nullptr so we can know if we have a dialog to forward messages to
        void OpenCloseMegaDrum();

        // called by the workerThread when a note is sent out to the output MIDI port, this slot informs the ui thread of what was sent out
        // midiNotesContainerEntryListIndex is set to -1 if the note isn't covered by the program
        // those kind of notes will only be sent over if the MidiMonitor is open
        void MidiNoteSent(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage midiMessage);
        void CreateRuleMatrixEntryOnWorkerThread(int tabIndex, int noteFamily, int noteName); // called via the midiNotesContainer when it creates a new entry in it's entry_list, the midiNotesContainer tells the mainWindow which has a reference to the WorkerThread to create a new entry in it's rule_matrix
        void ModifyRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex, int newNoteFamily, int newNoteName);
        void RemoveRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex);
        void CreateRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule);
        void ModifyRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int workerRuleIndex, void* workerRuleParams);
        void RemoveRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int workerRuleIndex);
        void CreateTabWorkerThread();
        void RemoveTabWorkerThread(int tabIndex);
        void ActivateTabWorkerThread(int tabIndex);
        void WorkerThreadRuleWantsToSwitchTab(int tabIndex); // when a rule switches a tab on worker_thread, this is how it will relay that information to the ui_thread



};
#endif // MAINWINDOW_H
