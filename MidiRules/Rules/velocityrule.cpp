#include "velocityrule.h"

VelocityRule::VelocityRule(QObject *parent) : QObject(parent), Rule(17, 35, 194, "VG")
{
    rule_params = new VelocityWorkerRuleParams;
    rule_params->check_larger_than_lower_limit = false;
    rule_params->check_smaller_than_upper_limit = false;
    rule_params->velocity_lower_limit = 0.0f;
    rule_params->velocity_upper_limit = 0.0f;
}

VelocityRule::~VelocityRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

VelocityRule::VelocityWorkerRule::VelocityWorkerRule()
{
    rule_params = nullptr;
}

VelocityRule::VelocityWorkerRule::~VelocityWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void VelocityRule::VelocityWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<VelocityWorkerRuleParams*>(params);
}

void VelocityRule::VelocityWorkerRule::Reset()
{
    return;
}

void VelocityRule::VelocityWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void VelocityRule::VelocityWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                             int& queueCurrentNoteIndex,
                                                 std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                     int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                         qint64& workerThreadTime)
{
    if(rule_params != nullptr)
    {
        if(rule_params->check_larger_than_lower_limit == true)
        {
            if((int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] >= rule_params->velocity_lower_limit)
            {
                queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
            }
            else
            {
                queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
                goto End;
            }
        }

        if(rule_params->check_smaller_than_upper_limit == true)
        {
            if((int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2] <= rule_params->velocity_upper_limit)
            {
                queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
            }
            else
            {
                queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
            }
        }

        End:
        queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
    }
}

WorkerRule* VelocityRule::GetNewWorkerRule()
{
    return new VelocityWorkerRule();
}

void* VelocityRule::GetRuleParams()
{
    return rule_params;
}

void* VelocityRule::GetRuleParamsCopy()
{
    VelocityWorkerRuleParams* paramsCopy = new VelocityWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof (VelocityWorkerRuleParams));
    return paramsCopy;
}

void VelocityRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<VelocityWorkerRuleParams*>(newParams);
}

QString VelocityRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "check_larger_than_lower_limit: ";
        if(rule_params->check_larger_than_lower_limit == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "check_smaller_than_upper_limit: ";
        if(rule_params->check_smaller_than_upper_limit == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "velocity_lower_limit: ";
        result += QString::number(rule_params->velocity_lower_limit);
        result += "\n";
        result += "velocity_upper_limit: ";
        result += QString::number(rule_params->velocity_upper_limit);
        result += "\n";
    }

    return result;
}

void VelocityRule::SetParamsWithStringList(QStringList &paramsList)
{
    if(paramsList[0].toInt() == 1)
    {
        rule_params->check_larger_than_lower_limit = true;
    }
    else
    {
        rule_params->check_larger_than_lower_limit = false;
    }
    if(paramsList[1].toInt() == 1)
    {
        rule_params->check_smaller_than_upper_limit = true;
    }
    else
    {
        rule_params->check_smaller_than_upper_limit = false;
    }
    rule_params->velocity_lower_limit = paramsList[2].toInt();
    rule_params->velocity_upper_limit = paramsList[3].toInt();
}

ParameterDialog* VelocityRule::GetDialogForParams(void *params)
{
    VelocityParameterDialog* newDialog = new VelocityParameterDialog();
    newDialog->resize(188, 266);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelAllowLowerLimitChkBox = new QLabel(newDialog);
    QLabel* newLabelLowerLimit = new QLabel(newDialog);
    QLabel* newLabelAllowUpperLimitChkBox = new QLabel(newDialog);
    QLabel* newLabelUpperLimit = new QLabel(newDialog);

    newLabelAllowLowerLimitChkBox->setText("Allow Lower Limit:");
    newLabelLowerLimit->setText("Lower Limit:");
    newLabelAllowUpperLimitChkBox->setText("Allow Upper Limit:");
    newLabelUpperLimit->setText("Upper Limit:");

    newGridLayout->addWidget(newLabelAllowLowerLimitChkBox, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelLowerLimit, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelAllowUpperLimitChkBox, 2, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelUpperLimit, 3, 0, Qt::AlignCenter);

    newDialog->allow_lower_limit_chk_box = new QCheckBox(newDialog);
    newDialog->allow_upper_limit_chk_box = new QCheckBox(newDialog);
    newDialog->lower_limit = new QLineEdit(newDialog);
    newDialog->upper_limit = new QLineEdit(newDialog);

    VelocityWorkerRuleParams* currentParams = static_cast<VelocityWorkerRuleParams*>(params);

    newDialog->allow_lower_limit_chk_box->setChecked(currentParams->check_larger_than_lower_limit);
    newDialog->allow_upper_limit_chk_box->setChecked(currentParams->check_smaller_than_upper_limit);
    newDialog->lower_limit->setText(QString::number(currentParams->velocity_lower_limit));
    newDialog->upper_limit->setText(QString::number(currentParams->velocity_upper_limit));

    newDialog->lower_limit->setAlignment(Qt::AlignCenter);
    newDialog->upper_limit->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    newDialog->lower_limit->setValidator(intValidator);
    newDialog->upper_limit->setValidator(intValidator);

    newGridLayout->addWidget(newDialog->allow_lower_limit_chk_box, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->lower_limit, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->allow_upper_limit_chk_box, 2, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->upper_limit, 3, 1, Qt::AlignCenter);

    // Create accept and cancel buttons
    QPushButton* newPushButtonAccept = new QPushButton(newDialog);
    newPushButtonAccept->setText("Accept");
    connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));

    QPushButton* newPushButtonReject = new QPushButton(newDialog);
    newPushButtonReject->setText("Cancel");
    connect(newPushButtonReject, SIGNAL(clicked()), newDialog, SLOT(reject()));

    newGridLayout->addWidget(newPushButtonAccept, 4, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newPushButtonReject, 5, 1, Qt::AlignCenter);

    return newDialog;
}

void* VelocityParameterDialog::GetDialogOutPut()
{
    VelocityWorkerRuleParams* newParams = new VelocityWorkerRuleParams;
    newParams->check_larger_than_lower_limit = allow_lower_limit_chk_box->isChecked();
    newParams->check_smaller_than_upper_limit = allow_upper_limit_chk_box->isChecked();
    newParams->velocity_lower_limit = lower_limit->text().toFloat();
    newParams->velocity_upper_limit = upper_limit->text().toFloat();

    return newParams;
}

Rule* VelocityRule::GetNewInstance()
{
    return new VelocityRule();
}

