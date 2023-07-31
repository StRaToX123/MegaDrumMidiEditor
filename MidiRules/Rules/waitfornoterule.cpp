#include "waitfornoterule.h"

WaitForNoteRule::WaitForNoteRule(QObject *parent) : QObject(parent), Rule(59, 148, 59, "WN")
{
    rule_params = new WaitForNoteWorkerRuleParams;
    rule_params->nano_seconds_timeout = 0;
    rule_params->waiting_on_note_name = 0;
    rule_params->waiting_on_note_family = 0;
    rule_params->waiting_on_note_velocity_lower_limit = 0;
    rule_params->waiting_on_note_velocity_upper_limit = 0;
    rule_params->note_waited_on_must_be_stronger = false;
    rule_params->note_waited_on_must_be_weaker = false;
    rule_params->drop_entry_that_is_being_waited_on = false;
    rule_params->fail_on_timeout = false;
    rule_params->drop_this_entry = false;
}

WaitForNoteRule::~WaitForNoteRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

WaitForNoteRule::WaitForNoteWorkerRule::WaitForNoteWorkerRule()
{
    rule_params = nullptr;
    per_note_timer_start.reserve(10);
}

WaitForNoteRule::WaitForNoteWorkerRule::~WaitForNoteWorkerRule()
{
    if(rule_params != nullptr)
    {
        delete rule_params;
        rule_params = nullptr;
    }
}

void WaitForNoteRule::WaitForNoteWorkerRule::UpdateWorkerParams(void* params)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<WaitForNoteWorkerRuleParams*>(params);
}

void WaitForNoteRule::WaitForNoteWorkerRule::Reset()
{
    per_note_timer_start.clear();
}

// Expects the index to be the key for the map
void WaitForNoteRule::WaitForNoteWorkerRule::ResetAtIndex(int index, int optional)
{
    per_note_timer_start.erase(index);
}

void WaitForNoteRule::WaitForNoteWorkerRule::Check(QueueEntryArray*& queuedNotes,
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
                //qDebug() << "WorkerThread: TIMED OUT " << (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[1] << (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2];
                queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
                if(rule_params->fail_on_timeout == true)
                {
                    queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
                }
                else
                {
                    queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
                }
                //////////////////////////////////////////////////////////////////////////////////////////////
                per_note_timer_start.erase(queuedNotes->array[queueCurrentNoteIndex].queue_entry_id);
                //////////////////////////////////////////////////////////////////////////////////////////////

                return;
            }
            else
            {
                queuedNotes->array[queueCurrentNoteIndex].test_pending = true;
                queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
            }
        }
        catch (std::out_of_range ex)
        {
            per_note_timer_start[queuedNotes->array[queueCurrentNoteIndex].queue_entry_id] = workerThreadTime;
            queuedNotes->array[queueCurrentNoteIndex].test_pending = true;
            queuedNotes->array[queueCurrentNoteIndex].test_passed = false;

            return;
        }

        for(int i = 0; i < queuedNotes->array.size(); i++)
        {
            bool exitLoop = false;
            int midiMessagesStartingIndex = queuedNotes->array[i].midi_messages_per_note_per_rule_matrix_entry_array_index;
            int midiMessagesEndingIndex = midiMessagesStartingIndex + queuedNotes->array[i].rule_matrix_indexes_array_length;
            for(int j = midiMessagesStartingIndex; j < midiMessagesEndingIndex; j++)
            {
                if((i == queueCurrentNoteIndex) && (j == midiMessagesPerNotePerMatrixEntryArrayIndexOffset))
                {
                    continue;
                }

                // only if the midiNoteIndexEntry has not been processed yet
                if(queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] == false)
                {
                    if(((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[1] == rule_params->waiting_on_note_name)
                            && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[0] == rule_params->waiting_on_note_family)
                                && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] >= rule_params->waiting_on_note_velocity_lower_limit)
                                    && ((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] <= rule_params->waiting_on_note_velocity_upper_limit))
                    {
                        //qDebug() << "WorkerThread: FOUND COPY " << (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[1] << (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2];
                        if(rule_params->note_waited_on_must_be_stronger == true)
                        {
                            // if the note we waited on ends up being weaker than our note we're evaluating this rule againts
                            // we can just continue
                            if((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] <= (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2])
                            {
                                continue;
                            }
                        }

                        if(rule_params->note_waited_on_must_be_weaker == true)
                        {
                            // if the note we waited on ends up being stronger than our note we're evaluating this rule againts
                            // we can just continue
                            if((int)midiMessagesPerNotePerRuleMatrixEntryArray[j].message[2] > (int)midiMessagesPerNotePerRuleMatrixEntryArray[midiMessagesPerNotePerMatrixEntryArrayIndexOffset].message[2])
                            {
                                continue;
                            }
                        }

                        queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
                        if(rule_params->drop_this_entry == true)
                        {
                            queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
                        }
                        else
                        {
                            queuedNotes->array[queueCurrentNoteIndex].test_passed = true;
                        }

                        //////////////////////////////////////////////////////////////////////////////////////////////
                        per_note_timer_start.erase(queuedNotes->array[queueCurrentNoteIndex].queue_entry_id);
                        //////////////////////////////////////////////////////////////////////////////////////////////

                        // Removing entry from queue
                        if(rule_params->drop_entry_that_is_being_waited_on == true)
                        {
                            // because the note as a whole might foll under multiple rule_matrixes
                            // it's not ok to remove the entire note, instead just invalidate the rule_matrixes that this note is having an issue with
                            queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] = true;
                            queuedNotes->array[i].rule_matrix_indexes_array_total_proccessed++;

                            QList<WorkerRule*>* m_rules = &queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[j - midiMessagesStartingIndex]]->rules;
                            int numberOfRules = (*(m_rules)).count();
                            for(int k = 0; k < numberOfRules; k++)
                            {
                                (*(m_rules))[k]->ResetAtIndex(queuedNotes->array[i].queue_entry_id, j);
                            }

                            // in case incrementing this notes rule_matrix_indexes_array_total_proccessed doesnt cover all the rule_matrix entries the note folls under
                            // we will need to update this notes rule_matrix_indexes_array_last_proccessed_in_a_row
                            if(queuedNotes->array[i].rule_matrix_indexes_array_total_proccessed < queuedNotes->array[i].rule_matrix_indexes_array_length)
                            {
                                for(int k = 0; k < queuedNotes->array[i].rule_matrix_indexes_array_length; k++)
                                {
                                    if(queuedNotes->m_parent->rule_matrix[queuedNotes->m_parent->active_tab_index][queuedNotes->array[i].rule_matrix_indexes_array[k]]->this_matrix_entry_proccessed_per_note[queuedNotes->array[i].queue_entry_id] == false)
                                    {
                                        queuedNotes->array[i].rule_matrix_indexes_array_last_proccessed_in_a_row = k;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                queuedNotes->array[i].already_processed = true;
                            }
                        }

                        exitLoop = true;
                        break;
                    }
                }
            }

            if(exitLoop == true)
            {
                break;
            }
        }
    }
}

