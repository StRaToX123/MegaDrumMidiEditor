#include "activatetabrule.h"

ActivateTabRule::ActivateTabRule(QObject *parent) : QObject(parent), Rule(22, 193, 196, "AT")
{
    rule_params = new ActivateTabWorkerRuleParams;
    rule_params->tab_to_activate_index = 0;
}

ActivateTabRule::~ActivateTabRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

ActivateTabRule::ActivateTabWorkerRule::ActivateTabWorkerRule()
{
    rule_params = nullptr;
}

ActivateTabRule::ActivateTabWorkerRule::~ActivateTabWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void ActivateTabRule::ActivateTabWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                                 int& queueCurrentNoteIndex,
                                                     std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                         int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                             qint64& workerThreadTime)
{
    bool result = queuedNotes->m_parent->RequestTabSwitch(rule_params->tab_to_activate_index);
    queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
    queuedNotes->array[queueCurrentNoteIndex].test_passed = result;
}


void ActivateTabRule::ActivateTabWorkerRule::Reset()
{
    return;
}

void ActivateTabRule::ActivateTabWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void ActivateTabRule::ActivateTabWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<ActivateTabWorkerRuleParams*>(params);
}

WorkerRule* ActivateTabRule::GetNewWorkerRule()
{
    return new ActivateTabWorkerRule();
}

void* ActivateTabRule::GetRuleParams()
{
    return rule_params;
}

void* ActivateTabRule::GetRuleParamsCopy()
{
    ActivateTabWorkerRuleParams* paramsCopy = new ActivateTabWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof(ActivateTabWorkerRuleParams));
    return paramsCopy;
}

void ActivateTabRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<ActivateTabWorkerRuleParams*>(newParams);
}

QString ActivateTabRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "tab_to_activate_index: ";
        result += QString::number(rule_params->tab_to_activate_index);
        result += "\n";
    }

    return result;
}

void ActivateTabRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->tab_to_activate_index = paramsList[0].toInt();
}

ParameterDialog* ActivateTabRule::GetDialogForParams(void *params)
{
    ActivateTabParameterDialog* newDialog = new ActivateTabParameterDialog();
    newDialog->resize(183, 100);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelTabToActivateIndex = new QLabel(newDialog);

    newLabelTabToActivateIndex->setText("Tab To Activate:");

    newGridLayout->addWidget(newLabelTabToActivateIndex, 0, 0, Qt::AlignCenter);

    ActivateTabWorkerRuleParams* currentParams = static_cast<ActivateTabWorkerRuleParams*>(params);

    newDialog->line_edit_tab_to_activate_index = new QLineEdit(newDialog);

    newDialog->line_edit_tab_to_activate_index->setText(QString::number(currentParams->tab_to_activate_index));

    newDialog->line_edit_tab_to_activate_index->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    newDialog->line_edit_tab_to_activate_index->setValidator(intValidator);

    newGridLayout->addWidget(newDialog->line_edit_tab_to_activate_index, 0, 1, Qt::AlignCenter);

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

void* ActivateTabParameterDialog::GetDialogOutPut()
{
    ActivateTabWorkerRuleParams* params = new ActivateTabWorkerRuleParams;
    params->tab_to_activate_index = line_edit_tab_to_activate_index->text().toInt();

    return params;
}

Rule* ActivateTabRule::GetNewInstance()
{
    return new ActivateTabRule();
}



