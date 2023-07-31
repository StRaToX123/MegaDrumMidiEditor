#ifndef REMAPRULE_H
#define REMAPRULE_H

#include <QObject>
#include "MidiRules/rule.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QString>
#include <QCheckBox>
#include <QtAlgorithms> // for std::lerp

struct RemapWorkerRuleParams
{
    int remaped_family;
    int remaped_name;
    bool velocity_remaping_linear_lerp;
    int velocity_linear_lerp_lower_limit;
    int velocity_linear_lerp_upper_limit;
    int velocity_linear_lerp_remap_lower_limit;
    int velocity_linear_lerp_remap_upper_limit;
    int remaped_value;
};

class RemapRule : public QObject, public Rule
{
    Q_OBJECT

    public:
        explicit RemapRule(QObject *parent = nullptr);
        ~RemapRule();

        class RemapWorkerRule : public WorkerRule
        {
            public:
                RemapWorkerRule();
                ~RemapWorkerRule();

                RemapWorkerRuleParams* rule_params;

                void Check(QueueEntryArray*& queuedNotes,
                           int& queueCurrentNoteIndex,
                               std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                   int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                       qint64& workerThreadTime) override;
                void UpdateWorkerParams(void *params) override;
                void Reset() override;
                void ResetAtIndex(int index, int optional) override;
        };

        RemapWorkerRuleParams* rule_params;

        void* GetRuleParams() override;
        void * GetRuleParamsCopy() override;
        void SetRuleParams(void *newParams) override;
        void SetParamsWithStringList(QStringList &paramsList) override;
        QString GetParamsAsString() override;
        ParameterDialog* GetDialogForParams(void *params) override;
        Rule* GetNewInstance() override;
        WorkerRule* GetNewWorkerRule() override;
};

class RemapParameterDialog : public ParameterDialog
{
    Q_OBJECT

    public:
        RemapParameterDialog(QWidget* parent = nullptr) : ParameterDialog(parent) {};
        ~RemapParameterDialog(){};

        void DialogCheckBoxDisableFields(bool checkState, RemapParameterDialog* dialog);

        QLineEdit* textEditRemapFamily;
        QLineEdit* textEditRemapName;
        QLineEdit* textEditRemapValue;
        QCheckBox* checkBoxVelocityRemapingLinearLerp;
        QLineEdit* textEditVelocityLinearLerpLowerLimit;
        QLineEdit* textEditVelocityLinearLerpUpperLimit;
        QLineEdit* textEditVelocityLinearLerpRemapLowerLimit;
        QLineEdit* textEditVelocityLinearLerpRemapUpperLimit;

        void* GetDialogOutPut() override;

    public slots:
        void DialogChechBoxVelocityLinearLerpSlot(bool checkState);
};

#endif // REMAPRULE_H
