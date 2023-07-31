#include "createmessagerule.h"

CreateMessageRule::CreateMessageRule(QObject *parent) : QObject(parent), Rule(26, 244, 230, "CM")
{
    rule_params = new CreateMessageWorkerRuleParams;
    rule_params->create_message_note_family = 0;
    rule_params->create_message_note_name = 0;
    rule_params->create_message_note_value = 0;
}

CreateMessageRule::~CreateMessageRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

CreateMessageRule::CreateMessageWorkerRule::CreateMessageWorkerRule()
{
    rule_params = nullptr;
}

CreateMessageRule::CreateMessageWorkerRule::~CreateMessageWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void CreateMessageRule::CreateMessageWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                                 int& queueCurrentNoteIndex,
                                                     std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                         int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                             qint64& workerThreadTime)
{
    if(rule_params != nullptr)
    {
        emit queuedNotes->m_parent->AddNoteToQueueSignal(MidiMessage(std::move(rule_params->create_message_note_family),
                                                            std::move(rule_params->create_message_note_name),
                                                                    std::move(rule_params->create_message_note_value)));

        queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
        queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
    }
}

void CreateMessageRule::CreateMessageWorkerRule::Reset()
{
    return;
}

void CreateMessageRule::CreateMessageWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void CreateMessageRule::CreateMessageWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<CreateMessageWorkerRuleParams*>(params);
}

WorkerRule* CreateMessageRule::GetNewWorkerRule()
{
    return new CreateMessageWorkerRule();
}

void* CreateMessageRule::GetRuleParams()
{
    return rule_params;
}

void* CreateMessageRule::GetRuleParamsCopy()
{
    CreateMessageWorkerRuleParams* paramsCopy = new CreateMessageWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof(CreateMessageWorkerRuleParams));
    return paramsCopy;
}

void CreateMessageRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<CreateMessageWorkerRuleParams*>(newParams);
}

QString CreateMessageRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "create_message_note_family: ";
        result += QString::number(rule_params->create_message_note_family);
        result += "\n";
        result += "create_message_note_name: ";
        result += QString::number(rule_params->create_message_note_name);
        result += "\n";
        result += "create_message_note_value: ";
        result += QString::number(rule_params->create_message_note_value);
        result += "\n";
    }

    return result;
}

void CreateMessageRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->create_message_note_family = paramsList[0].toInt();
    rule_params->create_message_note_name = paramsList[1].toInt();
    rule_params->create_message_note_value = paramsList[2].toInt();
}

ParameterDialog* CreateMessageRule::GetDialogForParams(void *params)
{
    CreateMessageParameterDialog* newDialog = new CreateMessageParameterDialog();
    newDialog->resize(196, 228);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelCreateMessageNoteFamily = new QLabel(newDialog);
    QLabel* newLabelCreateMessageNoteName = new QLabel(newDialog);
    QLabel* newLabelCreateMessageNoteValue = new QLabel(newDialog);

    newLabelCreateMessageNoteFamily->setText("Note Family:");
    newLabelCreateMessageNoteName->setText("Note Name:");
    newLabelCreateMessageNoteValue->setText("Note Value:");

    newGridLayout->addWidget(newLabelCreateMessageNoteFamily, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelCreateMessageNoteName, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelCreateMessageNoteValue, 2, 0, Qt::AlignCenter);

    CreateMessageWorkerRuleParams* currentParams = static_cast<CreateMessageWorkerRuleParams*>(params);

    newDialog->line_edit_create_message_note_family = new QLineEdit(newDialog);
    newDialog->line_edit_create_message_note_name = new QLineEdit(newDialog);
    newDialog->line_edit_create_message_note_value = new QLineEdit(newDialog);

    newDialog->line_edit_create_message_note_family->setText(QString::number(currentParams->create_message_note_family));
    newDialog->line_edit_create_message_note_name->setText(QString::number(currentParams->create_message_note_name));
    newDialog->line_edit_create_message_note_value->setText(QString::number(currentParams->create_message_note_value));

    newDialog->line_edit_create_message_note_family->setAlignment(Qt::AlignCenter);
    newDialog->line_edit_create_message_note_name->setAlignment(Qt::AlignCenter);
    newDialog->line_edit_create_message_note_value->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    newDialog->line_edit_create_message_note_family->setValidator(intValidator);
    newDialog->line_edit_create_message_note_name->setValidator(intValidator);
    newDialog->line_edit_create_message_note_value->setValidator(intValidator);

    newGridLayout->addWidget(newDialog->line_edit_create_message_note_family, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->line_edit_create_message_note_name, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->line_edit_create_message_note_value, 2, 1, Qt::AlignCenter);

    // Create accept and cancel buttons
    QPushButton* newPushButtonAccept = new QPushButton(newDialog);
    newPushButtonAccept->setText("Accept");
    connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));

    QPushButton* newPushButtonReject = new QPushButton(newDialog);
    newPushButtonReject->setText("Cancel");
    connect(newPushButtonReject, SIGNAL(clicked()), newDialog, SLOT(reject()));

    newGridLayout->addWidget(newPushButtonAccept, 3, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newPushButtonReject, 4, 1, Qt::AlignCenter);

    return newDialog;
}

void* CreateMessageParameterDialog::GetDialogOutPut()
{
    CreateMessageWorkerRuleParams* params = new CreateMessageWorkerRuleParams;
    params->create_message_note_family = line_edit_create_message_note_family->text().toInt();
    params->create_message_note_name = line_edit_create_message_note_name->text().toInt();
    params->create_message_note_value = line_edit_create_message_note_value->text().toInt();

    return params;
}

Rule* CreateMessageRule::GetNewInstance()
{
    return new CreateMessageRule();
}