WorkerRule* WaitForNoteRule::GetNewWorkerRule()
{
    return new WaitForNoteWorkerRule();
}

Rule* WaitForNoteRule::GetNewInstance()
{
    return new WaitForNoteRule();
}

void* WaitForNoteRule::GetRuleParams()
{
    return rule_params;
}

void* WaitForNoteRule::GetRuleParamsCopy()
{
    WaitForNoteWorkerRuleParams* paramsCopy = new WaitForNoteWorkerRuleParams;
    memcpy(paramsCopy, rule_params, sizeof (WaitForNoteWorkerRuleParams));
    return paramsCopy;
}

void WaitForNoteRule::SetRuleParams(void *newParams)
{
    if(rule_params != nullptr)
    {
        delete rule_params;
    }
    rule_params = static_cast<WaitForNoteWorkerRuleParams*>(newParams);
}

QString WaitForNoteRule::GetParamsAsString()
{
    QString result;
    if(rule_params != nullptr)
    {
        result += "nano_seconds_timeout: ";
        result += QString::number(rule_params->nano_seconds_timeout);
        result += "\n";
        result += "waiting_on_note_family: ";
        result += QString::number(rule_params->waiting_on_note_family);
        result += "\n";
        result += "waiting_on_note_name: ";
        result += QString::number(rule_params->waiting_on_note_name);
        result += "\n";
        result += "waiting_on_note_velocity_lower_limit: ";
        result += QString::number(rule_params->waiting_on_note_velocity_lower_limit);
        result += "\n";
        result += "waiting_on_note_velocity_upper_limit: ";
        result += QString::number(rule_params->waiting_on_note_velocity_upper_limit);
        result += "\n";
        result += "delete_note_that_is_being_waited_on: ";
        if(rule_params->drop_entry_that_is_being_waited_on == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "fail_on_timeout: ";
        if(rule_params->fail_on_timeout == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "drop_self: ";
        if(rule_params->drop_this_entry == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "note_waited_on_must_be_stronger: ";
        if(rule_params->note_waited_on_must_be_stronger == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
        result += "note_waited_on_must_be_weaker: ";
        if(rule_params->note_waited_on_must_be_weaker == true)
        {
            result += "1";
        }
        else
        {
            result += "0";
        }
        result += "\n";
    }

    return result;
}

void WaitForNoteRule::SetParamsWithStringList(QStringList &paramsList)
{
    rule_params->nano_seconds_timeout = paramsList[0].toULongLong();
    rule_params->waiting_on_note_family = paramsList[1].toInt();
    rule_params->waiting_on_note_name = paramsList[2].toInt();
    rule_params->waiting_on_note_velocity_lower_limit = paramsList[3].toInt();
    rule_params->waiting_on_note_velocity_upper_limit = paramsList[4].toInt();
    if(paramsList[5].toInt() == 1)
    {
        rule_params->drop_entry_that_is_being_waited_on = true;
    }
    else
    {
        rule_params->drop_entry_that_is_being_waited_on = false;
    }
    if(paramsList[6].toInt() == 1)
    {
        rule_params->fail_on_timeout = true;
    }
    else
    {
        rule_params->fail_on_timeout = false;
    }
    if(paramsList[7].toInt() == 1)
    {
        rule_params->drop_this_entry = true;
    }
    else
    {
        rule_params->drop_this_entry = false;
    }
    if(paramsList[8].toInt() == 1)
    {
        rule_params->note_waited_on_must_be_stronger = true;
    }
    else
    {
        rule_params->note_waited_on_must_be_stronger = false;
    }
    if(paramsList[9].toInt() == 1)
    {
        rule_params->note_waited_on_must_be_weaker = true;
    }
    else
    {
        rule_params->note_waited_on_must_be_weaker = false;
    }
}

ParameterDialog* WaitForNoteRule::GetDialogForParams(void* params)
{
    WaitForNoteParameterDialog* newDialog = new WaitForNoteParameterDialog();
    newDialog->resize(218, 324);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelNanoSecondsTimeout = new QLabel(newDialog);
    QLabel* newLabelWaitOnNoteFamily = new QLabel(newDialog);
    QLabel* newLabelWaitOnNoteName = new QLabel(newDialog);
    QLabel* newLabelWaitOnNoteValueLowerLimit = new QLabel(newDialog);
    QLabel* newLabelWaitOnNoteValueUpperLimit = new QLabel(newDialog);
    QLabel* newLabelNoteWaitedOnMustBeStronger = new QLabel(newDialog);
    QLabel* newLabelNoteWaitedOnMustBeWeaker = new QLabel(newDialog);
    QLabel* newLabelDeleteNoteWaitedOnChkBox = new QLabel(newDialog);
    QLabel* newLabelFailOnTimeout = new QLabel(newDialog);
    QLabel* newLabelDropSelf = new QLabel(newDialog);

    newLabelNanoSecondsTimeout->setText("Nanoseconds Timeout:");
    newLabelWaitOnNoteFamily->setText("Wait For Note Family:");
    newLabelWaitOnNoteName->setText("Wait For Note Name:");
    newLabelWaitOnNoteValueLowerLimit->setText("Wait For Value Lower Limit:");
    newLabelWaitOnNoteValueUpperLimit->setText("Wait For Value Upper Limit:");
    newLabelNoteWaitedOnMustBeStronger->setText("Note Waited On Must Be Stronger:");
    newLabelNoteWaitedOnMustBeWeaker->setText("Note Waited On Must Be Weaker:");
    newLabelDeleteNoteWaitedOnChkBox->setText("Delete Entry Waited On:");
    newLabelFailOnTimeout->setText("Fail on Timeout:");
    newLabelDropSelf->setText("Drop This Entry:");

    newGridLayout->addWidget(newLabelNanoSecondsTimeout, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelWaitOnNoteFamily, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelWaitOnNoteName, 2, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelWaitOnNoteValueLowerLimit, 3, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelWaitOnNoteValueUpperLimit, 4, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelNoteWaitedOnMustBeStronger, 5, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelNoteWaitedOnMustBeWeaker, 6, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDeleteNoteWaitedOnChkBox, 7, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelFailOnTimeout, 8, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDropSelf, 9, 0, Qt::AlignCenter);

    newDialog->note_waited_on_must_be_stronger = new QCheckBox(newDialog);
    newDialog->note_waited_on_must_be_weaker = new QCheckBox(newDialog);
    newDialog->drop_entry_that_is_being_waited_on = new QCheckBox(newDialog);
    newDialog->nano_seconds_timeout = new QLineEdit(newDialog);
    newDialog->waiting_on_note_family = new QLineEdit(newDialog);
    newDialog->waiting_on_note_name = new QLineEdit(newDialog);
    newDialog->waiting_on_note_value_lower_limit = new QLineEdit(newDialog);
    newDialog->waiting_on_note_value_upper_limit = new QLineEdit(newDialog);
    newDialog->fail_on_timeout = new QCheckBox(newDialog);
    newDialog->drop_this_entry = new QCheckBox(newDialog);

    WaitForNoteWorkerRuleParams* currentParams = static_cast<WaitForNoteWorkerRuleParams*>(params);

    newDialog->drop_entry_that_is_being_waited_on->setChecked(currentParams->drop_entry_that_is_being_waited_on);
    newDialog->nano_seconds_timeout->setText(QString::number(currentParams->nano_seconds_timeout));
    newDialog->waiting_on_note_family->setText(QString::number(currentParams->waiting_on_note_family));
    newDialog->waiting_on_note_name->setText(QString::number(currentParams->waiting_on_note_name));
    newDialog->waiting_on_note_value_lower_limit->setText(QString::number(currentParams->waiting_on_note_velocity_lower_limit));
    newDialog->waiting_on_note_value_upper_limit->setText(QString::number(currentParams->waiting_on_note_velocity_upper_limit));
    newDialog->fail_on_timeout->setChecked(currentParams->fail_on_timeout);
    newDialog->drop_this_entry->setChecked(currentParams->drop_this_entry);
    newDialog->note_waited_on_must_be_stronger->setChecked(currentParams->note_waited_on_must_be_stronger);
    newDialog->note_waited_on_must_be_weaker->setChecked(currentParams->note_waited_on_must_be_weaker);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    auto nanoSecsIntValidator = new QIntValidator(0, 2147483647, newDialog);
    newDialog->waiting_on_note_family->setValidator(intValidator);
    newDialog->waiting_on_note_name->setValidator(intValidator);
    newDialog->waiting_on_note_value_lower_limit->setValidator(intValidator);
    newDialog->waiting_on_note_value_upper_limit->setValidator(intValidator);
    newDialog->nano_seconds_timeout->setValidator(nanoSecsIntValidator);

    newGridLayout->addWidget(newDialog->nano_seconds_timeout, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->waiting_on_note_family, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->waiting_on_note_name, 2, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->waiting_on_note_value_lower_limit, 3, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->waiting_on_note_value_upper_limit, 4, 1 ,Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->note_waited_on_must_be_stronger, 5, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->note_waited_on_must_be_weaker, 6, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->drop_entry_that_is_being_waited_on, 7, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->fail_on_timeout, 8, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newDialog->drop_this_entry, 9, 1, Qt::AlignCenter);

    // Create accept and cancel buttons
    QPushButton* newPushButtonAccept = new QPushButton(newDialog);
    newPushButtonAccept->setText("Accept");
    connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));

    QPushButton* newPushButtonReject = new QPushButton(newDialog);
    newPushButtonReject->setText("Cancel");
    connect(newPushButtonReject, SIGNAL(clicked()), newDialog, SLOT(reject()));

    newGridLayout->addWidget(newPushButtonAccept, 10, 1, Qt::AlignCenter);
    newGridLayout->addWidget(newPushButtonReject, 11, 1, Qt::AlignCenter);

    return newDialog;
}

void* WaitForNoteParameterDialog::GetDialogOutPut()
{
    WaitForNoteWorkerRuleParams* newParams = new WaitForNoteWorkerRuleParams;
    newParams->nano_seconds_timeout = nano_seconds_timeout->text().toInt();
    newParams->waiting_on_note_name = waiting_on_note_name->text().toInt();
    newParams->waiting_on_note_velocity_lower_limit = waiting_on_note_value_lower_limit->text().toInt();
    newParams->waiting_on_note_velocity_upper_limit = waiting_on_note_value_upper_limit->text().toInt();
    newParams->waiting_on_note_family = waiting_on_note_family->text().toInt();
    newParams->drop_entry_that_is_being_waited_on = drop_entry_that_is_being_waited_on->isChecked();
    newParams->fail_on_timeout = fail_on_timeout->isChecked();
    newParams->drop_this_entry = drop_this_entry->isChecked();
    newParams->note_waited_on_must_be_stronger = note_waited_on_must_be_stronger->isChecked();
    newParams->note_waited_on_must_be_weaker = note_waited_on_must_be_weaker->isChecked();

    return newParams;
}

