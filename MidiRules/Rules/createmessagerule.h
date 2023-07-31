#ifndef CREATEMESSAGERULE_H
#define CREATEMESSAGERULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QString>

struct CreateMessageWorkerRuleParams
{
    int create_message_note_family;
    int create_message_note_name;
    int create_message_note_value;
};

class CreateMessageRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit CreateMessageRule(QObject *parent = nullptr);
        ~CreateMessageRule();

        class CreateMessageWorkerRule : public WorkerRule
        {
            public:
                CreateMessageWorkerRule();
                ~CreateMessageWorkerRule();

                CreateMessageWorkerRuleParams* rule_params;

                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        CreateMessageWorkerRuleParams* rule_params;

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void* GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;
};

class CreateMessageParameterDialog : public ParameterDialog
{
    public:
        CreateMessageParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent)
        {
        };
        ~CreateMessageParameterDialog()
        {
        };

        QLineEdit* line_edit_create_message_note_family;
        QLineEdit* line_edit_create_message_note_name;
        QLineEdit* line_edit_create_message_note_value;

        void* GetDialogOutPut() override;
};

#endif // CREATEMESSAGERULE_H
