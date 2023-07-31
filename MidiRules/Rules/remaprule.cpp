#include "remaprule.h"

RemapRule::RemapRule(QObject *parent) : QObject(parent), Rule(201, 201, 52, "RM")
{
    rule_params = new RemapWorkerRuleParams;
    rule_params->remaped_name = 0;
    rule_params->remaped_family = 0;
    rule_params->remaped_value = 127;
    rule_params->velocity_remaping_linear_lerp = false;
    rule_params->velocity_linear_lerp_lower_limit = 0;
    rule_params->velocity_linear_lerp_upper_limit = 0;
    rule_params->velocity_linear_lerp_remap_lower_limit = 0;
    rule_params->velocity_linear_lerp_remap_upper_limit = 0;
}

RemapRule::~RemapRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

RemapRule::RemapWorkerRule::RemapWorkerRule()
{
    rule_params = nullptr;
}

RemapRule::RemapWorkerRule::~RemapWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void RemapRule::RemapWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<RemapWorkerRuleParams*>(params);
}

void RemapRule::RemapWorkerRule::Reset()
{
    return;
}

void RemapRule::RemapWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void RemapRule::RemapWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                       int& queueCurrentNoteIndex,
                                           std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                               int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                   qint64& workerThreadTime)
{
    if(rule_params != nullptr)
    {
        midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[0] = (unsigned char)rule_params->remaped_family;
        midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[1] = (unsigned char)rule_params->remaped_name;

        if(rule_params->velocity_remaping_linear_lerp == false)
        {    
            midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] = (unsigned char)rule_params->remaped_value;
        }
        else
        {
            if(midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] <= rule_params->velocity_linear_lerp_lower_limit)
            {
                midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] = rule_params->velocity_linear_lerp_remap_lower_limit;
                goto End;
            }
            if(midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] >= rule_params->velocity_linear_lerp_upper_limit)
            {
                midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] = rule_params->velocity_linear_lerp_remap_upper_limit;
                goto End;
            }

            // Lerp the value to be inbetween the new limits
            // low2 + (value - low1) * (high2 - low2) / (high1 - low1)
            midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] = (unsigned char)(rule_params->velocity_linear_lerp_remap_lower_limit + (((unsigned char)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2]) - rule_params->velocity_linear_lerp_lower_limit) * (rule_params->velocity_linear_lerp_remap_upper_limit - rule_params->velocity_linear_lerp_remap_lower_limit) / (rule_params->velocity_linear_lerp_upper_limit - rule_params->velocity_linear_lerp_lower_limit));
        }

        End:
        queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
        queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
    }
}

WorkerRule* RemapRule::GetNewWorkerRule()
{
    return new RemapWorkerRule();
}

void* RemapRule::GetRuleParams()
{
    return rule_params;
}

void* RemapRule::GetRuleParamsCopy()
{
    RemapWorkerRuleParams* paramsCopy = new RemapWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof (RemapWorkerRuleParams));
    return paramsCopy;
}

void RemapRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<RemapWorkerRuleParams*>(newParams);
}

QString RemapRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "remaped_family: ";
        result += QString::number(rule_params->remaped_family);
        result += "\n";
        result += "remaped_name: ";
        result += QString::number(rule_params->remaped_name);
        result += "\n";
        result += "velocity_remaping_linear_lerp: ";
        if(rule_params->velocity_remaping_linear_lerp == true)
        {
            result += QString::number(1);
        }
        else
        {
            result += QString::number(0);
        }
        result += "\n";
        result += "velocity_linear_lerp_lower_limit: ";
        result += QString::number(rule_params->velocity_linear_lerp_lower_limit);
        result += "\n";
        result += "velocity_linear_lerp_upper_limit: ";
        result += QString::number(rule_params->velocity_linear_lerp_upper_limit);
        result += "\n";
        result += "velocity_linear_lerp_remap_lower_limit: ";
        result += QString::number(rule_params->velocity_linear_lerp_remap_lower_limit);
        result += "\n";
        result += "velocity_linear_lerp_remap_upper_limit: ";
        result += QString::number(rule_params->velocity_linear_lerp_remap_upper_limit);
        result += "\n";
        result += "remaped_value: ";
        result += QString::number(rule_params->remaped_value);
        result += "\n";
    }

    return result;
}

void RemapRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->remaped_family = paramsList[0].toInt();
    rule_params->remaped_name = paramsList[1].toInt();
    if(paramsList[2].toInt() == 1)
    {
        rule_params->velocity_remaping_linear_lerp = true;
    }
    else
    {
        rule_params->velocity_remaping_linear_lerp = false;
    }
    rule_params->velocity_linear_lerp_lower_limit = paramsList[3].toInt();
    rule_params->velocity_linear_lerp_upper_limit = paramsList[4].toInt();
    rule_params->velocity_linear_lerp_remap_lower_limit = paramsList[5].toInt();
    rule_params->velocity_linear_lerp_remap_upper_limit = paramsList[6].toInt();
    rule_params->remaped_value = paramsList[7].toInt();
}

