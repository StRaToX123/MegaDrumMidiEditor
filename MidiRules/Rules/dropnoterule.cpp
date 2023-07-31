#include "dropnoterule.h"

DropNoteRule::DropNoteRule(QObject *parent) : QObject(parent), Rule(168, 101, 42, "DN")
{

}

DropNoteRule::~DropNoteRule()
{

}

DropNoteRule::DropNoteWorkerRule::DropNoteWorkerRule()
{

}

DropNoteRule::DropNoteWorkerRule::~DropNoteWorkerRule()
{

}

void DropNoteRule::DropNoteWorkerRule::Check(QueueEntryArray*& queuedNotes,
                                             int& queueCurrentNoteIndex,
                                                 std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                                     int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                                         qint64& workerThreadTime)
{
    queuedNotes->array[queueCurrentNoteIndex].test_pending = false;
    queuedNotes->array[queueCurrentNoteIndex].test_passed = false;
}

void DropNoteRule::DropNoteWorkerRule::UpdateWorkerParams(void *params)
{
    return;
}

void DropNoteRule::DropNoteWorkerRule::Reset()
{
    return;
}

void DropNoteRule::DropNoteWorkerRule::ResetAtIndex(int index, int optional)
{
    return;
}

void* DropNoteRule::GetRuleParams()
{
    return nullptr;
}

void* DropNoteRule::GetRuleParamsCopy()
{
    return nullptr;
}

void DropNoteRule::SetRuleParams(void *newParams)
{
    return;
}

QString DropNoteRule::GetParamsAsString()
{
    return QString();
}

void DropNoteRule::SetParamsWithStringList(QStringList &paramsList)
{
    return;
}

ParameterDialog* DropNoteRule::GetDialogForParams(void* params)
{
    return nullptr;
}

Rule* DropNoteRule::GetNewInstance()
{
    return new DropNoteRule();
}

WorkerRule* DropNoteRule::GetNewWorkerRule()
{
    return new DropNoteWorkerRule();
}
