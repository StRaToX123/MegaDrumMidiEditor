#ifndef ACTIVATETABRULE_H
#define ACTIVATETABRULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QString>

struct ActivateTabWorkerRuleParams
{
    int tab_to_activate_index;
};

class ActivateTabRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit ActivateTabRule(QObject *parent = nullptr);
        ~ActivateTabRule();

        class ActivateTabWorkerRule : public WorkerRule
        {
            public:
                ActivateTabWorkerRule();
                ~ActivateTabWorkerRule();

                ActivateTabWorkerRuleParams* rule_params;


                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        ActivateTabWorkerRuleParams* rule_params;

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;
};

class ActivateTabParameterDialog : public ParameterDialog
{
    public:
        ActivateTabParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent)
        {

        };
        ~ActivateTabParameterDialog()
        {

        };

        QLineEdit* line_edit_tab_to_activate_index;

        void* GetDialogOutPut() override;
};

#endif // ACTIVATETABRULE_H