ParameterDialog* RemapRule::GetDialogForParams(void* params)
{
    RemapParameterDialog* newDialog = new RemapParameterDialog();
    newDialog->resize(187, 305);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelRemapFamily = new QLabel(newDialog);
    QLabel* newLabelRemapName = new QLabel(newDialog);
    QLabel* newLabelVelocityRemapingLinearLerp = new QLabel(newDialog);
    QLabel* newLabelVelocityLinearLerpLowerLimit = new QLabel(newDialog);
    QLabel* newLabelVelocityLinearLerpUpperLimit = new QLabel(newDialog);
    QLabel* newLabelVelocityLinearLerpRemapLowerLimit = new QLabel(newDialog);
    QLabel* newLabelVelocityLinearLerpRemapUpperLimit = new QLabel(newDialog);
    QLabel* newLabelRemapValue = new QLabel(newDialog);

    newLabelRemapFamily->setText("New Note Family:");
    newLabelRemapName->setText("New Note Name:");
    newLabelVelocityRemapingLinearLerp->setText("Enable Velocity Linear Remaping:");
    newLabelVelocityLinearLerpLowerLimit->setText("Remap Value Incoming Lower Limit:");
    newLabelVelocityLinearLerpUpperLimit->setText("Remap Value Incoming Upper Limit:");
    newLabelVelocityLinearLerpRemapLowerLimit->setText("Remap Value OutGoing Lower Limit:");
    newLabelVelocityLinearLerpRemapUpperLimit->setText("Remap Value Outgoing Upper Limit:");
    newLabelRemapValue->setText("New Note Value:");

    newGridLayout->addWidget(newLabelRemapFamily, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelRemapName, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelVelocityRemapingLinearLerp, 2, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelVelocityLinearLerpLowerLimit, 3, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelVelocityLinearLerpUpperLimit, 4, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelVelocityLinearLerpRemapLowerLimit, 5, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelVelocityLinearLerpRemapUpperLimit, 6, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelRemapValue, 7, 0, Qt::AlignCenter);

    newDialog->textEditRemapFamily = new QLineEdit(newDialog);
    newDialog->textEditRemapName = new QLineEdit(newDialog);
    newDialog->checkBoxVelocityRemapingLinearLerp = new QCheckBox(newDialog);
    newDialog->textEditVelocityLinearLerpLowerLimit = new QLineEdit(newDialog);
    newDialog->textEditVelocityLinearLerpUpperLimit = new QLineEdit(newDialog);
    newDialog->textEditVelocityLinearLerpRemapLowerLimit = new QLineEdit(newDialog);
    newDialog->textEditVelocityLinearLerpRemapUpperLimit = new QLineEdit(newDialog);
    newDialog->textEditRemapValue = new QLineEdit(newDialog);

    RemapWorkerRuleParams* currentParams = static_cast<RemapWorkerRuleParams*>(params);

    newDialog->textEditRemapFamily->setText(QString::number(currentParams->remaped_family));
    newDialog->textEditRemapName->setText(QString::number(currentParams->remaped_name));
    newDialog->checkBoxVelocityRemapingLinearLerp->setChecked(currentParams->velocity_remaping_linear_lerp);
    newDialog->textEditVelocityLinearLerpLowerLimit->setText(QString::number(currentParams->velocity_linear_lerp_lower_limit));
    newDialog->textEditVelocityLinearLerpUpperLimit->setText(QString::number(currentParams->velocity_linear_lerp_upper_limit));
    newDialog->textEditVelocityLinearLerpRemapLowerLimit->setText(QString::number(currentParams->velocity_linear_lerp_remap_lower_limit));
    newDialog->textEditVelocityLinearLerpRemapUpperLimit->setText(QString::number(currentParams->velocity_linear_lerp_remap_upper_limit));
    newDialog->textEditRemapValue->setText(QString::number(currentParams->remaped_value));

    newDialog->textEditRemapFamily->setAlignment(Qt::AlignCenter);
    newDialog->textEditRemapName->setAlignment(Qt::AlignCenter);
    newDialog->textEditVelocityLinearLerpLowerLimit->setAlignment(Qt::AlignCenter);
    newDialog->textEditVelocityLinearLerpUpperLimit->setAlignment(Qt::AlignCenter);
    newDialog->textEditVelocityLinearLerpRemapLowerLimit->setAlignment(Qt::AlignCenter);
    newDialog->textEditVelocityLinearLerpRemapUpperLimit->setAlignment(Qt::AlignCenter);
    newDialog->textEditRemapValue->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    newDialog->textEditRemapFamily->setValidator(intValidator);
    newDialog->textEditRemapName->setValidator(intValidator);
    newDialog->textEditVelocityLinearLerpLowerLimit->setValidator(intValidator);
    newDialog->textEditVelocityLinearLerpUpperLimit->setValidator(intValidator);
    newDialog->textEditVelocityLinearLerpRemapLowerLimit->setValidator(intValidator);
    newDialog->textEditVelocityLinearLerpRemapUpperLimit->setValidator(intValidator);
    newDialog->textEditRemapValue->setValidator(intValidator);

    newGridLayout->addWidget(newDialog->textEditRemapFamily, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditRemapName, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->checkBoxVelocityRemapingLinearLerp, 2, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditVelocityLinearLerpLowerLimit, 3, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditVelocityLinearLerpUpperLimit, 4, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditVelocityLinearLerpRemapLowerLimit, 5, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditVelocityLinearLerpRemapUpperLimit, 6, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->textEditRemapValue, 7, 1 , Qt::AlignCenter);

    ////////////////////////////////////////////////////////////////////
    connect(newDialog->checkBoxVelocityRemapingLinearLerp, SIGNAL(toggled(bool)), newDialog, SLOT(DialogChechBoxVelocityLinearLerpSlot(bool)));
    ////////////////////////////////////////////////////////////////////

    if(currentParams->velocity_remaping_linear_lerp == true)
    {
        newDialog->textEditRemapValue->setEnabled(false);
    }
    else
    {
        newDialog->textEditVelocityLinearLerpLowerLimit->setEnabled(false);
        newDialog->textEditVelocityLinearLerpUpperLimit->setEnabled(false);
        newDialog->textEditVelocityLinearLerpRemapLowerLimit->setEnabled(false);
        newDialog->textEditVelocityLinearLerpRemapUpperLimit->setEnabled(false);
    }

    // Create accept and cancel buttons
    QPushButton* newPushButtonAccept = new QPushButton(newDialog);
    newPushButtonAccept->setText("Accept");
    connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));

    QPushButton* newPushButtonReject = new QPushButton(newDialog);
    newPushButtonReject->setText("Cancel");
    connect(newPushButtonReject, SIGNAL(clicked()), newDialog, SLOT(reject()));

    newGridLayout->addWidget(newPushButtonAccept, 8, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newPushButtonReject, 9, 1, Qt::AlignCenter);

    return newDialog;
}

