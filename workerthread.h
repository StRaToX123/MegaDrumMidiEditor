#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QQueue>
#include <qDebug>
#include <QList>
#include <QThread>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <teVirtualMIDI.h> // for creating virtual MIDI ports
#include <RtMidi.h> // for sending and receiving data over MIDI ports
#include <unordered_map>
#include <QFile>
#include <QDir>
#include <QRandomGenerator>


//#define DEBUG_PERFORMANCE
// if this define is set, then instead of processing the pedal_data
// all the data received doring the programs life-time will instead be saved to a txt
// this data can then be fed to the hihat thread for debug purposes
//#define HIHAT_SAVE_DATA_FOR_REPLAY
//#define HIHAT_REPLAY_SAVED_DATA // define this one and undefine HIHAT_SAVE_DATA_FOR_REPLAY in order for the replay to be fed to the thread
#define HIHAT_REPLAY_DATA_NAME "replay.txt"

#ifdef HIHAT_SAVE_DATA_FOR_REPLAY
    #include <chrono>
    #include <thread>
#endif

// Because we're hard coding the hihat pedal
// in order to not oversaturate the Work thread since the pedal constantly sends dummy data
// we're gonna create another thread specificaly to handle these hihat pedal messages
// and we have two threads now, the RtMidi callback function needs to check the midiMessage
// and then decide if it's gonna send it to the Work thread or the WorkHiHatHardCode thread.
// thats why each thread will put a pointer to it's self in this struct, and then this struct will be
// handed over to the callback as a userdata pointer, so that the callback can actually reference both threads.
struct CallBackUserData
{
    void* work;
    void* workHiHatHardCode;
};

static CallBackUserData gs_callBackUserData;

struct MidiMessage // declared as a q_meta_type in main.cpp
{
    unsigned char message[3];
    MidiMessage(){};
    MidiMessage(unsigned char&& mess01, unsigned char&& mess02, unsigned char&& mess03) : message{mess01, mess02, mess03}{};
};

class QueueEntry; // defined here right under the Work class
class WorkerRule; // defined in rule.h
class QueueEntryArray;

// rule needs to be included down here because it usses MidiMessage and QueueEntry
#include "MidiRules/rule.h"

class Work : public QObject
{
    Q_OBJECT

    public:
        Work();
        ~Work();
        void ResetPerTabNoteFamilyAndNameToRuleMatrixEntriesCache(int tabIndex);
        bool RequestTabSwitch(int tabIndex);
        bool RideHardCode();
        bool OpenOutputMidiPort(LPVM_MIDI_PORT& midiPort);
        bool FindMegaDrum(RtMidiIn*& midiPort);
        void Initialize();
        static void MegaDrumCallBack(double deltatime, std::vector<unsigned char>* message, void* userData);

        RtMidiIn* rt_midi_in;
        LPVM_MIDI_PORT midi_out; // passed to OpenMidiPort
        void* worker_thread; // pointer to worker_thread calling this work class
        bool keep_looping;
        // Notes are added to the queue from the back, the queue is traversed from back to front
        QueueEntryArray* queued_notes;
        int queue_current_note_index; // Index of the current note being proccessed
        QElapsedTimer thread_timer;
        bool send_ui_thread_info_about_every_note; // makes the thread decide for which notes to emit the MidiSendOut signal
        int queue_entry_id_counter;
        int active_tab_index;
        // if a tab_switch has been requested by any rule
        // this int would not be -1, if a tab switch has been requested then all subsequent tab switch requests
        // will be ignored
        // this int is set via the RequestTabSwitch function
        // it will also mark the last note's perform_tab_switch_here signaling that that is where we make the tab switch
        // Tab switches happen only if the note marked with the perform_tab_switch_here is the first note in the queue.
        // A tab switch will never happen as long as there are notes in the queue still requering the current tab to be the active one
        // Tab Switches happen when only the tab switch rule returns true, other rules after it don't have to evaluate to true
        // even if the note evaluates to true on it's request to switch a tab, it can still reset to queue_current_note_index to 0
        // if it's not the first one in the queue
        int tab_switch_index;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////// HARD CODED RIDE BOW AND BELL SEPARATION ///////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        bool previous_sent_note_was_ride_bell;


