#ifndef DROPALLRULE_H
#define DROPALLRULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QString>

struct DropAllWorkerRuleParams
{
    int drop_all_note_family;
    int drop_all_note_name;
    int drop_all_velocity_lower_limit;
    int drop_all_velocity_upper_limit;
};

class DropAllRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit DropAllRule(QObject *parent = nullptr);
        ~DropAllRule();

        class DropAllWorkerRule : public WorkerRule
        {
            public:
                DropAllWorkerRule();
                ~DropAllWorkerRule();

                DropAllWorkerRuleParams* rule_params;

                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        DropAllWorkerRuleParams* rule_params;

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;
};

class DropAllParameterDialog : public ParameterDialog
{
    public:
        DropAllParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent)
        {
        };
        ~DropAllParameterDialog()
        {
        };

        QLineEdit* line_edit_drop_all_note_family;
        QLineEdit* line_edit_drop_all_note_name;
        QLineEdit* line_edit_drop_all_velocity_lower_limit;
        QLineEdit* line_edit_drop_all_velocity_upper_limit;

        void* GetDialogOutPut() override;
};

#endif // DROPALLRULE_H
