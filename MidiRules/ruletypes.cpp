#include "ruletypes.h"

RuleTypes::RuleTypes(QObject *parent) : QObject(parent)
{
    types_list.push_back(new WaitForNoteRule(this));
    types_list.push_back(new WaitForTimeRule(this));
    types_list.push_back(new RemapRule(this));
    types_list.push_back(new ActivateTabRule(this));
    types_list.push_back(new CreateMessageRule(this));
    types_list.push_back(new VelocityRule(this));
    types_list.push_back(new DropAllRule(this));
    types_list.push_back(new DropNoteRule(this));
}

RuleTypes::~RuleTypes()
{
    for(int i = 0; i < types_list.count(); i++)
    {
        delete types_list[i];
    }
}