       class RuleMatrixEntry
       {
           public:
               RuleMatrixEntry(int noteFamily, int noteName)
               {
                   note_family = noteFamily;
                   note_name = noteName;
                   last_rule_processed_per_note.reserve(100);
                   this_matrix_entry_proccessed_per_note.reserve(100);
               };
               ~RuleMatrixEntry()
               {
                   for(int i = 0 ; i < rules.count(); i++)
                   {
                       delete rules[i];
                   }
               };
               int note_family;
               int note_name;
               QList<WorkerRule*> rules;
               // if a rule needs time to process and returs a pendig = true, we can use this index to know where we left off the next time we process it
               std::unordered_map<int, int> last_rule_processed_per_note;
               std::unordered_map<int, bool> this_matrix_entry_proccessed_per_note;
       };

       // The rule_matrix is a mirror image of how the ui thread sees all its midiNotes inside the midiNotesContainer and how each MidiNote has it's own rules
       QList<QList<RuleMatrixEntry*>> rule_matrix; // per tab per note rules
       // In order to skip having to go over the rule_matrix every time a new note comes in
       // we have this map, that acts like a cache
       // storing an array of rule_matrix indexes maped to a notes identifier (that is it's note family and name)
       // this way insted of allocating a new array of rule_matrix entries this kind of note folls under everytime a new one comes in
       // we can just reuse already created arrays (ones that were cached out)
       // (in other words, a slow start but a fast late game)
       QList<std::map<std::pair<int, int>, std::pair<int*, int>>> per_tab_map_note_family_and_name_to_rule_matrix_entries_cache;
       // Becouse each note can return different results for each of its rule_matrix entries
       // it is important that each note in the queue has a copy of the midi message for each
       // rule_matrix entry it will evaluate againts
       // this vector is where we keep all the midiMessage copies
       // the queued_note keeps track of a midi_messages_per_note_per_rule_matrix_entry_array_index that indexes into this array and gives the
       // the first midiMessage copy ment for the first rule_matrix entry that queued_note folls under
       // it is safe to traverse this array from that queued_notes's midi_messages_per_note_per_rule_matrix_entry_array_index onto that midi_messages_per_note_per_rule_matrix_entry_array_index + rule_matrix_indexes_array_length
       std::vector<MidiMessage> midi_messages_per_note_per_rule_matrix_entry_array;


#ifdef DEBUG_PERFORMANCE
       // Debug clock to check the performance
       QElapsedTimer performance_timer;
       bool performance_timer_started;
       qint64 performance_timer_start_value;
#endif


    public slots:
        void ReInitialize(); // call by this classes ReInitialize signal that gets received by the UI thread
        void CloseMegaDrum(); // Close the midi port
        void DoWork(); // Performs the main loop
        void AddRuleToRuleMatrix(int tabIndex, int noteIndex, int ruleIndex, void* workerRule);
        void RemoveRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex);
        void ModifyRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex, void* workerRuleParams);
        void AddNoteToRuleMatrix(int tabIndex, int noteFamily, int noteName);
        void RemoveNoteFromRuleMatrix(int tabIndex, int noteIndex);
        void ModifyNoteFromRuleMatrix(int tabIndex, int noteIndex, int newNoteFamily, int newNoteName);
        void StopLoop(); // call this so the doWork loop can be exited and the main thread loop would become active (to handle quit events)
        void AddNoteToQueueSlot(MidiMessage midiMessage);
        void SetSendUiThreadInfoAboutEveryNote(bool newValue);
        void CreateTab();
        void RemoveTab(int tabIndex);
        void ActivateTab(int tabIndex);
        void SendOutMidiMessageSlot(MidiMessage midiMessage);

    signals:
        // Error Code:
        // 0 : Initialization Successful
        // 1 : could not open drumMidi-Out channel
        // 2 : could not find MegaDrum
        void ErrorReporting(int errorCode); // emited if there is an error doring midi channel creation or megaDrum detection
        void AddNoteToQueueSignal(MidiMessage midiMessage); // used by the rtMidi Thread to active the slot of the same name on the worker thread
        void MidiNoteSentWorkerThread(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage* midiMessage); // tells the worker thread about a note that was just sent out, it will then forward the message to the UI thread
        void RuleWantsToSwitchTabWorkerThread(int tabIndex);
        void SendOutMidiMessageSignal(MidiMessage midiMessage);
};

