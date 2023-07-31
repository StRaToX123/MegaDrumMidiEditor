#ifndef RULETYPES_H
#define RULETYPES_H

#include <QObject>
#include "MidiRules/rule.h"
#include "MidiRules/Rules/waitfornoterule.h"
#include "MidiRules/Rules/waitfortimerule.h"
#include "MidiRules/Rules/remaprule.h"
#include "MidiRules/Rules/dropallrule.h"
#include "MidiRules/Rules/velocityrule.h"
#include "MidiRules/Rules/dropnoterule.h"
#include "MidiRules/Rules/activatetabrule.h"
#include "MidiRules/Rules/createmessagerule.h"

/////////////////////////////////////////////////////////////////////////////////////
// This class holds all available rule types in a list //////////////////////////////
// ruletypes.cpp is where you would add or remove a rule type from the application //
/////////////////////////////////////////////////////////////////////////////////////

class RuleTypes : public QObject
{
    Q_OBJECT

    public:
        explicit RuleTypes(QObject *parent = nullptr);
        ~RuleTypes();
        QList<Rule*> types_list;

    signals:

};

#endif // RULETYPES_H
