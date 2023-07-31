#ifndef DROPNOTERULE_H
#define DROPNOTERULE_H

#include <QObject>
#include "MidiRules/rule.h"


class DropNoteRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit DropNoteRule(QObject *parent = nullptr);
        ~DropNoteRule();

        class DropNoteWorkerRule : public WorkerRule
        {
            public:
                DropNoteWorkerRule();
                ~DropNoteWorkerRule();

                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void* params) override;
        Rule* GetNewInstance() override;
};

#endif // DROPNOTERULE_H
