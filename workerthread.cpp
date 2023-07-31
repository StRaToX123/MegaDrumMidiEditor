#include "workerthread.h"

WorkerThread::WorkerThread(QObject *parent) : QObject(parent)
{
    Work* work = new Work();
    //////////////////////////////////////////
    gs_callBackUserData.work = work;
    //////////////////////////////////////////
    work->worker_thread = this;
    work->moveToThread(&thread);
    connect(&thread, &QThread::finished, work, &QObject::deleteLater);
    connect(this, SIGNAL(Operate()), work, SLOT(DoWork()), Qt::QueuedConnection);
    connect(this, SIGNAL(StopLoop()), work, SLOT(StopLoop()), Qt::QueuedConnection);
    connect(work, SIGNAL(ErrorReporting(int)), this, SLOT(ErrorReporting(int)));
    connect(this, SIGNAL(ReInitialize()), work, SLOT(ReInitialize()));
    connect(this, SIGNAL(CloseMegaDrum()), work, SLOT(CloseMegaDrum()), Qt::QueuedConnection);
    connect(work, SIGNAL(AddNoteToQueueSignal(MidiMessage)), work, SLOT(AddNoteToQueueSlot(MidiMessage)), Qt::QueuedConnection);
    connect(this, SIGNAL(AddRuleToRuleMatrix(int, int, int, void*)), work, SLOT(AddRuleToRuleMatrix(int, int, int, void*)), Qt::QueuedConnection);
    connect(this, SIGNAL(RemoveRuleFromRuleMatrix(int, int, int)), work, SLOT(RemoveRuleFromRuleMatrix(int, int, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(ModifyRuleFromRuleMatrix(int, int, int, void*)), work, SLOT(ModifyRuleFromRuleMatrix(int, int, int, void*)), Qt::QueuedConnection);
    connect(this, SIGNAL(AddNoteToRuleMatrix(int, int, int)), work, SLOT(AddNoteToRuleMatrix(int, int, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(RemoveNoteFromRuleMatrix(int, int)), work, SLOT(RemoveNoteFromRuleMatrix(int, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(ModifyNoteFromRuleMatrix(int, int, int, int)), work, SLOT(ModifyNoteFromRuleMatrix(int, int, int, int)), Qt::QueuedConnection);
    connect(this, SIGNAL(SetSendUiThreadInfoAboutEveryNote(bool)), work, SLOT(SetSendUiThreadInfoAboutEveryNote(bool)), Qt::QueuedConnection);
    connect(work, SIGNAL(MidiNoteSentWorkerThread(int, int, MidiMessage*)), this, SLOT(MidiNoteSentWorkerThread(int, int, MidiMessage*)), Qt::DirectConnection);
    connect(this, SIGNAL(CreateTab()), work, SLOT(CreateTab()), Qt::QueuedConnection);
    connect(this, SIGNAL(RemoveTab(int)), work, SLOT(RemoveTab(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(ActivateTab(int)), work, SLOT(ActivateTab(int)), Qt::QueuedConnection);
    connect(work, SIGNAL(RuleWantsToSwitchTabWorkerThread(int)), this, SLOT(RuleWantsToSwitchTabWorkerThread(int)), Qt::DirectConnection);
    connect(work, SIGNAL(SendOutMidiMessageSignal(MidiMessage)), work, SLOT(SendOutMidiMessageSlot(MidiMessage)), Qt::QueuedConnection);


    thread.start();
}

WorkerThread::~WorkerThread()
{
    emit StopLoop();
    thread.quit();
    thread.wait();
}

void WorkerThread::ErrorReporting(int errorCode)
{
    emit InitializationError(errorCode);
}

Work::Work()
{
    rt_midi_in = nullptr;
    midi_out = NULL;
    queued_notes = new QueueEntryArray(this, 100);
    keep_looping = false;
    thread_timer.start();
    queue_entry_id_counter = 0;
    send_ui_thread_info_about_every_note = false;
    // prepare for starter tab
    active_tab_index = -1;
    tab_switch_index = -1;
    ////////////////////////////////////////////
    previous_sent_note_was_ride_bell = false;
    ////////////////////////////////////////////
#ifdef DEBUG_PERFORMANCE
    performance_timer_started = false;
#endif

    midi_messages_per_note_per_rule_matrix_entry_array.reserve(1000);
}

Work::~Work()
{
    if(midi_out != NULL)
    {
        virtualMIDIClosePort(midi_out);
        midi_out = NULL;
    }

    if(rt_midi_in != nullptr)
    {
        delete rt_midi_in;
        rt_midi_in = nullptr;
    }

    /////////////////////////////////
    thread_timer.invalidate();
    /////////////////////////////////

    active_tab_index = -1;
    while(rule_matrix.count() != 0)
    {
        RemoveTab(0);
    }

    delete queued_notes;
}


bool Work::OpenOutputMidiPort(LPVM_MIDI_PORT& midiPort)
{
    QString portOutName = "midiDrum-Out";
    LPCWSTR portNameLPCWSTR = (const wchar_t*) portOutName.utf16();
    midiPort = virtualMIDICreatePortEx2(portNameLPCWSTR, NULL, NULL, 1, NULL);

    if(midiPort == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Work::FindMegaDrum(RtMidiIn*& midiPort)
{
    // Initializing rtMidi (Creating an inputPort)
    midiPort = new RtMidiIn();
    unsigned int nPorts = midiPort->getPortCount();
    if(nPorts == 0)
    {
        delete midiPort;
        midiPort = nullptr;
        return false;
    }

    QString portName;
    bool portIsSet = false;
    for(unsigned int i = 0; i < nPorts; i++)
    {
        try
        {
            portName = QString(midiPort->getPortName(i).c_str());
            //qDebug() << "WORKER: found port " << portName;
        }
        catch(RtMidiError &error)
        {

        }

        if(portName.contains("MegaDrum"))
        {
            try
            {
                midiPort->openPort(i);
                portIsSet = true;
            }
            catch (...)
            {
                delete midiPort;
                midiPort = nullptr;
                return false;
            }

            break;
        }
    }

    if(portIsSet == true)
    {
        midiPort->setCallback(&MegaDrumCallBack, &gs_callBackUserData);
        //midiPort->ignoreTypes(false, false, false);
        return true;
    }
    else
    {
        delete midiPort;
        midiPort = nullptr;
        return false;
    }
}

void Work::Initialize()
{
    bool midiOutSuccess = false;
    bool midiInSuccess = false; // <-------------------  OOOOOOOOOOOOOOOOOOOOO

    if(midi_out == NULL)
    {
        if(!OpenOutputMidiPort(midi_out))
        {
            emit ErrorReporting(1);
        }
        else
        {
            midiOutSuccess = true;
        }
    }
    else
    {
        midiOutSuccess = true;
    }

    if(rt_midi_in == nullptr)
    {
        if(!FindMegaDrum(rt_midi_in))
        {
            emit ErrorReporting(2);
        }
        else
        {
            midiInSuccess = true;
        }
    }
    else
    {
        midiInSuccess = true;
    }

    if((midiOutSuccess == true) && (midiInSuccess == true))
    {
        emit ErrorReporting(0); // success error code
    }
}


void Work::ReInitialize() // Slot called by workerThread ReInitialize Signal
{
    Initialize();
}

void Work::CloseMegaDrum()
{
    if(rt_midi_in != nullptr)
    {
        delete rt_midi_in;
        rt_midi_in = nullptr;
        emit ErrorReporting(-1);
    }
}

void Work::CreateTab()
{
    rule_matrix.push_back(QList<RuleMatrixEntry*>());
    per_tab_map_note_family_and_name_to_rule_matrix_entries_cache.push_back(std::map<std::pair<int, int>, std::pair<int*, int>>());
    if(rule_matrix.count() == 1)
    {
        active_tab_index = 0;
    }
}

void Work::RemoveTab(int tabIndex)
{
    queued_notes->Clear(); // also sets keep_looping to false
    midi_messages_per_note_per_rule_matrix_entry_array.clear();

    for(int i = 0; i < rule_matrix[tabIndex].count(); i++)
    {
        delete rule_matrix[tabIndex][i];
    }
    rule_matrix.removeAt(tabIndex);

    // Iterate over the map using c++11 range based for loop
    for (std::pair<std::pair<int, int>, std::pair<int*, int>> element : per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex])
    {
        delete[] element.second.first;
    }

    per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex].clear();
    per_tab_map_note_family_and_name_to_rule_matrix_entries_cache.removeAt(tabIndex);

    if(tabIndex == active_tab_index)
    {
        active_tab_index = -1;
    }

    active_tab_index = -1;

    // if the last tab has been deleted, then the UI thread
    // will issue another tab create
}

// This is a forced Activation called fom the UI Thread
// this tab switch happens immediately disregarding the any possible tab switches
// set via any rules
// which means it will have to clear out the queue
void Work::ActivateTab(int tabIndex)
{
    queued_notes->Clear(); // also sets keep_looping to false
    midi_messages_per_note_per_rule_matrix_entry_array.clear();
    active_tab_index = tabIndex;
}

bool Work::RequestTabSwitch(int tabIndex)
{
    if((tab_switch_index == -1) && (tabIndex != active_tab_index))
    {
        // Check if the tab the rule is requesting to switch to even exists
        if(tabIndex <= (rule_matrix.count() - 1))
        {
            tab_switch_index = tabIndex;
            queued_notes->array[queued_notes->array.size() - 1].perform_tab_switch_here = true;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


// CallBack happens on an rtmidi thread, so in order to escape having to lock and unlock a mutex for accessing rules
// This call back will only post to the event queue of the worker Thread, by calling a slot on the workerThread that will actually do that
void Work::MegaDrumCallBack(double deltatime, std::vector<unsigned char>* message, void* userData)
{
    if((int)(*(message))[2] != 0)
    {
        MidiMessage midiMessage(std::move((*(message))[0]), std::move((*(message))[1]), std::move((*(message))[2]));

        // Check if we need to send this message to the hardcode hihatWork Thead
        if((int)(*(message))[2] == 1)
        {
            return;
        }

        if((((int)(*(message))[0] == 185)
                && ((int)(*(message))[1] == 4))
                   /* ||
                (((int)(*(message))[0] == 153) && (((int)(*(message))[1] == 8) || ((int)(*(message))[1] == 7)))*/)
        {
            #ifndef HIHAT_REPLAY_SAVED_DATA
                //qDebug() << "we got [0] = " << (int)(*(message))[0] << " [1] = " << (int)(*(message))[1] << " [2] = " << (int)(*(message))[2];
                emit static_cast<WorkHiHatHardCode*>(static_cast<CallBackUserData*>(userData)->workHiHatHardCode)->ProccessNoteSIGNAL(midiMessage);
            #endif
        }
        else
        {
            emit static_cast<Work*>(static_cast<CallBackUserData*>(userData)->work)->AddNoteToQueueSignal(midiMessage);
        }
    }
}

void Work::AddNoteToQueueSlot(MidiMessage midiMessage)
{
#ifdef DEBUG_PERFORMANCE
    if(performance_timer_started == false)
    {
        performance_timer_started = true;
        performance_timer.start();
        performance_timer_start_value = performance_timer.elapsed();
    }
#endif
    if(active_tab_index == -1)
    {
        emit SendOutMidiMessageSignal(midiMessage);

        if(send_ui_thread_info_about_every_note == true)
        {
            emit MidiNoteSentWorkerThread(active_tab_index, -1, &midiMessage); // this emit will happen immediately so event though we're sending over a function parameter, it won't get lost yet
        }
    }
    else
    {
        //qDebug() << "WorkerThread: " << (int)midiMessage.message[2];
        int tabIndex = active_tab_index;
        if(tab_switch_index != -1)
        {
            tabIndex = tab_switch_index;
        }
        // If there is no array maping this kind of note to some rule_matrix entries in the cache, then we will have to make a new one
        auto cacheLookUpResult = per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex].find({(int)midiMessage.message[0], (int)midiMessage.message[1]});
        if(cacheLookUpResult == per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex].end())
        {
            int counter = 0;
            for(int i = 0; i < rule_matrix[tabIndex].count(); i++)
            {
                if((rule_matrix[tabIndex][i]->note_family == (int)midiMessage.message[0]) && (rule_matrix[tabIndex][i]->note_name == (int)midiMessage.message[1]))
                {
                    counter++;
                }
            }

            if(counter != 0) // are there even any rules for this note in the rule_matrix
            {
                int* newArray = new int[counter];
                counter = 0;
                for(int i = 0; i < rule_matrix[tabIndex].count(); i++)
                {
                    if((rule_matrix[tabIndex][i]->note_family == (int)midiMessage.message[0]) && (rule_matrix[tabIndex][i]->note_name == (int)midiMessage.message[1]))
                    {
                        *(newArray + counter) = i;
                        counter++;
                    }
                }

                per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex][{(int)midiMessage.message[0], (int)midiMessage.message[1]}] = {newArray, counter};

                int midiMessagesPerNotePerRuleMatrixEntryArrayIndex = midi_messages_per_note_per_rule_matrix_entry_array.size();
                for(int i = 0; i < counter; i++)
                {
                    midi_messages_per_note_per_rule_matrix_entry_array.emplace_back((int)midiMessage.message[0], (int)midiMessage.message[1], (int)midiMessage.message[2]);
                }

                for(int i = 0; i < counter; i++)
                {
                    rule_matrix[tabIndex][*(newArray + i)]->last_rule_processed_per_note.emplace(std::make_pair(queue_entry_id_counter, 0));
                    rule_matrix[tabIndex][*(newArray + i)]->this_matrix_entry_proccessed_per_note.emplace(std::make_pair(queue_entry_id_counter, false));
                }


                queued_notes->array.emplace_back(
                                                     (int)midiMessage.message[0],
                                                         (int)midiMessage.message[1],
                                                             (int)midiMessage.message[2],
                                                                 midiMessagesPerNotePerRuleMatrixEntryArrayIndex,
                                                                     newArray,
                                                                         counter,
                                                                             queue_entry_id_counter,
                                                                                 false,
                                                                                     false);


                queue_entry_id_counter++;
                if(keep_looping == false)
                {
                   keep_looping = true;
                   emit static_cast<WorkerThread*>(worker_thread)->Operate(); // shequle an Operate to our event queue
                }
            }
            else // if there are no rules for it, we will send it out immediately
            {
               // qDebug() << "WorkerThread: SIGNAL message[0] = " << (int)midiMessage.message[0] << " message[1] = " << (int)midiMessage.message[1] << " message[2] = " << (int)midiMessage.message[2];

                // Since the note wasn't addded to queue, send it over to output
                emit SendOutMidiMessageSignal(midiMessage);


                if(send_ui_thread_info_about_every_note == true)
                {
                    emit MidiNoteSentWorkerThread(tabIndex, -1, &midiMessage); // this emit will happen immediately so event though we're sending over a function parameter, it won't get lost yet
                }
            }
        }
        else
        {
            int midiMessagesPerNotePerRuleMatrixEntryArrayIndex = midi_messages_per_note_per_rule_matrix_entry_array.size();
            for(int i = 0; i < cacheLookUpResult->second.second; i++)
            {
                midi_messages_per_note_per_rule_matrix_entry_array.emplace_back((int)midiMessage.message[0], (int)midiMessage.message[1], (int)midiMessage.message[2]);
            }

            for(int i = 0; i < cacheLookUpResult->second.second; i++)
            {
                rule_matrix[tabIndex][*(cacheLookUpResult->second.first + i)]->last_rule_processed_per_note.emplace(std::make_pair(queue_entry_id_counter, 0));
                rule_matrix[tabIndex][*(cacheLookUpResult->second.first + i)]->this_matrix_entry_proccessed_per_note.emplace(std::make_pair(queue_entry_id_counter, false));
            }

            queued_notes->array.emplace_back(
                                                 (int)midiMessage.message[0],
                                                     (int)midiMessage.message[1],
                                                         (int)midiMessage.message[2],
                                                             midiMessagesPerNotePerRuleMatrixEntryArrayIndex,
                                                                 cacheLookUpResult->second.first,
                                                                     cacheLookUpResult->second.second,
                                                                         queue_entry_id_counter,
                                                                             false,
                                                                                 false);



            queue_entry_id_counter++;
            if(keep_looping == false)
            {
                keep_looping = true;
                emit static_cast<WorkerThread*>(worker_thread)->Operate(); // shequle an Operate to our event queue
            }
        }
    }
}

void Work::DoWork()
{
    if(active_tab_index != -1)
    {
        queue_current_note_index = 0;
        bool ruleMatrixEntriesEvaluatedInARow;
        while(keep_looping == true)
        {
            ruleMatrixEntriesEvaluatedInARow = true;
            if(queued_notes->array.size() > 0)
            {
                // notes in the queue get marked with already_processed if all the rule_matrix entries they foll under have been evaluated
                // but that note is the one where a tab switch needs to occur, however the queue_current_note_index was reset because
                // the note wasn't the first in queue, where it is safe to perform a tab switch
                // thus it was marked with already_processed so that it wouldn't have to go throught the whole loop again
                bool hardCodeResult = false;
                if(queued_notes->array[queue_current_note_index].already_processed == false)
                {
                    int ruleMatrixIndexesArrayLastProccessed = queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_last_proccessed_in_a_row;
                    for(int j = ruleMatrixIndexesArrayLastProccessed; j < queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_length; j++)
                    {
                        // Because rule matrix entries can produce a test_pending result while later entries for the note can get evaluated
                        // the rule_matrix_indexes_array_last_processed_in_a_row will start the loop for that note from before the entries that got evaluated
                        // in order to stop them from getting re-evaluated, we have make a check here
                        if(rule_matrix[active_tab_index][queued_notes->array[queue_current_note_index].rule_matrix_indexes_array[j]]->this_matrix_entry_proccessed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] == true)
                        {
                            continue;
                        }

                        hardCodeResult = false;
                        int ruleMatrixIndex = queued_notes->array[queue_current_note_index].rule_matrix_indexes_array[j];
                        if(rule_matrix[active_tab_index][ruleMatrixIndex]->rules.count() > 0) // are there even any rules set for this note
                        {
                            for(int i = rule_matrix[active_tab_index][ruleMatrixIndex]->last_rule_processed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id]; i < rule_matrix[active_tab_index][ruleMatrixIndex]->rules.count(); i++)
                            {
                                // Call the rules Check function
                                qint64 currentThreadTime = thread_timer.nsecsElapsed();
                                rule_matrix[active_tab_index][ruleMatrixIndex]->rules[i]->Check(queued_notes,
                                                                                  queue_current_note_index,
                                                                                      midi_messages_per_note_per_rule_matrix_entry_array,
                                                                                          (queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index + j),
                                                                                              currentThreadTime);
                                hardCodeResult = RideHardCode();
                                if(hardCodeResult == true)
                                {
                                    break;
                                }
                                else
                                {
                                    if(queued_notes->array[queue_current_note_index].test_pending == true) // if the rule is pending stop processing rules for this note and move on to another
                                    {
                                        rule_matrix[active_tab_index][ruleMatrixIndex]->last_rule_processed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] = i;
                                        ruleMatrixEntriesEvaluatedInARow = false;
                                        break;
                                    }
                                    else
                                    {
                                        if(queued_notes->array[queue_current_note_index].test_passed == true)
                                        {
                                            if(i == (rule_matrix[active_tab_index][ruleMatrixIndex]->rules.count() - 1)) // if this is the last rule in the list of rules for this note, we can send it out
                                            {
                                                emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index + j]);
                                                emit MidiNoteSentWorkerThread(active_tab_index,
                                                                                  ruleMatrixIndex,
                                                                                          &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index + j]);
                                                if(ruleMatrixEntriesEvaluatedInARow == true)
                                                {
                                                    queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_last_proccessed_in_a_row++;
                                                }
                                                rule_matrix[active_tab_index][ruleMatrixIndex]->this_matrix_entry_proccessed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] = true;
                                                queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_total_proccessed++;
                                            }
                                        }
                                        else
                                        {
                                            if(ruleMatrixEntriesEvaluatedInARow == true)
                                            {
                                                queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_last_proccessed_in_a_row++;
                                            }
                                            rule_matrix[active_tab_index][ruleMatrixIndex]->this_matrix_entry_proccessed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] = true;
                                            queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_total_proccessed++;

                                            break;
                                        }
                                    }
                                }
                            }

                            if(hardCodeResult == true)
                            {
                                break;
                            }
                        }
                        else // if there are no rules set for this note, it counts as passed so send it out and notify the ui thread
                        {
                            emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index + j]);
                            emit MidiNoteSentWorkerThread(active_tab_index,
                                                              ruleMatrixIndex,
                                                                      &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index + j]);
                            // Mark this rule_matrix entry as proccessed and move on to the next one
                            // if we're still proccessing rule_matrix entries in a row, we are allowed to advanced this notes counter
                            // otherwise some rule_matrix before this one has requested a test_pending
                            if(ruleMatrixEntriesEvaluatedInARow == true)
                            {
                                queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_last_proccessed_in_a_row++;
                            }
                            // In order to be able to skip over this one in case the rule_matrix_indexes_array for this note
                            // starts from before this rule_matrix entry, we will mark this rule_matrix entry for this note only
                            // as proccessed
                            rule_matrix[active_tab_index][ruleMatrixIndex]->this_matrix_entry_proccessed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] = true;
                            queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_total_proccessed++;
                        }

                        // We have to process events here because some rules might have created new messages with the CreateMessageRule
                        /////////////////////////////////////////////////////////////////////////////////////////////////////////
                        QCoreApplication::processEvents(); // process events for adding or removing or modifying rules
                        /////////////////////////////////////////////////////////////////////////////////////////////////////////

                        // Move on to the next rule_matrix entry for this note
                        // We need to skip over all the ones that were already evaluated
                        j++;
                        while(j < queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_length)
                        {
                            if(rule_matrix[active_tab_index][queued_notes->array[queue_current_note_index].rule_matrix_indexes_array[j]]->this_matrix_entry_proccessed_per_note[queued_notes->array[queue_current_note_index].queue_entry_id] == true)
                            {
                                j++;
                            }
                            else
                            {
                                break;
                            }
                        }
                        j--;

                    }
                }

                if(hardCodeResult == true)
                {
                    continue;
                }

                // Before removing this note from the queue, check if this is where we need to perform a tab switch
                if(queued_notes->array[queue_current_note_index].perform_tab_switch_here == true)
                {
                    // Check if all rule_matrix entries for this queued_note have been proccessed
                    // if so we can remove it, but not before we know if it is safe to perform a tab switch
                    // if the note isnt the first in the queue, tab switching is not safe and even though the note is ripe for removing
                    // we will need to reset the queue, but before we do that mark this note as already processed so that, the thread can skip to this park
                    // upon the next arrival
                    if(queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_total_proccessed == queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_length)
                    {
                        // if the note requering the tab switch is the first note in the queue, as in
                        // now it is safe to do the tab switch, all the previous notes have been proccessed
                        if(queue_current_note_index == 0)
                        {
                            // The destructor of the queueEntry removes all the mapped values in the
                            // midi messages array and in each rule_matrix entry
                            queued_notes->Erase(queue_current_note_index);
                            // !!!!! Switching Tab !!!!!
                            emit RuleWantsToSwitchTabWorkerThread(tab_switch_index);
                            active_tab_index = tab_switch_index;
                            tab_switch_index = -1;

                        }
                        else // otherwise there are still notes in the current tab that need proccessing
                        {
                            queued_notes->array[queue_current_note_index].already_processed = true;
                            queue_current_note_index = -1; // set to -1 because the upcoming loop for advencing the queue_current_note_index is coming up, and it will set it to 0
                        }
                    }
                    else // for example if the note is where we need to perform a tab switch but also this note returned test_pending = true
                    {
                         queue_current_note_index = -1; // set to -1 because the upcoming loop for advencing the queue_current_note_index is coming up, and it will set it to 0
                    }
                }
                else
                {
                    if(queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_total_proccessed == queued_notes->array[queue_current_note_index].rule_matrix_indexes_array_length)
                    {
                        // The destructor of the queueEntry removes all the mapped values in the
                        // midi messages array and in each rule_matrix entry
                        queued_notes->Erase(queue_current_note_index);
                    }
                }

                queue_current_note_index++; // the index gets subtracted by one regardless if a note was removed from the queue, since we are moving from back to front
                if(queue_current_note_index > ((int)queued_notes->array.size() - 1))
                {
                    if(queued_notes->array.size() == 0)
                    {
                        keep_looping = false;
                    }
                    else
                    {
                        queue_current_note_index = 0;
                    }
                }
            }
            else // if there is nothing in the queue go back to the event loop
            {
                keep_looping = false;
            }
        }
    }

