#ifndef WAITFORTIMERULE_H
#define WAITFORTIMERULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QCheckBox>
#include <QPushButton>
#include <QString>
#include <QHash>

struct WaitForTimeWorkerRuleParams
{
    qint64 nano_seconds_timeout;
};

class WaitForTimeRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit WaitForTimeRule(QObject *parent = nullptr);
        ~WaitForTimeRule();

        class WaitForTimeWorkerRule : public WorkerRule
        {
            public:
                WaitForTimeWorkerRule();
                ~WaitForTimeWorkerRule();

                WaitForTimeWorkerRuleParams* rule_params;

                std::unordered_map<int, qint64> per_note_timer_start;

                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
        };

        WaitForTimeWorkerRuleParams* rule_params;

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void* newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void* params) override;
        Rule* GetNewInstance() override;

    signals:

};

class WaitForTimeParameterDialog : public ParameterDialog
{
    public:
        WaitForTimeParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent) {};
        ~WaitForTimeParameterDialog(){};

        QLineEdit* nano_seconds_timeout;

        void* GetDialogOutPut() override;
};

#endif // WAITFORTIMERULE_H