void RemapParameterDialog::DialogChechBoxVelocityLinearLerpSlot(bool checkState)
{
    DialogCheckBoxDisableFields(checkState, this);
}

void RemapParameterDialog::DialogCheckBoxDisableFields(bool checkState, RemapParameterDialog* dialog)
{
    if(checkState == false)
    {
        dialog->textEditVelocityLinearLerpLowerLimit->setEnabled(false);
        dialog->textEditVelocityLinearLerpUpperLimit->setEnabled(false);
        dialog->textEditVelocityLinearLerpRemapLowerLimit->setEnabled(false);
        dialog->textEditVelocityLinearLerpRemapUpperLimit->setEnabled(false);
        dialog->textEditRemapValue->setEnabled(true);
    }
    else
    {
        dialog->textEditVelocityLinearLerpLowerLimit->setEnabled(true);
        dialog->textEditVelocityLinearLerpUpperLimit->setEnabled(true);
        dialog->textEditVelocityLinearLerpRemapLowerLimit->setEnabled(true);
        dialog->textEditVelocityLinearLerpRemapUpperLimit->setEnabled(true);
        dialog->textEditRemapValue->setEnabled(false);
    }
}

Rule* RemapRule::GetNewInstance()
{
    return new RemapRule();
}

void* RemapParameterDialog::GetDialogOutPut()
{
    RemapWorkerRuleParams* newParams = new RemapWorkerRuleParams;
    newParams->remaped_family = textEditRemapFamily->text().toInt();
    newParams->remaped_name = textEditRemapName->text().toInt();
    newParams->remaped_value = textEditRemapValue->text().toInt();
    newParams->velocity_remaping_linear_lerp = checkBoxVelocityRemapingLinearLerp->isChecked();
    newParams->velocity_linear_lerp_lower_limit = textEditVelocityLinearLerpLowerLimit->text().toInt();
    newParams->velocity_linear_lerp_upper_limit = textEditVelocityLinearLerpUpperLimit->text().toInt();
    newParams->velocity_linear_lerp_remap_lower_limit = textEditVelocityLinearLerpRemapLowerLimit->text().toInt();
    newParams->velocity_linear_lerp_remap_upper_limit = textEditVelocityLinearLerpRemapUpperLimit->text().toInt();
    return newParams;
}
