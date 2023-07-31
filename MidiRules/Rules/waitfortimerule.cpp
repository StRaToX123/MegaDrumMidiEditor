#include "waitfortimerule.h"

WaitForTimeRule::WaitForTimeRule(QObject *parent) : QObject(parent), Rule(156, 62, 62, "WT")
{
    rule_params = new WaitForTimeWorkerRuleParams;
    rule_params->nano_seconds_timeout = 0;
}

WaitForTimeRule::~WaitForTimeRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

WaitForTimeRule::WaitForTimeWorkerRule::WaitForTimeWorkerRule()
{
    rule_params = nullptr;
    per_note_timer_start.reserve(10);
}

WaitForTimeRule::WaitForTimeWorkerRule::~WaitForTimeWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void WaitForTimeRule::WaitForTimeWorkerRule::UpdateWorkerParams(void *params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<WaitForTimeWorkerRuleParams*>(params);
}

void WaitForTimeRule::WaitForTimeWorkerRule::Reset()
{
    per_note_timer_start.clear();
}

// Expects the index to be the key for the map
void WaitForTimeRule::WaitForTimeWorkerRule::ResetAtIndex(int index, int optional)
{
    per_note_timer_start.erase(index);
}

void WaitForTimeRule::WaitForTimeWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                                   int& queueCurrentNoteIndex,
                                                       std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                           int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                               qint64& workerThreadTime)
{
    if(rule_params != nullptr)
    {
        try
        {
            qint64 foundTimerStartValue = per_note_timer_start.at(queuedNotes->array[queueCurrentNoteIndex].queue_entry_id);

            if((workerThreadTime - foundTimerStartValue) >= rule_params->nano_seconds_timeout)
            {
                queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
                queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
                per_note_timer_start.erase(queuedNotes->array[queueCurrentNoteIndex].queue_entry_id);
            }
            else
            {
                queuedNotes->array[queueCurrentNoteIndex].test_pending = true;
                queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
            }
        }
        catch(std::out_of_range ex)
        {
            per_note_timer_start[queuedNotes->array[queueCurrentNoteIndex].queue_entry_id] = workerThreadTime;
            queuedNotes->array[queueCurrentNoteIndex].test_pending = true;
            queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
        }
    }
}

WorkerRule* WaitForTimeRule::GetNewWorkerRule()
{
    return new WaitForTimeWorkerRule();
}

Rule* WaitForTimeRule::GetNewInstance()
{
    return new WaitForTimeRule();
}

void* WaitForTimeRule::GetRuleParams()
{
    return rule_params;
}

void* WaitForTimeRule::GetRuleParamsCopy()
{
    WaitForTimeWorkerRuleParams* paramsCopy = new WaitForTimeWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof (WaitForTimeWorkerRuleParams));
    return paramsCopy;
}

void WaitForTimeRule::SetRuleParams(void* newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<WaitForTimeWorkerRuleParams*>(newParams);
}

QString WaitForTimeRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "nano_seconds_timeout: ";
        result += QString::number(rule_params->nano_seconds_timeout);
        result += "\n";
    }

    return result;
}

void WaitForTimeRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->nano_seconds_timeout = paramsList[0].toULongLong();
}

ParameterDialog* WaitForTimeRule::GetDialogForParams(void *params)
{
    WaitForTimeParameterDialog* newDialog = new WaitForTimeParameterDialog();
    newDialog->resize(212, 151);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelNanoSecsTimeOut = new QLabel(newDialog);
    newLabelNanoSecsTimeOut->setText("Nanoseconds Timeout:");
    newGridLayout->addWidget(newLabelNanoSecsTimeOut, 0, 0, Qt::AlignCenter);

    WaitForTimeWorkerRuleParams* currentParams = static_cast<WaitForTimeWorkerRuleParams*>(params);
    newDialog->nano_seconds_timeout = new QLineEdit(newDialog);
    newDialog->nano_seconds_timeout->setText(QString::number(currentParams->nano_seconds_timeout));
    newDialog->nano_seconds_timeout->setAlignment(Qt::AlignCenter);
    auto intValidator = new QIntValidator(0, 2147483647, newDialog);
    newDialog->nano_seconds_timeout->setValidator(intValidator);
    newGridLayout->addWidget(newDialog->nano_seconds_timeout, 0, 1, Qt::AlignCenter);

    // Create accept and cancel buttons
    QPushButton* newPushButtonAccept = new QPushButton(newDialog);
    newPushButtonAccept->setText("Accept");
    connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));

    QPushButton* newPushButtonReject = new QPushButton(newDialog);
    newPushButtonReject->setText("Cancel");
    connect(newPushButtonReject, SIGNAL(clicked()), newDialog, SLOT(reject()));

    newGridLayout->addWidget(newPushButtonAccept, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newPushButtonReject, 2, 1, Qt::AlignCenter);

    return newDialog;
}

void* WaitForTimeParameterDialog::GetDialogOutPut()
{
    WaitForTimeWorkerRuleParams* newParams = new WaitForTimeWorkerRuleParams;
    newParams->nano_seconds_timeout = nano_seconds_timeout->text().toInt();

    return newParams;
}