class QueueEntry
{
    public:
    QueueEntry(int noteFamily,
                   int noteName,
                       int noteValue,
                           int midiMessagesPerNotePerRuleMatrixEntryArrayIndex,
                               int* ruleMatrixIndexesArray,
                                   int ruleMatrixIndexesArrayLength,
                                       int queueEntryId,
                                           bool testPending,
                                               bool testPassed) :
                      note_family(noteFamily),
                           note_name(noteName),
                               note_value(noteValue),
                                   midi_messages_per_note_per_rule_matrix_entry_array_index(midiMessagesPerNotePerRuleMatrixEntryArrayIndex),
                                       rule_matrix_indexes_array(ruleMatrixIndexesArray),
                                           rule_matrix_indexes_array_length(ruleMatrixIndexesArrayLength),
                                               rule_matrix_indexes_array_last_proccessed_in_a_row(0),
                                                   rule_matrix_indexes_array_total_proccessed(0),
                                                       queue_entry_id(queueEntryId),
                                                           test_pending(testPending),
                                                               test_passed(testPassed),
                                                                   perform_tab_switch_here(false),
                                                                       already_processed(false)
    {

    };

        ~QueueEntry()
        {

        };

        int note_family;
        int note_name;
        int note_value;
        int midi_messages_per_note_per_rule_matrix_entry_array_index;
        int* rule_matrix_indexes_array;
        int rule_matrix_indexes_array_length;
        int rule_matrix_indexes_array_last_proccessed_in_a_row;
        int rule_matrix_indexes_array_total_proccessed;

        int queue_entry_id; // unique number only for this note
        // a rule can set this to true if it's still not done checking and needs to wait, the worker thread will come back to it
        // or set this bool to false if the rule is done checking and can return a final value
        bool test_pending;
        // once test_pending is false, a rule should set this as it's final value
        bool test_passed;
        bool perform_tab_switch_here;
        bool already_processed;
};

class QueueEntryArray
{
    public:
        explicit QueueEntryArray(Work* parent = nullptr, int capasity = 100)
        {
            m_parent = parent;
            array.reserve(capasity);
        };
        ~QueueEntryArray()
        {

        };

        void Clear()
        {
            while(array.size() > 0)
            {
                Erase(0);
            }

            m_parent->keep_looping = false;
        };

        void Erase(int index)
        {
            for(int i = 0; i < array[index].rule_matrix_indexes_array_length; i++)
            {
                m_parent->rule_matrix[m_parent->active_tab_index][array[index].rule_matrix_indexes_array[i]]->last_rule_processed_per_note.erase(array[index].queue_entry_id);
                m_parent->rule_matrix[m_parent->active_tab_index][array[index].rule_matrix_indexes_array[i]]->this_matrix_entry_proccessed_per_note.erase(array[index].queue_entry_id);
                m_parent->midi_messages_per_note_per_rule_matrix_entry_array.erase(m_parent->midi_messages_per_note_per_rule_matrix_entry_array.begin() + array[index].midi_messages_per_note_per_rule_matrix_entry_array_index);
            }

            for(int i = (index + 1); i < array.size(); i++)
            {
                array[i].midi_messages_per_note_per_rule_matrix_entry_array_index -= array[index].rule_matrix_indexes_array_length;
            }

            array.erase(array.begin() + index);

            if(index <= m_parent->queue_current_note_index)
            {
                m_parent->queue_current_note_index--;
            }
        };

