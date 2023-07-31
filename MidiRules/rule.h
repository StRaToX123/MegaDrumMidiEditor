#ifndef RULE_H
#define RULE_H

#include <QString>
#include <QDialog>
#include "workerthread.h"

class ParameterDialog : public QDialog
{
    public:
        ParameterDialog(QWidget* parent = nullptr) : QDialog(parent){};
        virtual ~ParameterDialog(){};
        virtual void* GetDialogOutPut() = 0;
};

class WorkerRule // subclass this class
{
    public:
        explicit WorkerRule(){};
        virtual ~WorkerRule(){};

        // performs the chech to see if this rule has passed
        // returns true or false based on the rule evaluation
        virtual void Check(QueueEntryArray*& queuedNotes,
                               int& queueCurrentNoteIndex,
                                   std::vector<MidiMessage>& midiMessagesPerNotePerRuleMatrixEntryArray,
                                       int midiMessagesPerNotePerMatrixEntryArrayIndexOffset,
                                           qint64& workerThreadTime) = 0; // called when the workerThread is checking this rule

        virtual void UpdateWorkerParams(void* params) = 0; // called when the ui thread gives the worker new params for the rule
        virtual void Reset() = 0; // in case this rule uses any formes of caching, the workerThread might need to reset it in scertain cases
        virtual void ResetAtIndex(int index, int optional) = 0; // same as reset but for a specific element of something that the rule might be using, usefull if some other rule is trying to delete or mark as processed another rule
};

class Rule
{
    public:
        Rule(int colorR = 0, int colorG = 0, int colorB = 0, QString Name = "NN")
        {
            color_r = colorR;
            color_g = colorG;
            color_b = colorB;
            name = Name;
        };
        virtual ~Rule(){};

        virtual void* GetRuleParams() = 0; // returns that rules specific parameters struct
        virtual void* GetRuleParamsCopy() = 0;
        virtual void SetRuleParams(void* newParams) = 0; // the void* should be a pointer to that rules specific parameters struct
        virtual void SetParamsWithStringList(QStringList& paramsList) = 0; // used when the application is trying to load in params from a file
        virtual QString GetParamsAsString() = 0; // used when the application is trying to save to a file
        virtual ParameterDialog* GetDialogForParams(void* params) = 0;
        virtual Rule* GetNewInstance() = 0;

        // These params are needed for the ui to be able to display this ruleType
        int color_r;
        int color_g;
        int color_b;
        QString name;       

        virtual WorkerRule* GetNewWorkerRule() = 0; // return the subclassed workerRule
};

#endif // RULE_H
