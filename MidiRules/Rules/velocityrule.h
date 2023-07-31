#ifndef VELOCITYRULE_H
#define VELOCITYRULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QCheckBox>
#include <QPushButton>
#include <QString>

struct VelocityWorkerRuleParams
{
    bool check_larger_than_lower_limit;
    bool check_smaller_than_upper_limit;
    int velocity_lower_limit;
    int velocity_upper_limit;
};

class VelocityRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit VelocityRule(QObject *parent = nullptr);
        ~VelocityRule();


        class VelocityWorkerRule : public WorkerRule
        {
            public:
                VelocityWorkerRule();
                ~VelocityWorkerRule();

                VelocityWorkerRuleParams* rule_params;

                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
        };

        VelocityWorkerRuleParams* rule_params;

        WorkerRule* GetNewWorkerRule() override;
        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;

    signals:

};


class VelocityParameterDialog : public ParameterDialog
{
    public:
        VelocityParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent) {};
        ~VelocityParameterDialog(){};

        QCheckBox* allow_lower_limit_chk_box;
        QCheckBox* allow_upper_limit_chk_box;
        QLineEdit* lower_limit;
        QLineEdit* upper_limit;

        void* GetDialogOutPut() override;
};

#endif // VELOCITYRULE_H