        Work* m_parent;
        std::vector<QueueEntry> array;
};

class WorkerThread : public QObject
{
    Q_OBJECT

    QThread thread;

    public:
        explicit WorkerThread(QObject* parent = nullptr);
        ~WorkerThread();

    signals:
        void Operate();
        void AddRuleToRuleMatrix(int tabIndex, int noteIndex, int ruleIndex, void*);
        void RemoveRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex);
        void ModifyRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex, void* workerRuleParams);
        void AddNoteToRuleMatrix(int tabIndex, int noteFamily, int noteName);
        void RemoveNoteFromRuleMatrix(int tabIndex, int noteIndex);
        void ModifyNoteFromRuleMatrix(int tabIndex, int noteIndex, int newNoteFamily, int newNoteName);
        void StopLoop();
        void ReInitialize(); // tells the Work class to run the Initialize function again
        void CloseMegaDrum(); // Close The Midi Port
        void InitializationError(int errorCode); // tells main thread what happened so that it can display it
        void SetSendUiThreadInfoAboutEveryNote(bool newValue);
        void MidiNoteSent(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage midiMessage);
        void CreateTab();
        void RemoveTab(int tabIndex);
        void ActivateTab(int tabIndex);
        void RuleWantsToSwitchTab(int tabIndex);

    public slots:
        void ErrorReporting(int errorCode); // gets the error code from worker
        void MidiNoteSentWorkerThread(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage* midiMessage);
        void RuleWantsToSwitchTabWorkerThread(int tabIndex);
};

class WorkHiHatHardCode : public QObject
{
    Q_OBJECT

    public:
        WorkHiHatHardCode(int chickNoteName, int splashNoteName, qint64 chickSplashTimeHoldTreshold);
        ~WorkHiHatHardCode();
        void Clamp(qint64& value, qint64 lowerLimit, qint64 higherLimit)
        {
            if(value < lowerLimit)
            {
                value = lowerLimit;
            }
            else
            {
                if(value > higherLimit)
                {
                    value = higherLimit;
                }
            }
        };
        void Remap(qint64& value, qint64 low1, qint64 high1, qint64 low2, qint64 high2)
        {
            value = low2 + (value - low1) * (high2 - low2) / (high1 - low1);
        };

        std::vector<MidiMessage> queued_notes;
        struct PedalLevelData
        {
            int lower_threshold;
            int upper_threshold;
            qint64 time_delta_threshold; // the delta time inbetween the current timer read and the previous timer read, in which this note must have arrived in order to be valid
            int note_count_towards_switch; // counter towards number_of_notes_needed_for_switch
            int note_count_mistakes_towarads_switch; // counter towards number_of_allowed_mistakes_towarads_switch
            int number_of_notes_needed_for_switch; // after this many notes were detected, a switch will happen
            int number_of_allowed_mistakes_towarads_switch; // since the signal bounces alot, this is the number of notes that dont contribute to a switch, but are allowed to happen inbetween the ones that do
            qint64 switch_start_timer_read; // when in time did we start a possible switch
            qint64 switch_timeout; // if the switch doesnt happen in this time period, then there was no switch at all
            qint64 last_time_a_note_for_this_pedal_level_was_received; // Only relevant when this is the level we're trying to switch to
            int bow_hit_note_name;
            int edge_hit_note_name;
            int cc_message_family;
            int cc_message_name;
            int cc_message_value;
            struct ChangeLevelDataFunction
            {
              using FunctionType = std::function<void()>;
              ChangeLevelDataFunction(FunctionType function) : fn {std::move(function)} {}
              FunctionType fn;
            };
            ChangeLevelDataFunction* change_levels_data_when_this_is_the_active_level;

