#include "dropallrule.h"

DropAllRule::DropAllRule(QObject *parent) : QObject(parent), Rule(171, 65, 160, "DA")
{
    rule_params = new DropAllWorkerRuleParams;
    rule_params->drop_all_note_family = 0;
    rule_params->drop_all_note_name = 0;
    rule_params->drop_all_velocity_lower_limit = 0;
    rule_params->drop_all_velocity_upper_limit = 0;
}

DropAllRule::~DropAllRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

DropAllRule::DropAllWorkerRule::DropAllWorkerRule()
{
    rule_params = nullptr;
}

DropAllRule::DropAllWorkerRule::~DropAllWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void DropAllRule::DropAllWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                                 int& queueCurrentNoteIndex,
                                                     std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                         int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                             qint64& workerThreadTime)
{
    if(rule_params != nullptr)
    {
        for(int i = 0; i < queuedNotes->array.size(); i++)
        {
            if(i != queueCurrentNoteIndex)
            { 
                int midiMessagesStartingIndex = queuedNotes->array[i].midi_messages_per_note_per_rule_matrix_entry_array_index;
                int midiMessagesEndingIndex = midiMessagesStartingIndex + queuedNotes->array[i].rule_matrix_indexes_array_length;
                for(int j = midiMessagesStartingIndex; j < midiMessagesEndingIndex; j++)
                {
                    if(queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] == false)
                    {
                        if(((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[1] == rule_params->drop_all_note_name)
                                && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[0] == rule_params->drop_all_note_family)
                                    && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] >= rule_params->drop_all_velocity_lower_limit)
                                        && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] <= rule_params->drop_all_velocity_upper_limit))
                        {
                            // instead of removing the note
                            // because the note as a whole might foll under multiple rule_matrixes
                            // it's not ok to remove the entire note, instead just invalidate the rule_matrixes that this note is having an issue with
                            queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] = true;
                            queuedNotes->array[i].rule_matrix_indexes_array_total_proccessed++;
                            // Clear caches of the note we're removing from all  the rules belonging to the rule matrix entry we just marked as processed
                            QList<WorkerRule*>* m_rules = &queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->rules;
                            int numberOfRules = (*(m_rules)).count();
                            for(int k = 0; k < numberOfRules; k++)
                            {
                                (*(m_rules))[k]->ResetAtIndex(queuedNotes->array[i].queue_entry_id, -1);
                            }
                        }
                    }
                }

                // in case incrementing this notes rule_matrix_indexes_array_total_proccessed doesnt cover all the rule_matrix entries the note folls under
                // we will need to update this notes rule_matrix_indexes_array_last_proccessed_in_a_row
                if(queuedNotes->array[i].rule_matrix_indexes_array_total_proccessed < queuedNotes->array[i].rule_matrix_indexes_array_length)
                {
                    for(int j = 0; j < queuedNotes->array[i].rule_matrix_indexes_array_length; j++)
                    {
                        if(queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] == false)
                        {
                            queuedNotes->array[i].rule_matrix_indexes_array_last_proccessed_in_a_row = j;
                            break;
                        }
                    }
                }
                else
                {
                    queuedNotes->array[i].already_processed = true;
                }
            }
        }

        queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
        queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
    }
}

void DropAllRule::DropAllWorkerRule::Reset()
{
    return;
}

void DropAllRule::DropAllWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void DropAllRule::DropAllWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<DropAllWorkerRuleParams*>(params);
}

WorkerRule* DropAllRule::GetNewWorkerRule()
{
    return new DropAllWorkerRule();
}

void* DropAllRule::GetRuleParams()
{
    return rule_params;
}

void* DropAllRule::GetRuleParamsCopy()
{
    DropAllWorkerRuleParams* paramsCopy = new DropAllWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof(DropAllWorkerRuleParams));
    return paramsCopy;
}

void DropAllRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<DropAllWorkerRuleParams*>(newParams);
}

QString DropAllRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "drop_all_note_family: ";
        result += QString::number(rule_params->drop_all_note_family);
        result += "\n";
        result += "drop_all_note_name: ";
        result += QString::number(rule_params->drop_all_note_name);
        result += "\n";
        result += "drop_all_velocity_lower_limit: ";
        result += QString::number(rule_params->drop_all_velocity_lower_limit);
        result += "\n";
        result += "drop_all_velocity_upper_limit: ";
        result += QString::number(rule_params->drop_all_velocity_upper_limit);
        result += "\n";
    }

    return result;
}

void DropAllRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->drop_all_note_family = paramsList[0].toInt();
    rule_params->drop_all_note_name = paramsList[1].toInt();
    rule_params->drop_all_velocity_lower_limit = paramsList[2].toInt();
    rule_params->drop_all_velocity_upper_limit = paramsList[3].toInt();
}

ParameterDialog* DropAllRule::GetDialogForParams(void *params)
{
    DropAllParameterDialog* newDialog = new DropAllParameterDialog();
    newDialog->resize(196, 228);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelDropAllNoteFamily = new QLabel(newDialog);
    QLabel* newLabelDropAllNoteName = new QLabel(newDialog);
    QLabel* newLabelDropAllNoteVelocityLowerLimit = new QLabel(newDialog);
    QLabel* newLabelDropAllNoteVelocityUpperLimit = new QLabel(newDialog);

    newLabelDropAllNoteFamily->setText("Note Family:");
    newLabelDropAllNoteName->setText("Note Name:");
    newLabelDropAllNoteVelocityLowerLimit->setText("Velocity Lower Limit:");
    newLabelDropAllNoteVelocityUpperLimit->setText("Velocity Upper Limit:");

    newGridLayout->addWidget(newLabelDropAllNoteFamily, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDropAllNoteName, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDropAllNoteVelocityLowerLimit, 2, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDropAllNoteVelocityUpperLimit, 3, 0, Qt::AlignCenter);

    DropAllWorkerRuleParams* currentParams = static_cast<DropAllWorkerRuleParams*>(params);

    newDialog->line_edit_drop_all_note_family = new QLineEdit(newDialog);
    newDialog->line_edit_drop_all_note_name = new QLineEdit(newDialog);
    newDialog->line_edit_drop_all_velocity_lower_limit = new QLineEdit(newDialog);
    newDialog->line_edit_drop_all_velocity_upper_limit = new QLineEdit(newDialog);

    newDialog->line_edit_drop_all_note_family->setText(QString::number(currentParams->drop_all_note_family));
    newDialog->line_edit_drop_all_note_name->setText(QString::number(currentParams->drop_all_note_name));
    newDialog->line_edit_drop_all_velocity_lower_limit->setText(QString::number(currentParams->drop_all_velocity_lower_limit));
    newDialog->line_edit_drop_all_velocity_upper_limit->setText(QString::number(currentParams->drop_all_velocity_upper_limit));

    newDialog->line_edit_drop_all_note_family->setAlignment(Qt::AlignCenter);
    newDialog->line_edit_drop_all_note_name->setAlignment(Qt::AlignCenter);
    newDialog->line_edit_drop_all_velocity_lower_limit->setAlignment(Qt::AlignCenter);
    newDialog->line_edit_drop_all_velocity_upper_limit->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    newDialog->line_edit_drop_all_note_family->setValidator(intValidator);
    newDialog->line_edit_drop_all_note_name->setValidator(intValidator);
    newDialog->line_edit_drop_all_velocity_lower_limit->setValidator(intValidator);
    newDialog->line_edit_drop_all_velocity_upper_limit->setValidator(intValidator);

    newGridLayout->addWidget(newDialog->line_edit_drop_all_note_family, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->line_edit_drop_all_note_name, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->line_edit_drop_all_velocity_lower_limit, 2, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->line_edit_drop_all_velocity_upper_limit, 3, 1, Qt::AlignCenter);

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

void* DropAllParameterDialog::GetDialogOutPut()
{
    DropAllWorkerRuleParams* params = new DropAllWorkerRuleParams;
    params->drop_all_note_family = line_edit_drop_all_note_family->text().toInt();
    params->drop_all_note_name = line_edit_drop_all_note_name->text().toInt();
    params->drop_all_velocity_lower_limit = line_edit_drop_all_velocity_lower_limit->text().toInt();
    params->drop_all_velocity_upper_limit = line_edit_drop_all_velocity_upper_limit->text().toInt();

    return params;
}

Rule* DropAllRule::GetNewInstance()
{
    return new DropAllRule();
}