#ifdef DEBUG_PERFORMANCE
    if(performance_timer_started == true)
    {
        performance_timer_started = false;
        qDebug() << "WorkerThread : Elapsed Time in miliSeconds : " << performance_timer.elapsed() - performance_timer_start_value;
        performance_timer.invalidate();
    }
#endif
}

bool Work::RideHardCode()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////// HARD CODED RIDE BOW EDGE SEPERATION /////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    int bellIndex = -1;
    if((queued_notes->array[queue_current_note_index].note_family == 153)
            && (queued_notes->array[queue_current_note_index].note_name == 1))
    {
        bellIndex = queue_current_note_index + 1;
        if(bellIndex <= (queued_notes->array.size() - 1))
        {
            if(!((queued_notes->array[bellIndex].note_family == 153)
                    && (queued_notes->array[bellIndex].note_name == 65)))
            {
                emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                emit MidiNoteSentWorkerThread(active_tab_index,
                                                  queued_notes->array[queue_current_note_index].rule_matrix_indexes_array[0],
                                                          &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                previous_sent_note_was_ride_bell = false;
                queued_notes->Erase(queue_current_note_index);
                queue_current_note_index++;
                return true;
            }
        }
        else
        {
            emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index]);
            emit MidiNoteSentWorkerThread(active_tab_index,
                                              queued_notes->array[queue_current_note_index].rule_matrix_indexes_array[0],
                                                      &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[queue_current_note_index].midi_messages_per_note_per_rule_matrix_entry_array_index]);
            previous_sent_note_was_ride_bell = false;
            queued_notes->Erase(queue_current_note_index);
            queue_current_note_index++;
            return true;
        }
    }
    else
    {
        if((queued_notes->array[queue_current_note_index].note_family == 153)
                && (queued_notes->array[queue_current_note_index].note_name == 65))
        {
            bellIndex = queue_current_note_index - 1;
            if(bellIndex >= 0)
            {
                if((queued_notes->array[bellIndex].note_family == 153)
                        && (queued_notes->array[bellIndex].note_name == 1))
                {
                    bellIndex = queue_current_note_index;
                }
                else
                {
                    previous_sent_note_was_ride_bell = false;
                    queued_notes->Erase(queue_current_note_index);
                    queue_current_note_index++;
                    return true;
                }
            }
            else
            {
                previous_sent_note_was_ride_bell = false;
                queued_notes->Erase(queue_current_note_index);
                queue_current_note_index++;
                return true;
            }
        }
    }

    if(bellIndex > 0)
    {
        bool bro1 = false;
        bool bro2 = false;
        bool bro3 = false;
        bool bro4 = false;
        bool bro5 = false;
        bool eraseBell = false;
        // Check if it's value is above 18
        if((queued_notes->array[bellIndex].note_value > 18)
                && (queued_notes->array[bellIndex].note_value != 23))
        {
            int previousNoteIndex = bellIndex - 1;
            if(previousNoteIndex >= 0)
            {
                if(queued_notes->array[bellIndex].note_value >= queued_notes->array[previousNoteIndex].note_value)
                {
                    emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                    emit MidiNoteSentWorkerThread(active_tab_index,
                                                      queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                              &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                    previous_sent_note_was_ride_bell = true;
                    queued_notes->Erase(bellIndex - 1);
                    queued_notes->Erase(bellIndex - 1);
                    queue_current_note_index++;
                    return true;
                }
                else
                {
                    // If the bow that came before the bell is lower than 100 in it's strength than we can continue
                    if(queued_notes->array[previousNoteIndex].note_value < 100)
                    {
                        int valueDifference = queued_notes->array[previousNoteIndex].note_value - queued_notes->array[bellIndex].note_value;
                        if(queued_notes->array[bellIndex].note_value < 23)
                        {
                            if(previous_sent_note_was_ride_bell == true)
                            {
                                if((valueDifference > 23) && (valueDifference < 29))
                                {
                                    emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    emit MidiNoteSentWorkerThread(active_tab_index,
                                                                      queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                                              &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    previous_sent_note_was_ride_bell = true;
                                    queued_notes->Erase(bellIndex - 1);
                                    queued_notes->Erase(bellIndex - 1);
                                    queue_current_note_index++;
                                    return true;
                                }
                                else
                                {
                                    eraseBell = true;
                                    bro1 = true;
                                }
                            }
                            else
                            {
                                if((valueDifference > 23) && (valueDifference < 31))
                                {
                                    emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    emit MidiNoteSentWorkerThread(active_tab_index,
                                                                      queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                                              &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    previous_sent_note_was_ride_bell = true;
                                    queued_notes->Erase(bellIndex - 1);
                                    queued_notes->Erase(bellIndex - 1);
                                    queue_current_note_index++;
                                    return true;
                                }
                                else
                                {
                                    eraseBell = true;
                                    bro1 = true;
                                }
                            }
                        }
                        else
                        {
                            if(previous_sent_note_was_ride_bell == true)
                            {
                                if(valueDifference > 16)
                                {
                                    emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    emit MidiNoteSentWorkerThread(active_tab_index,
                                                                      queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                                              &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    previous_sent_note_was_ride_bell = true;
                                    queued_notes->Erase(bellIndex - 1);
                                    queued_notes->Erase(bellIndex - 1);
                                    queue_current_note_index++;
                                    return true;
                                }
                                else
                                {
                                    eraseBell = true;
                                    bro2 = true;
                                }
                            }
                            else
                            {
                                if(valueDifference > 36)
                                {
                                    emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    emit MidiNoteSentWorkerThread(active_tab_index,
                                                                      queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                                              &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                                    previous_sent_note_was_ride_bell = true;
                                    queued_notes->Erase(bellIndex - 1);
                                    queued_notes->Erase(bellIndex - 1);
                                    queue_current_note_index++;
                                    return true;
                                }
                                else
                                {
                                    eraseBell = true;
                                    bro3 = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                        emit MidiNoteSentWorkerThread(active_tab_index,
                                                          queued_notes->array[bellIndex].rule_matrix_indexes_array[0],
                                                                  &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[previousNoteIndex].midi_messages_per_note_per_rule_matrix_entry_array_index]);
                        previous_sent_note_was_ride_bell = true;
                        queued_notes->Erase(bellIndex - 1);
                        queued_notes->Erase(bellIndex - 1);
                        queue_current_note_index++;
                        return true;
                    }
                }
            }
            else
            {
                eraseBell = true;
                bro4 = true;
            }
        }
        else
        {
            eraseBell = true;
            bro5 = true;
        }


        if(eraseBell == true)
        {
            emit SendOutMidiMessageSignal(midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[bellIndex - 1].midi_messages_per_note_per_rule_matrix_entry_array_index]);
            emit MidiNoteSentWorkerThread(active_tab_index,
                                              queued_notes->array[bellIndex - 1].rule_matrix_indexes_array[0],
                                                      &midi_messages_per_note_per_rule_matrix_entry_array[queued_notes->array[bellIndex - 1].midi_messages_per_note_per_rule_matrix_entry_array_index]);
            previous_sent_note_was_ride_bell = false;
            queued_notes->Erase(bellIndex - 1);
            queued_notes->Erase(bellIndex - 1);
            queue_current_note_index++;
            return true;
        }
    }
    else
    {
        return false;
    }

    // Will never be reached
    return false;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// END OF HARD CODE //////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
}


void WorkerThread::MidiNoteSentWorkerThread(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage* midiMessage)
{
    //qDebug() << "WorkerThread: SLOT message[0] = " << (int)midiMessage->message[0] << " message[1] = " << (int)midiMessage->message[1] << " message[2] = " << (int)midiMessage->message[2];
    emit MidiNoteSent(tabIndex, midiNotesContainerEntryListIndex, *midiMessage);
}

void WorkerThread::RuleWantsToSwitchTabWorkerThread(int tabIndex)
{
    emit RuleWantsToSwitchTab(tabIndex);
}

// It is expected that the noteIndex is not out of range
void Work::AddRuleToRuleMatrix(int tabIndex, int noteIndex, int ruleIndex, void* workerRule)
{
    if(ruleIndex == rule_matrix[tabIndex][noteIndex]->rules.count()) // if we're trying to make a new note
    {
        rule_matrix[tabIndex][noteIndex]->rules.push_back(static_cast<WorkerRule*>(workerRule));
    }
    else // otherwise replace an existing one
    {
        delete rule_matrix[tabIndex][noteIndex]->rules[ruleIndex];
        rule_matrix[tabIndex][noteIndex]->rules[ruleIndex] = static_cast<WorkerRule*>(workerRule);
    }
}

void Work::RemoveRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex)
{
    delete rule_matrix[tabIndex][noteIndex]->rules[workerRuleIndex];
    rule_matrix[tabIndex][noteIndex]->rules.removeAt(workerRuleIndex);
}

void Work::ModifyRuleFromRuleMatrix(int tabIndex, int noteIndex, int workerRuleIndex, void* workerRuleParams)
{
    rule_matrix[tabIndex][noteIndex]->rules[workerRuleIndex]->UpdateWorkerParams(workerRuleParams);
}

void Work::AddNoteToRuleMatrix(int tabIndex, int noteFamily, int noteName)
{
    rule_matrix[tabIndex].push_back(new RuleMatrixEntry(noteFamily, noteName));
    ResetPerTabNoteFamilyAndNameToRuleMatrixEntriesCache(tabIndex);
}

void Work::RemoveNoteFromRuleMatrix(int tabIndex, int noteIndex)
{
    delete rule_matrix[tabIndex][noteIndex];
    rule_matrix[tabIndex].removeAt(noteIndex);
    ResetPerTabNoteFamilyAndNameToRuleMatrixEntriesCache(tabIndex);
}

void Work::ModifyNoteFromRuleMatrix(int tabIndex, int noteIndex, int newNoteFamily, int newNoteName)
{
    rule_matrix[tabIndex][noteIndex]->note_family = newNoteFamily;
    rule_matrix[tabIndex][noteIndex]->note_name = newNoteName;
    ResetPerTabNoteFamilyAndNameToRuleMatrixEntriesCache(tabIndex);
}

void Work::StopLoop()
{
    keep_looping = false;
}

void Work::SetSendUiThreadInfoAboutEveryNote(bool newValue)
{
    send_ui_thread_info_about_every_note = newValue;
}

void Work::ResetPerTabNoteFamilyAndNameToRuleMatrixEntriesCache(int tabIndex)
{
    // Iterate over the map using c++11 range based for loop
    for (std::pair<std::pair<int, int>, std::pair<int*, int>> element : per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex])
    {
        delete[] element.second.first;
    }

    /////////////////////////////////////////////////////////////////////////////
    per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex].clear();
    /////////////////////////////////////////////////////////////////////////////

    if(tabIndex == active_tab_index)
    {
        // Now that the cache has been cleared out
        // Because this can happen during an active do_work loop, meaning the thread will go
        // back to operating on the queue right after this
        // all the notes in there will be needing their rule_matrix indexes arrays
        // we need to go over the queue and replace everyones rule_matrix_indexes_array pointers
        // with newly formed ones
        // First Get a list of all the different note_family and note_name combinations
        QList<std::pair<int, int>> differentPairs;
        for(int i = 0; i < queued_notes->array.size(); i++)
        {
            std::pair<int, int> newPair = std::make_pair(queued_notes->array[i].note_family, queued_notes->array[i].note_name);
            bool alreadyCovered = false;
            for(int j = 0; j < differentPairs.count(); j++)
            {
                if(newPair == differentPairs[j])
                {
                    alreadyCovered = true;
                }
            }

            if(alreadyCovered == false)
            {
                differentPairs.push_back(newPair);
            }
        }

        // Now make a rule_matrix_indexes_array for each unique pair
        for(int i = 0; i < differentPairs.count(); i++)
        {
            int counter = 0;
            for(int j = 0; j < rule_matrix[tabIndex].count(); j++)
            {
                if((rule_matrix[tabIndex][j]->note_family == differentPairs[i].first) && (rule_matrix[tabIndex][j]->note_name == differentPairs[i].second))
                {
                    counter++;
                }
            }

            int* newArray = new int[counter];
            counter = 0;

            for(int j = 0; j < rule_matrix[tabIndex].count(); j++)
            {
                if((rule_matrix[tabIndex][j]->note_family == differentPairs[i].first) && (rule_matrix[tabIndex][j]->note_name == differentPairs[i].second))
                {
                    *(newArray + counter) = j;
                    counter++;
                }
            }

            per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex][{differentPairs[i].first, differentPairs[i].second}] = {newArray, counter};
        }

        // Clear all rule_matrix cached things and rule specific caches
        for(int i = 0; i < rule_matrix[tabIndex].count(); i++)
        {
            rule_matrix[tabIndex][i]->last_rule_processed_per_note.clear();
            rule_matrix[tabIndex][i]->this_matrix_entry_proccessed_per_note.clear();
            for(int j = 0; j < rule_matrix[tabIndex][i]->rules.count(); j++)
            {
                rule_matrix[tabIndex][i]->rules[j]->Reset();
            }
        }

        // Clear all midi message copies

        midi_messages_per_note_per_rule_matrix_entry_array.clear();

        // Now update all the parameters for all the notes currently in the queue
        int index = 0;
        for(int i = 0; i < queued_notes->array.size(); i++)
        {
            auto mapResult = per_tab_map_note_family_and_name_to_rule_matrix_entries_cache[tabIndex].find({queued_notes->array[i].note_family, queued_notes->array[i].note_name});
            queued_notes->array[i].rule_matrix_indexes_array = mapResult->second.first;
            queued_notes->array[i].rule_matrix_indexes_array_length = mapResult->second.second;
            queued_notes->array[i].rule_matrix_indexes_array_total_proccessed = 0;
            queued_notes->array[i].rule_matrix_indexes_array_last_proccessed_in_a_row = 0;
            // Assign new midi message copies and getting the starting index
            queued_notes->array[i].midi_messages_per_note_per_rule_matrix_entry_array_index = index;
            for(int j = 0; j < queued_notes->array[i].rule_matrix_indexes_array_length; j++)
            {
                midi_messages_per_note_per_rule_matrix_entry_array.emplace_back(queued_notes->array[i].note_family, queued_notes->array[i].note_name, queued_notes->array[i].note_value);
                index++;
            }
        }

        ///////////////////////////////////////////////
        queue_current_note_index = 0;
        ///////////////////////////////////////////////
    }
}

void Work::SendOutMidiMessageSlot(MidiMessage midiMessage)
{
    try
    {
        virtualMIDISendData(midi_out, midiMessage.message, 3);
    }
    catch(RtMidiError &error)
    {
       // qDebug() << "WorkerThread: " << QString(error.getMessage().c_str());
    }
}

///////////////////////////////////////////////////////////////////
////////////////// HIHAT WORKER THREAD ////////////////////////////
///////////////////////////////////////////////////////////////////

WorkerThreadHiHatHardCode::WorkerThreadHiHatHardCode(QObject* parent) : QObject(parent)
{
    // We cant be saving and replaying saved data at the same time
    #if defined(HIHAT_SAVE_DATA_FOR_REPLAY) && defined(HIHAT_REPLAY_SAVED_DATA)
        emit SendApplicationExitSignal();
    #endif
    #ifndef QT_DEBUG
        #undef HIHAT_SAVE_DATA_FOR_REPLAY
        #undef HIHAT_REPLAY_SAVED_DATA
    #endif
    WorkHiHatHardCode* workHiHatHardCode = new WorkHiHatHardCode(48, 59, 60000000);
    //////////////////////////////////////////
    gs_callBackUserData.workHiHatHardCode = workHiHatHardCode;
    //////////////////////////////////////////
    workHiHatHardCode->moveToThread(&thread);
    connect(&thread, &QThread::finished, workHiHatHardCode, &QObject::deleteLater);
    connect(this, SIGNAL(StopLoop()), workHiHatHardCode, SLOT(StopLoop()), Qt::ConnectionType::QueuedConnection);
    connect(workHiHatHardCode, SIGNAL(ProccessNoteSIGNAL(MidiMessage)), workHiHatHardCode, SLOT(ProccessNoteSLOT(MidiMessage)), Qt::ConnectionType::QueuedConnection);
    connect(workHiHatHardCode, SIGNAL(Operate()), workHiHatHardCode, SLOT(DoWork()), Qt::ConnectionType::QueuedConnection);

    thread.start();

    // If we're replaying saved data, it would be loaded at this point so it's safe to start of the loop here
    #ifdef HIHAT_REPLAY_SAVED_DATA
        workHiHatHardCode->keep_looping = true;
        emit workHiHatHardCode->Operate();
    #endif
}

WorkerThreadHiHatHardCode::~WorkerThreadHiHatHardCode()
{
    emit StopLoop();
    thread.quit();
    thread.wait();
}

WorkHiHatHardCode::WorkHiHatHardCode(int chickNoteName, int splashNoteName, qint64 chickSplashTimeHoldTreshold)
{
    chick_note_name = chickNoteName;
    splash_note_name = splashNoteName;
    chick_splash_time_hold_treshold = chickSplashTimeHoldTreshold;
    chick_check_is_active = false;
    splash_check_is_active = false;
    queued_notes.reserve(1000);
    pedal_level = 0;
    pedal_level_trying_to_swith_to = -1;
    previous_pedal_level = -1;
    timer.start();
    current_timer_read = 0;
    keep_looping = false;
    reset_notes_towards_switch_count_upon_receiving_current_level_note = false;
    last_note_processed_timer_read = 0;
    last_level_switch_timer_read = 0;
    moment_the_pedal_started_going_down_timer_read = 0;
    //pedal_level_data_array.push_back(PedalLevelData(0, 2, 200000000, 4, 2, 88000000, 57, 7, 185, 4, 127));
    //pedal_level_data_array.push_back(PedalLevelData(3, 4, 250000000, 2, 5, 240000000, 55, 7, 185, 4, 80));
    //pedal_level_data_array.push_back(PedalLevelData(5, 7, 200000000, 2, 2, 110000000, 51, 52, 185, 4, 40));
    //pedal_level_data_array.push_back(PedalLevelData(20, 50, 200000000, 2, 3, 55000000, 49, 50, 185, 4, 0));
    pedal_level_data_array.push_back(PedalLevelData(0, 2, 200000000, 2, 2, 88000000, 57, 7, 185, 4, 127));
    pedal_level_data_array.push_back(PedalLevelData(3, 50, 200000000, 4, 3, 55000000, 49, 50, 185, 4, 0));
    failed_due_to_note_timeout = false;
    failed_due_to_mistakes_count = false;
    failed_due_to_switch_timeout = false;
    level_zero_timeout_switch_successful = false;
    // If we're saving pedal data for debug purposes we need to open up a txt in write mode
    #if defined(HIHAT_SAVE_DATA_FOR_REPLAY)
        QString replayFileFullPath = QDir::currentPath();
        replayFileFullPath += "/";
        replayFileFullPath += HIHAT_REPLAY_DATA_NAME;
        replay_file = new QFile(replayFileFullPath);
        replay_file_text_stream = nullptr;
        if (replay_file->open(QIODevice::WriteOnly))
        {
            replay_file_text_stream = new QTextStream(replay_file);
        }
        qDebug() << "WorkerThread: Created " << replayFileFullPath;
    #endif

    // If were replaying saved pedal data for debug purposes, we need to open up an existing file in read mode
    #ifdef HIHAT_REPLAY_SAVED_DATA
        QString replayFileFullPath = QDir::currentPath();
        replayFileFullPath += "/";
        replayFileFullPath += HIHAT_REPLAY_DATA_NAME;
        replay_file = new QFile(replayFileFullPath);
        replay_file_text_stream = nullptr;
        if(replay_file->open(QIODevice::ReadOnly))
        {
            replay_file_text_stream = new QTextStream(replay_file);
            QString wholeFile = replay_file_text_stream->readAll();
            replay_file->close();
            QStringList wholeFileStringList = wholeFile.split('\n');
            MidiMessage newMidiMessage;
            newMidiMessage.message[0] = (unsigned char)185;
            newMidiMessage.message[1] = (unsigned char)4;
            for(int i = 0; i < wholeFileStringList.count(); i++)
            {
                int indexOfSpace = wholeFileStringList[i].indexOf(' ');
                newMidiMessage.message[2] = (unsigned char)wholeFileStringList[i].mid(0, indexOfSpace).toInt();
                qint64 time = wholeFileStringList[i].mid(indexOfSpace + 1, wholeFileStringList[i].count() - (indexOfSpace + 1)).toLong();
                queued_notes.push_back(newMidiMessage);
                replay_file_data_array.push_back(time);
            }
            delete replay_file_text_stream;
            delete replay_file;
        }
        replay_file_data_array_counter = 0;
    #endif
}

WorkHiHatHardCode::~WorkHiHatHardCode()
{
    timer.invalidate();

    for(int i = 0; i < pedal_level_data_array.count(); i++)
    {
        if(pedal_level_data_array[i].change_levels_data_when_this_is_the_active_level != nullptr)
        {
            delete pedal_level_data_array[i].change_levels_data_when_this_is_the_active_level;
            pedal_level_data_array[i].change_levels_data_when_this_is_the_active_level = nullptr;
        }
    }

    #ifdef HIHAT_SAVE_DATA_FOR_REPLAY
        if(replay_file_text_stream != nullptr)
        {
            (*(replay_file_text_stream)) << endl;
        }

        replay_file->close();
        if(replay_file_text_stream != nullptr)
        {
            delete replay_file_text_stream;
        }
        delete replay_file;
    #endif
}

void WorkHiHatHardCode::StopLoop()
{
    keep_looping = false;
}

void WorkHiHatHardCode::ProccessNoteSLOT(MidiMessage midiMessage)
{
    queued_notes.emplace_back(midiMessage);
    if(keep_looping == false)
    {
        keep_looping = true;
        emit Operate();
    }
}


void WorkHiHatHardCode::DoWork()
{
    static auto SwitchToPedalLevel = [&](int index) -> void {
        if(pedal_level_trying_to_swith_to != -1)
        {
            pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch = 0;
            pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_mistakes_towarads_switch = 0;
            pedal_level_data_array[pedal_level_trying_to_swith_to].switch_start_timer_read = 0;
            pedal_level_data_array[pedal_level_trying_to_swith_to].last_time_a_note_for_this_pedal_level_was_received = 0;
        }

        if(pedal_level_data_array[index].change_levels_data_when_this_is_the_active_level != nullptr)
        {
            pedal_level_data_array[index].change_levels_data_when_this_is_the_active_level->fn();
        }

        // the chick and splash check will need to use the moment_the_pedal_started_going_down_timer_read
        if(index < pedal_level)
        {
            moment_the_pedal_started_going_down_timer_read = 0;
            /*
            if(splash_check_is_active == true)
            {
                if(index == 0)
                {
                    // the more we waited to play a splash the less loud it is
                    qint64 delta;
                    if(level_zero_timeout_switch_successful == true)
                    {
                        delta = last_note_processed_timer_read - last_level_switch_timer_read;
                        level_zero_timeout_switch_successful = false;
                    }
                    else
                    {
                        delta = current_timer_read - last_level_switch_timer_read;
                    }

                    Clamp(delta, 0, pedal_level_data_array[0].switch_timeout);
                    Remap(delta, 0, pedal_level_data_array[0].switch_timeout, 127, 1);
                    emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, splash_note_name, (int)delta));
                    splash_check_is_active = false;
                    chick_check_is_active = false;
                    //qDebug() << "WorkerThread: PLAYING SPLASH @@@@@@@@@@@ << " << (int)delta << "  " << current_timer_read << "  " << last_level_switch_timer_read;
                }
            }
            */
        }
        else
        {
            if(moment_the_pedal_started_going_down_timer_read == 0)
            {
                moment_the_pedal_started_going_down_timer_read = current_timer_read;
            }

            chick_check_is_active = true;
            /*
            if(index == (pedal_level_data_array.count() - 1))
            {
                // i dont want the chick check to happen everytime the pedal_level fluctuates between the two last ones
                if(previous_pedal_level < (pedal_level_data_array.count() - 2))
                {
                    chick_check_is_active = true;
                }        
            }
            else
            {
                if(index == (pedal_level_data_array.count() - 2))
                {
                    splash_check_is_active = true;
                    //qDebug() << "Splash Check Active = TRUE";
                }
            }
            */
        }

        pedal_level_trying_to_swith_to = -1;
        previous_pedal_level = pedal_level;
        pedal_level = index;
        last_level_switch_timer_read = current_timer_read;

        //qDebug() << "WorkerThread: Switched to pedal_level = " << pedal_level;

        // Send over the cc message for the pedal newly reached pedal level
        emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(std::move(pedal_level_data_array[pedal_level].cc_message_family),
                                                                            std::move(pedal_level_data_array[pedal_level].cc_message_name),
                                                                                std::move(pedal_level_data_array[pedal_level].cc_message_value)));
    };

    static auto ResetCountsTowardsPedalLevel= [&]() -> void {
        // Reset that previous levels data first
        pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch = 0;
        pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_mistakes_towarads_switch = 0;
        pedal_level_data_array[pedal_level_trying_to_swith_to].switch_start_timer_read = 0;
        pedal_level_data_array[pedal_level_trying_to_swith_to].last_time_a_note_for_this_pedal_level_was_received = 0;
        // Now set the new level we're trying to swith to
        pedal_level_trying_to_swith_to = -1;
    };

    static auto CountTowardsSwitch = [&]() -> void {
        pedal_level_data_array[pedal_level_trying_to_swith_to].last_time_a_note_for_this_pedal_level_was_received = current_timer_read;
        if(pedal_level_data_array[pedal_level_trying_to_swith_to].switch_start_timer_read == 0)
        {
            pedal_level_data_array[pedal_level_trying_to_swith_to].switch_start_timer_read = current_timer_read;
            pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch++;
        }
        else
        {
            if((current_timer_read - pedal_level_data_array[pedal_level_trying_to_swith_to].switch_start_timer_read) < pedal_level_data_array[pedal_level_trying_to_swith_to].switch_timeout)
            {
                pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch++;
                if(pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch > pedal_level_data_array[pedal_level_trying_to_swith_to].number_of_notes_needed_for_switch)
                {
                    SwitchToPedalLevel(pedal_level_trying_to_swith_to);
                    // qDebug() << "WorkerThread: $$$$$$$$$$$ PEDAL_LEVEL = " << pedal_level << " $$$$$$$$$$$";
                }
            }
            else
            {
                failed_due_to_switch_timeout = true;
                ResetCountsTowardsPedalLevel();
            }
        }
    };

    static auto CountMistakes = [&]() -> void {
        pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_mistakes_towarads_switch++;
        if(pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_mistakes_towarads_switch > pedal_level_data_array[pedal_level_trying_to_swith_to].number_of_allowed_mistakes_towarads_switch)
        {
            failed_due_to_mistakes_count = true;
            ResetCountsTowardsPedalLevel();
        }
    };

    while(keep_looping)
    {
        current_timer_read = timer.nsecsElapsed();

        // If we're trying to replay saved data, this is where we would put recorded notes into the queue when their time arrises
        #ifdef HIHAT_REPLAY_SAVED_DATA
            if(replay_file_data_array_counter == replay_file_data_array.count())
            {
                keep_looping = false;
                break;
            }

            if(replay_file_data_array_counter != 0)
            {
                if(current_timer_read < replay_file_data_array[replay_file_data_array_counter])
                {
                    continue;
                }
            }

            replay_file_data_array_counter++;
        #endif
        if(queued_notes.size() > 0)
        {
            // if the midiMessage is a pedal message
            if((int)queued_notes[0].message[1] == 4)
            {
                #ifdef HIHAT_SAVE_DATA_FOR_REPLAY
                    (*(replay_file_text_stream)) << (int)queued_notes[0].message[2] << " " << current_timer_read << "\n";
                #else
                    for(int i = 0; i < pedal_level_data_array.count(); i++)
                    {
                        // Check under wich pedal_level does this note foll under
                        if(((int)queued_notes[0].message[2] >= pedal_level_data_array[i].lower_threshold)
                                && ((int)queued_notes[0].message[2] <= pedal_level_data_array[i].upper_threshold))
                        {
                            // The first note that wants to change the pedal_level will get locked in untill it ether makes a switch or fails
                            if((pedal_level_trying_to_swith_to == -1) && (i != pedal_level))
                            {
                                pedal_level_trying_to_swith_to = i;
                                CountTowardsSwitch();
                            }
                            else
                            {
                                if(i == pedal_level_trying_to_swith_to)
                                {
                                    // Check if the note comes in under the time limit
                                    if((current_timer_read - pedal_level_data_array[i].last_time_a_note_for_this_pedal_level_was_received) <= pedal_level_data_array[i].time_delta_threshold)
                                    {
                                        CountTowardsSwitch();
                                    }
                                    else // the note came in late, we will need to reset all the counters
                                    {
                                        failed_due_to_note_timeout = true;
                                        ResetCountsTowardsPedalLevel();
                                    }
                                }
                                else
                                {
                                    if(pedal_level_trying_to_swith_to != -1) // if we're in pedal_Level 0, we dont want to count towards anything uppon receiving pedal_level 0 notes
                                    {
                                        CountMistakes();
                                        if((reset_notes_towards_switch_count_upon_receiving_current_level_note == true)
                                                && (i == pedal_level))
                                        {
                                            pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch = 0;
                                        }
                                    }
                                }
                            }



                            last_note_processed_timer_read = current_timer_read;
                            break;
                        }
                    }
                #endif
/*
                #ifdef QT_DEBUG
                    if(pedal_level_trying_to_swith_to == -1)
                    {
                        qDebug() << "WorkerThread: " << (int)queued_notes[0].message[2] << " time: " << current_timer_read << " ^^^^ " << 0 << " ^^^^" << " %%%% " << 0 << " %%%%" << " @@@@ " << pedal_level_trying_to_swith_to << " @@@@ !!!! " << pedal_level << " !!!! failed_due_to: N=" << failed_due_to_note_timeout << " S=" << failed_due_to_switch_timeout << " M=" << failed_due_to_mistakes_count;
                    }
                    else
                    {
                        qDebug() << "WorkerThread: " << (int)queued_notes[0].message[2] << " time: " << current_timer_read << " ^^^^ " << pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_towards_switch << " ^^^^" << " %%%% " << pedal_level_data_array[pedal_level_trying_to_swith_to].note_count_mistakes_towarads_switch << " %%%%" << " @@@@ " << pedal_level_trying_to_swith_to << " @@@@ !!!! " << pedal_level << " !!!! failed_due_to: N=" << failed_due_to_note_timeout << " S=" << failed_due_to_switch_timeout << " M=" << failed_due_to_mistakes_count;
                    }

                    failed_due_to_note_timeout = false;
                    failed_due_to_mistakes_count = false;
                    failed_due_to_switch_timeout = false;
                #endif
*/
            }
            /*
            else
            {
                if((int)queued_notes[0].message[1] == 8)
                {
                   // if((int)queued_notes[0].message[2] > 9)
                   // {
                        //qDebug() << "first [0] = " << (int)queued_notes[0].message[0] << " [1] = " << (int)queued_notes[0].message[1] << " [2] = " << (int)queued_notes[0].message[2];
                        //quint32 value = QRandomGenerator::global()->generate();
                        //value = value % 2;
                        //if(value == 1)
                        //{
                           // emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, 51, std::move(queued_notes[0].message[2])));
                      //  }
                       // else
                        //{
                         //   emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, 49, std::move(queued_notes[0].message[2])));
                        //}

                   // }
                    emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, pedal_level_data_array[pedal_level].edge_hit_note_name, std::move(queued_notes[0].message[2])));
                }
                else
                {
                    //qDebug() << "second [0] = " << (int)queued_notes[0].message[0] << " [1] = " << (int)queued_notes[0].message[1] << " [2] = " << (int)queued_notes[0].message[2];
                    emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, pedal_level_data_array[pedal_level].edge_hit_note_name, std::move(queued_notes[0].message[2])));
                }
                chick_check_is_active = false;
                splash_check_is_active = false;
            }
*/
            ////////////////////////////////////////////////////////
            queued_notes.erase(queued_notes.begin());
            ////////////////////////////////////////////////////////

        } // There are no notes left in the queue
        else
        {
            if(pedal_level == 0)
            {
              //  qDebug() << "WorkerThread: DIDNT GET IN : " << (current_timer_read - last_note_processed_timer_read);
                keep_looping = false;
            }
            else
            {
                // If nothing is received
                // and we're not pedal_level 0, we have to check for a switch to pedal_level 0 timeout
                // If we receive a pedal_level 0 note in the mean time whilst counting towards pedal_level 0 timout
                // that means we can switch to pedal_level 0 immediately (thats taken care off inside of the loop)
                if((current_timer_read - last_note_processed_timer_read) >= pedal_level_data_array[0].switch_timeout)
                {
                    //qDebug() << "WorkerThread: SWITCHING TO LEVEL_ZERO " << current_timer_read;
                    level_zero_timeout_switch_successful = true;
                    //qDebug() << "WorkerThread: WERE INSIDE : " << (current_timer_read - last_note_processed_timer_read);
                    SwitchToPedalLevel(0);
                  //  counting_towards_level_zero_timeout_switch = false;
                }
                //else
                //{
                  // counting_towards_level_zero_timeout_switch = true;
                //}
            }
        }

        // Checking for a chick
        if(chick_check_is_active == true)
        {
            //qDebug() << "WorkerThread: PLAYING Counting towards chick";
            // Check for timeout in order to play a chick
            qint64 delta = current_timer_read - last_level_switch_timer_read;
            if(delta >= chick_splash_time_hold_treshold)
            {
                if(pedal_level == (pedal_level_data_array.count() - 1))
                {
                    delta = (current_timer_read - moment_the_pedal_started_going_down_timer_read) - delta;
                    Clamp(delta, 0, 100000000);
                    Remap(delta, 0, 100000000, 127, 1);
                    emit static_cast<Work*>(gs_callBackUserData.work)->SendOutMidiMessageSignal(MidiMessage(153, chick_note_name, (int)delta));
                    qDebug() << "WorkerThread: PLAYING CHICK !!!!!!!!!!!!!!!!! << " << (int)delta;
                }

                chick_check_is_active = false;
                splash_check_is_active = false;
                //qDebug() << "Splash Check Active = FALSE";
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        QCoreApplication::processEvents(); // Read in possible new notes that might have arrived
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
}