            PedalLevelData(int lowerThreshold,
                               int upperThreshold,
                                 //  qint64 currentTime,
                                       qint64 timeDeltaThreshold,
                                           int numberOfNotesNeededForSwitch,
                                               int numberOfAllowedMistakesTowaradsSwitch,
                                                   qint64 switchTimeout,
                                                       int bowHitNoteName,
                                                           int edgeHitNoteName,
                                                               int ccMessageFamily,
                                                                   int ccMessageName,
                                                                       int ccMessageValue)
            {
                lower_threshold = lowerThreshold;
                upper_threshold = upperThreshold;
                time_delta_threshold = timeDeltaThreshold;
                last_time_a_note_for_this_pedal_level_was_received = 0;
                note_count_towards_switch = 0;
                note_count_mistakes_towarads_switch = 0;
                number_of_notes_needed_for_switch = numberOfNotesNeededForSwitch;
                number_of_allowed_mistakes_towarads_switch = numberOfAllowedMistakesTowaradsSwitch;
                switch_start_timer_read = 0;
                switch_timeout = switchTimeout;
                bow_hit_note_name = bowHitNoteName;
                edge_hit_note_name = edgeHitNoteName;
                change_levels_data_when_this_is_the_active_level = nullptr;
                cc_message_family = ccMessageFamily;
                cc_message_name = ccMessageName;
                cc_message_value = ccMessageValue;
            };
        };
        QList<PedalLevelData> pedal_level_data_array;
        int chick_note_name;
        int splash_note_name;
        // if we reach the last level and hold for longer then this time_threshold then a chick sound is played
        // otherwise if the pedal_level is switched back immediately a splash sound i played
        qint64 chick_splash_time_hold_treshold;
        // when this bool is true, the main loop will also check for a chick or a splash using the treshold above
        bool chick_check_is_active;
        bool splash_check_is_active;
        // 0 - Fully Open
        // 1 - Half Closed 1
        // 2 - Half Closed 2
        // 3 - Fully Closed
        // Initialy is set to 0
        int pedal_level;
        int pedal_level_trying_to_swith_to;
        int previous_pedal_level;
        qint64 current_timer_read;
        qint64 last_note_processed_timer_read; // used in order to count timeout switch to pedal_level 0
        qint64 last_level_switch_timer_read; // the last time a level switch occured
        qint64 moment_the_pedal_started_going_down_timer_read;
        QElapsedTimer timer;
        bool keep_looping;
        bool reset_notes_towards_switch_count_upon_receiving_current_level_note;
      //  bool counting_towards_level_zero_timeout_switch; // is set to true if we're currently counting towards a level zero timeout switch
        bool level_zero_timeout_switch_successful;
#if defined(HIHAT_SAVE_DATA_FOR_REPLAY) || defined(HIHAT_REPLAY_SAVED_DATA)
        QFile* replay_file;
        QTextStream* replay_file_text_stream;
        QList<qint64> replay_file_data_array;
        int replay_file_data_array_counter;
#endif
        bool failed_due_to_mistakes_count;
        bool failed_due_to_switch_timeout;
        bool failed_due_to_note_timeout;

    signals:
        void ProccessNoteSIGNAL(MidiMessage midiMessage);
        void Operate(); // Calls DoWork();

    public slots:
        void ProccessNoteSLOT(MidiMessage midiMessage);
        void DoWork();
        void StopLoop();
};

// This thread will be started in the constructor of the WorkerThread
class WorkerThreadHiHatHardCode : public QObject
{
    Q_OBJECT

    QThread thread;

    public:
        explicit WorkerThreadHiHatHardCode(QObject* parent = nullptr);
        ~WorkerThreadHiHatHardCode();

    signals:
        void StopLoop();
        void SendApplicationExitSignal();

    public slots:

};

#endif // WORKERTHREAD_H
