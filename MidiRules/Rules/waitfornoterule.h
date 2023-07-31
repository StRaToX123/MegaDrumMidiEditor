#ifndef WAITFORNOTERULE_H
#define WAITFORNOTERULE_H

#include <QObject>
#include <QWidget>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QCheckBox>
#include <QPushButton>
//#include <QDebug>
#include <QString>

struct WaitForNoteWorkerRuleParams
{
    qint64 nano_seconds_timeout;
    int waiting_on_note_family;
    int waiting_on_note_name;
    int waiting_on_note_velocity_lower_limit;
    int waiting_on_note_velocity_upper_limit;
    bool note_waited_on_must_be_stronger;
    bool note_waited_on_must_be_weaker;
    bool drop_entry_that_is_being_waited_on;
    bool fail_on_timeout;
    bool drop_this_entry;
};

class WaitForNoteRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit WaitForNoteRule(QObject* parent = nullptr);
        ~WaitForNoteRule();


        class WaitForNoteWorkerRule : public WorkerRule
        {
            public:
                WaitForNoteWorkerRule();
                ~WaitForNoteWorkerRule();

                WaitForNoteWorkerRuleParams* rule_params;

                std::unordered_map<int, qint64> per_note_timer_start;

                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        WaitForNoteWorkerRuleParams* rule_params;

        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void* newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;
        WorkerRule* GetNewWorkerRule() override;

    signals:

};

class WaitForNoteParameterDialog : public ParameterDialog
{
    public:
        WaitForNoteParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent){};
        ~WaitForNoteParameterDialog(){};

        QLineEdit* nano_seconds_timeout;
        QLineEdit* waiting_on_note_family;
        QLineEdit* waiting_on_note_name;
        QLineEdit* waiting_on_note_value_lower_limit;
        QLineEdit* waiting_on_note_value_upper_limit;
        QCheckBox* note_waited_on_must_be_stronger;
        QCheckBox* note_waited_on_must_be_weaker;
        QCheckBox* drop_entry_that_is_being_waited_on;
        QCheckBox* fail_on_timeout;
        QCheckBox* drop_this_entry;

        void* GetDialogOutPut() override;
};

#endif // WAITFORNOTERULE_H
