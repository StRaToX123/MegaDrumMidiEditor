#include "midinotescontainer.h"

MidiNotesContainer::MidiNotesContainer(QWidget *parent) : QWidget(parent)
{
}

MidiNotesContainer::~MidiNotesContainer()
{
}

void MidiNotesContainer::Initialize(QWidget* tabsContainer)
{
    this->setObjectName("MidiNotesContainer");
    do_once = false;
    active_tab = 0;
    current_selected_tab = 0;

    scroll_bar = new ScrollBar(this, 0, 0, 0, 0.4f, 255, 255, 255, 0.4f, 3);
    scroll_bar->setSpaceBetweenItems(15);

    vertical_line = new QFrame(this);
    vertical_line->setFrameShape(QFrame::VLine);

    rule_types = new RuleTypes(this);

    // Error overlay
    error_overlay = new ErrorOverlay(this);
    error_overlay->hide();

    // Creating the active tab
    tabs_container = tabsContainer;
    tabs_container_left = new QWidget(tabs_container);
    tabs_container_centre = new QWidget(tabs_container);
    tabs_container_right = new QWidget(tabs_container);

    tabs_scroll_offset = 0;
    active_tab_stylesheet += "QWidget#activeTab{ background-color: #3B3B3B;";
    active_tab_stylesheet += "border-color : rgb(255, 255, 255);";
    active_tab_stylesheet += "border-width : 2px;";
    active_tab_stylesheet += "border-style : solid;";
    active_tab_stylesheet += "border-top-left-radius: 10px;";
    active_tab_stylesheet += "border-top-right-radius: 10px;";
    active_tab_stylesheet += "border-bottom-left-radius: 0px;";
    active_tab_stylesheet += "border-bottom-right-radius: 0px;}";

    inactive_selected_tab_stylesheet += "QWidget#inactiveSelectedTab{ background-color: #3B3B3B;";
    inactive_selected_tab_stylesheet += "border-color : rgb(77, 77, 77);";
    inactive_selected_tab_stylesheet += "border-width : 2px;";
    inactive_selected_tab_stylesheet += "border-style : solid;";
    inactive_selected_tab_stylesheet += "border-top-left-radius: 10px;";
    inactive_selected_tab_stylesheet += "border-top-right-radius: 10px;";
    inactive_selected_tab_stylesheet += "border-bottom-left-radius: 0px;";
    inactive_selected_tab_stylesheet += "border-bottom-right-radius: 0px;}";

    inactive_unselected_tab_stylesheet += "QWidget#inactiveUnselectedTab{ background-color: #363636;";
    inactive_unselected_tab_stylesheet += "border-color : rgb(77, 77, 77);";
    inactive_unselected_tab_stylesheet += "border-width : 2px;";
    inactive_unselected_tab_stylesheet += "border-style : solid;";
    inactive_unselected_tab_stylesheet += "border-top-left-radius: 10px;";
    inactive_unselected_tab_stylesheet += "border-top-right-radius: 10px;";
    inactive_unselected_tab_stylesheet += "border-bottom-left-radius: 0px;";
    inactive_unselected_tab_stylesheet += "border-bottom-right-radius: 0px;}";

    //////////////////////////////////////////////////
    active_tab = -1;
    CreateTab(true);
    //////////////////////////////////////////////////

    tabs_scroll_left = new QPushButton(tabs_container_left);
    QSizePolicy newSizePolicy;
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalStretch(0);
    newSizePolicy.setHorizontalStretch(0);

    tabs_scroll_left->setSizePolicy(newSizePolicy);
    tabs_scroll_left->setMinimumSize(STARTER_TAB_SCROLL_BUTTON_WIDTH, STARTER_TAB_SCROLL_BUTTON_WIDTH);
    tabs_scroll_left->setMaximumSize(STARTER_TAB_SCROLL_BUTTON_WIDTH, STARTER_TAB_SCROLL_BUTTON_WIDTH);
    tabs_scroll_left->setText("");
    QIcon scrollLeftIcon(":/Resources/Images/TabsContainer/scroll_left_icon.png");
    tabs_scroll_left->setIcon(scrollLeftIcon);
    tabs_scroll_left->setFlat(true);
    tabs_scroll_left->setAutoRepeat(true);
    tabs_scroll_left->setAutoRepeatInterval(2);
    ///////////////////////////////////////////
    connect(tabs_scroll_left, SIGNAL(clicked()), this, SLOT(TabScrollLeft()));
    ///////////////////////////////////////////

    tabs_scroll_right = new QPushButton(tabs_container_right);
    tabs_scroll_right->setSizePolicy(newSizePolicy);
    tabs_scroll_right->setMinimumSize(STARTER_TAB_SCROLL_BUTTON_WIDTH, STARTER_TAB_SCROLL_BUTTON_WIDTH);
    tabs_scroll_right->setMaximumSize(STARTER_TAB_SCROLL_BUTTON_WIDTH, STARTER_TAB_SCROLL_BUTTON_WIDTH);
    tabs_scroll_right->setText("");
    QIcon scrollRightIcon(":/Resources/Images/TabsContainer/scroll_right_icon.png");
    tabs_scroll_right->setIcon(scrollRightIcon);
    tabs_scroll_right->setFlat(true);
    tabs_scroll_right->setAutoRepeat(true);
    tabs_scroll_right->setAutoRepeatInterval(2);
    ///////////////////////////////////////////
    connect(tabs_scroll_right, SIGNAL(clicked()), this, SLOT(TabScrollRight()));
    ///////////////////////////////////////////
}

void MidiNotesContainer::paintEvent(QPaintEvent* event)
{  
    if(do_once == false)
    {
        do_once = true;
        scroll_bar_width = this->width() * 0.02f;
        starter_tab_container_height = tabs_container->height();
    }

    // move and scale the tab_scroll widgets and buttons
    float tabScrollButtonSize = (tabs_container->height() * STARTER_TAB_SCROLL_BUTTON_WIDTH) / starter_tab_container_height;
    tabs_container_left->setMinimumWidth(tabScrollButtonSize);
    tabs_container_left->setMaximumWidth(tabScrollButtonSize);
    tabs_container_left->setMinimumHeight(tabs_container->height());
    tabs_container_left->setMaximumHeight(tabs_container->height());
    tabs_container_left->move(TABS_CONTAINER_LEFT_AND_RIGHT_MARGIN, 0);

    tabs_scroll_left->setMinimumWidth(tabScrollButtonSize);
    tabs_scroll_left->setMaximumWidth(tabScrollButtonSize);
    tabs_scroll_left->setMinimumHeight(tabScrollButtonSize);
    tabs_scroll_left->setMaximumHeight(tabScrollButtonSize);
    tabs_scroll_left->move(0, (tabs_container->height() - tabScrollButtonSize) / 2);

    float tabsContainerCentreWidth = tabs_container->width() - ((TABS_CONTAINER_LEFT_AND_RIGHT_MARGIN * 2) + (2 * TABS_SPACE_BETWEEN_SCROLL_BUTTON_AND_CENTRE_WIDGET) + (2 * tabScrollButtonSize));
    tabs_container_centre->setMinimumWidth(tabsContainerCentreWidth);
    tabs_container_centre->setMaximumWidth(tabsContainerCentreWidth);
    tabs_container_centre->setMinimumHeight(tabs_container->height());
    tabs_container_centre->setMaximumHeight(tabs_container->height());
    tabs_container_centre->move(TABS_CONTAINER_LEFT_AND_RIGHT_MARGIN + tabScrollButtonSize + TABS_SPACE_BETWEEN_SCROLL_BUTTON_AND_CENTRE_WIDGET, 0);

    tabs_container_right->setMinimumWidth(tabScrollButtonSize);
    tabs_container_right->setMaximumWidth(tabScrollButtonSize);
    tabs_container_right->setMinimumHeight(tabs_container->height());
    tabs_container_right->setMaximumHeight(tabs_container->height());
    tabs_container_right->move(TABS_CONTAINER_LEFT_AND_RIGHT_MARGIN + tabScrollButtonSize + (2 * TABS_SPACE_BETWEEN_SCROLL_BUTTON_AND_CENTRE_WIDGET) + tabsContainerCentreWidth, 0);

    tabs_scroll_right->setMinimumWidth(tabScrollButtonSize);
    tabs_scroll_right->setMaximumWidth(tabScrollButtonSize);
    tabs_scroll_right->setMinimumHeight(tabScrollButtonSize);
    tabs_scroll_right->setMaximumHeight(tabScrollButtonSize);
    tabs_scroll_right->move(0, (tabs_container->height() - tabScrollButtonSize) / 2);

    // Get Scale of the current monitor
    // So that we can  scale tab text boxes appropriately
    monitor_scale_factor = GetUIScalingForMonitorThisAppIsOn();

    // Set tab font sizes
    // This automatically sets each tabs width
    float tabFontSizeBig = (tabs_container->height() * STARTER_TAB_FONT_SIZE_BIG) / starter_tab_container_height;
    float tabFontSizeSmall = (tabs_container->height() * STARTER_TAB_FONT_SIZE_SMALL) / starter_tab_container_height;
    float tabCloseButtonSize = (tabs_container->height() * STARTER_TAB_CLOSE_BUTTON_SIZE) / starter_tab_container_height;
    float tabHeightUnselectedOrInactive = (tabs_container->height() * 90) / 100;
    for(int i = 0; i < tabs.count(); i++)
    {
        if((i != active_tab) && (i != current_selected_tab))
        {
            tabs[i]->SetFontSize(tabFontSizeSmall, monitor_scale_factor); // set fontSize automatically sets the tab width
            tabs[i]->SetHeight(tabHeightUnselectedOrInactive);
        }
        else
        {
            tabs[i]->SetFontSize(tabFontSizeBig, monitor_scale_factor); // set fontSize automatically sets the tab width
            tabs[i]->SetHeight(tabs_container->height());
        }

        tabs[i]->close_button->setMinimumWidth(tabCloseButtonSize);
        tabs[i]->close_button->setMaximumWidth(tabCloseButtonSize);
        tabs[i]->close_button->setMinimumHeight(tabCloseButtonSize);
        tabs[i]->close_button->setMaximumHeight(tabCloseButtonSize);
    }

    // Now move all the tabs into place
    float offsetX = tabs_scroll_offset;
    float lastTabStartingPositionWithoutOffset = 0;
    for(int i = 0; i < tabs.count(); i++)
    {
        if(i == (tabs.count() - 1))
        {
            if(offsetX < 0) // if the last tab went over the left side we need to clamp it
            {
                tabs_scroll_offset = -1 * lastTabStartingPositionWithoutOffset; // clamp the scroll offset to be just the size for the last tab to be on position 0
                offsetX = 0;
            }
        }
        tabs[i]->move(offsetX, tabs_container->height() - tabs[i]->GetHeight());
        offsetX += tabs[i]->GetWidth();
        offsetX += TABS_INBETWEEN_ITEMS_SPACE;
        lastTabStartingPositionWithoutOffset += tabs[i]->GetWidth();
        lastTabStartingPositionWithoutOffset += TABS_INBETWEEN_ITEMS_SPACE;
    }


    float percentOfWidth60 = (this->width() * 60) / 100;
    float rulecontainerSize = this->width() - percentOfWidth60 - 10 - vertical_line->width() + 20;

    vertical_line->move(percentOfWidth60 - 20, 0);
    vertical_line->resize(vertical_line->width(), this->height());

    for(int i = 0; i < per_tab_entry_list[current_selected_tab].count(); i++)
    {
        per_tab_entry_list[current_selected_tab][i]->rules_container->CalculateAllPositionsAndDimensions(rulecontainerSize); // Updates rules_container_height
        scroll_bar->itemPositions[i].SetItemDimension(per_tab_entry_list[current_selected_tab][i]->rules_container->rules_container_height);
        per_tab_entry_list[current_selected_tab][i]->rules_container->SetMinimumAndMaximumHorizontalVertical(rulecontainerSize , per_tab_entry_list[current_selected_tab][i]->rules_container->rules_container_height);
        per_tab_entry_list[current_selected_tab][i]->rules_container->move(percentOfWidth60 + vertical_line->width() - 50, scroll_bar->itemPositions[i].pos_y);
        per_tab_entry_list[current_selected_tab][i]->midi_note->SetMinimumAndMaximumHorizontalVertical(percentOfWidth60, per_tab_entry_list[current_selected_tab][i]->rules_container->rules_container_height);
        per_tab_entry_list[current_selected_tab][i]->midi_note->move(10, scroll_bar->itemPositions[i].pos_y);
    }

    // Set scrollbar item dimensions
    scroll_bar->setDimension(scroll_bar_width, this->height());
    scroll_bar->setPosition(this->width() - scroll_bar_width, 0);

    scroll_bar->paintEvent(event); // FORCED OUT OF NECESSITY SINCE THIS PAINT EVENT WON'T ALWAYS CALL THE SCROLL_BARS PAINT EVENT FOR SOME REASON (THIS COUSES ALLOT OF REPAINT AND PERFORMANCE HITS)

    QWidget::paintEvent(event);
}

void MidiNotesContainer::CreateMidiNote(bool emitRepaint) // Gets called via the titlebar
{
    Entry* newEntry = new Entry;
    newEntry->midi_note = new MidiNote(this);
    newEntry->midi_note->midi_notes_container_entry_list_tab_index = current_selected_tab;
    newEntry->midi_note->midi_notes_container_entry_list_note_index = per_tab_entry_list[current_selected_tab].count();
    ///////////////////////////////////////////////////////////////////
    connect(newEntry->midi_note, SIGNAL(AddRuleRow(int, int)), this, SLOT(AddRuleRow(int, int)));
    connect(newEntry->midi_note, SIGNAL(RemoveRuleRow(int, int)), this, SLOT(RemoveRuleRow(int, int)));
    connect(newEntry->midi_note, SIGNAL(RemoveMidiNote(int, int, bool, bool)), this, SLOT(RemoveMidiNote(int, int, bool, bool)));
    connect(newEntry->midi_note, SIGNAL(MidiNoteModified(int, int)), this, SLOT(MidiNoteModified(int, int)));
    ///////////////////////////////////////////////////////////////////
    newEntry->rules_container = new RulesContainer(this, rule_types);
    newEntry->rules_container->midi_notes_container_entry_list_tab_index = current_selected_tab;
    newEntry->rules_container->midi_notes_container_entry_list_note_index = per_tab_entry_list[current_selected_tab].count();
    ///////////////////////////////////////////////////////////////////
    connect(newEntry->rules_container, SIGNAL(RuleRowRuleCreated(int, int, int, void*)), this, SLOT(RuleRowRuleCreated(int, int, int, void*)));
    connect(newEntry->rules_container, SIGNAL(RuleRowRuleModified(int, int, int, void*)), this, SLOT(RuleRowRuleModified(int, int, int, void*)));
    connect(newEntry->rules_container, SIGNAL(RuleRowRuleRemoved(int, int, int)), this, SLOT(RuleRowRuleRemoved(int, int, int)));
    ///////////////////////////////////////////////////////////////////
    per_tab_entry_list[current_selected_tab].push_back(newEntry);
    scroll_bar->addItemToList();
    newEntry->midi_note->lower(); // so that the error overlay can apear above
    newEntry->rules_container->lower(); // so that the error overlay can apear above
    vertical_line->lower(); // otherwise it will blose the close button
    scroll_bar->list_size_calculated = false;
    if(emitRepaint == true)
    {
       newEntry->midi_note->show();
       newEntry->rules_container->show();
       emit repaint();
    }

    // Telling worker thread to create a new entry in the rule matrix
    emit CreateRuleMatrixEntryOnWorkerThread(current_selected_tab, newEntry->midi_note->note_family, newEntry->midi_note->note_name);
}

void MidiNotesContainer::RemoveMidiNote(int tabIndex, int midiNoteIndex, bool tellWorkerThread, bool applicationExiting) // Gets called via a signal from the midiNote
{
    delete per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container;
    delete per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note;
    delete per_tab_entry_list[tabIndex][midiNoteIndex];
    per_tab_entry_list[tabIndex].removeAt(midiNoteIndex);
    if(applicationExiting == false)
    {
        for(int i = 0; i < per_tab_entry_list[tabIndex].count(); i++)
        {
            per_tab_entry_list[tabIndex][i]->midi_note->midi_notes_container_entry_list_note_index = i;
            per_tab_entry_list[tabIndex][i]->rules_container->midi_notes_container_entry_list_note_index = i;
        }

        // Remove entry from scrollBar
        scroll_bar->RemoveItemPositionAt(midiNoteIndex);

        // Tell worker thread to reamove this entry from it's rule_matrix
        if(tellWorkerThread == true)
        {
            emit RemoveRuleMatrixEntryOnWorkerThread(tabIndex, midiNoteIndex);
            emit repaint();
        }
    }
}

// The BackUp scrollbar data exists because the scrollbar data needs to be backedUp when loading files
// But doesnt needs to be backed up when removing tabs
void MidiNotesContainer::TabClicked(int tabIndex, bool emitRepaint)
{
    // Set StyleSheets
    for(int i = 0; i < tabs.count(); i++)
    {
        if(i != tabIndex)
        {
            if(i != active_tab)
            {
                tabs[i]->setObjectName("inactiveUnselectedTab");
                tabs[i]->setStyleSheet(inactive_unselected_tab_stylesheet);
            }
        }
        else
        {
            if(i != active_tab)
            {
                tabs[i]->setObjectName("inactiveSelectedTab");
                tabs[i]->setStyleSheet(inactive_selected_tab_stylesheet);
            }
        }
    }

    if(emitRepaint == true)
    {
        // Hide all previous midiNotes and show the new ones
        for(int i = 0; i < per_tab_entry_list[current_selected_tab].count(); i++)
        {
            per_tab_entry_list[current_selected_tab][i]->midi_note->hide();
            per_tab_entry_list[current_selected_tab][i]->rules_container->hide();
        }

        scroll_bar->BackUpTab();
    }

    scroll_bar->ActivateTab(tabIndex);
    ////////////////////////////////////////////////////////////////////
    // because we are hiding and showing new midiNotes in this function
    // it will couse a repaint of the midiNotesContainer
    // in the paintEvent is a call to scrollbars paint event which will update all the item positions
    // and repaint the midiNotesContainer once again
    ////////////////////////////////////////////////////////////////////

    for(int i = 0; i < per_tab_entry_list[tabIndex].count(); i++)
    {
        per_tab_entry_list[tabIndex][i]->midi_note->lower();
        per_tab_entry_list[tabIndex][i]->rules_container->lower();


        per_tab_entry_list[tabIndex][i]->midi_note->show();
        per_tab_entry_list[tabIndex][i]->rules_container->show();
    }
    vertical_line->lower();

    ////////////////////////////////////////////////////////////////////
    current_selected_tab = tabIndex;
    ////////////////////////////////////////////////////////////////////

    if(emitRepaint == true)
    {
        emit repaint();
    }
}

void MidiNotesContainer::CreateTab(bool emitRepaint)
{
    tabs.push_back(new Tab(tabs_container_centre));
    tabs[tabs.count() - 1]->tabs_container_index = tabs.count() - 1;
    tabs[tabs.count() - 1]->name = "Tab ";
    tabs[tabs.count() - 1]->name += QString::number(tabs.count());
    connect(tabs[tabs.count() - 1], SIGNAL(TabClicked(int, bool)), this, SLOT(TabClicked(int, bool)));
    connect(tabs[tabs.count() - 1], SIGNAL(RemoveTab(int, bool)), this, SLOT(RemoveTab(int, bool)));
    per_tab_entry_list.push_back(QList<Entry*>());
    scroll_bar->CreateTab();
    // Tell workerThread to create a tab
    emit CreateTabWorkerThread();

    if(tabs.count() == 1)
    {
        TabClicked(0, false); // the tab was just created, no need to backUp its scroll bar data;
        ActivateCurrentTab(true, emitRepaint);
    }
    else
    {
        tabs[tabs.count() - 1]->setObjectName("inactiveUnselectedTab");
        tabs[tabs.count() - 1]->setStyleSheet(inactive_unselected_tab_stylesheet);
    }

    tabs[tabs.count() - 1]->lower();
    tabs[tabs.count() - 1]->show();

    if(emitRepaint == true)
    {
        emit repaint();
    }
}

void MidiNotesContainer::RemoveTab(int tabIndex, bool applicationExiting)
{
    // tell worker_thread to remove the tab
    // this will automaticaly remove all the notes and their rules
    emit RemoveTabWorkerThread(tabIndex);

    // In case there is only one tab available
    // we create a new one first and then delete the previous, we have to do it in this order
    // because the scroll_bar can only remove tabs and activate tabs,
    // as in we have to tell it to which tab to switch to, and during this proccess it will copy the data from the current tab to backup
    // before switching to the new one the current tab obviously needs to be there we cant delete it yet
    // thats why we have to create a new one and then delete the previous

    while(per_tab_entry_list[tabIndex].count() != 0)
    {
        RemoveMidiNote(tabIndex, 0, false, applicationExiting);
    }

    delete tabs[tabIndex];
    tabs.removeAt(tabIndex);
    scroll_bar->RemoveTab(tabIndex);
    per_tab_entry_list.removeAt(tabIndex);

    if(applicationExiting == false)
    {
        // stuff has to be added then deleted in order to avoid out of range on the scrollbar
        if(tabs.count() == 0)
        {
            active_tab = -1;
            CreateTab(false);
            TabClicked(0, false);
            ActivateCurrentTab(false, false); // no need to tell the worker thread, CreateTab already called createTab on the worker_thread wich automatically sets the worker_thread active_tab_index to 0 since there were no tabs once a new one was made
        }
        else
        {
            if(tabIndex == current_selected_tab)
            {
                int newTabIndex = tabIndex;
                if(newTabIndex > (tabs.count() - 1))
                {
                    newTabIndex = tabs.count() - 1;
                }
                TabClicked(newTabIndex, false);
            }
            else
            {
                if(tabIndex < current_selected_tab)
                {
                    current_selected_tab--;
                }
            }

            if(tabIndex == active_tab)
            {
                active_tab = -1;
            }
            else
            {
                if(tabIndex < active_tab)
                {
                    active_tab--;
                }
            }
        }

        for(int i = 0; i < tabs.count(); i++)
        {
            tabs[i]->tabs_container_index = i;
            for(int j = 0; j < per_tab_entry_list[i].count(); j++)
            {
                per_tab_entry_list[i][j]->midi_note->midi_notes_container_entry_list_tab_index = i;
                per_tab_entry_list[i][j]->rules_container->midi_notes_container_entry_list_tab_index = i;
            }
        }

        emit repaint();
    }
}

void MidiNotesContainer::ActivateCurrentTab(bool tellWorkerThread, bool emitRepaint)
{
    if(current_selected_tab != active_tab)
    {
        tabs[current_selected_tab]->setObjectName("activeTab");
        tabs[current_selected_tab]->setStyleSheet(active_tab_stylesheet);
        // active_tab is set to -1 initialy
        if(active_tab >= 0)
        {
            tabs[active_tab]->setObjectName("inactiveUnselectedTab");
            tabs[active_tab]->setStyleSheet(inactive_unselected_tab_stylesheet);
        }
        active_tab = current_selected_tab;

        // Tell WorkerThread to activate the same tab
        if(tellWorkerThread == true)
        {
            emit ActivateTabWorkerThread(active_tab);
        }

        if(emitRepaint == true)
        {
            emit repaint();
        }
    }
}

void MidiNotesContainer::TabScrollLeft()
{
    // the clamping will be done in the paintEvent
    tabs_scroll_offset += TABS_SCROLL_AMOUNT;
    if(tabs_scroll_offset > 0)
    {
        tabs_scroll_offset = 0;
    }

    emit repaint();
}

void MidiNotesContainer::TabScrollRight()
{
    tabs_scroll_offset -= TABS_SCROLL_AMOUNT;

    emit repaint();
}

void MidiNotesContainer::AddRuleRow(int tabIndex, int midiNoteIndex)
{
    per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->AddRuleRow();
}

void MidiNotesContainer::RemoveRuleRow(int tabIndex, int midiNoteIndex)
{
    per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->RemoveRuleRow();
}

void MidiNotesContainer::DisplayErrorMessage(QString message)
{
    error_overlay->SetMessage(message);
    error_overlay->show();
}

void MidiNotesContainer::HideErrorMessage()
{
    error_overlay->hide();
}

void MidiNotesContainer::AddMessageToMidiNote(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage& midiMessage)
{
    per_tab_entry_list[tabIndex][midiNotesContainerEntryListIndex]->midi_note->AddNoteToDisplay(midiMessage);
}

void MidiNotesContainer::MidiNoteModified(int tabIndex, int entryListIndex)
{
    emit ModifyRuleMatrixEntryOnWorkerThread(tabIndex, entryListIndex, per_tab_entry_list[tabIndex][entryListIndex]->midi_note->note_family, per_tab_entry_list[tabIndex][entryListIndex]->midi_note->note_name);
}

void MidiNotesContainer::RuleRowRuleCreated(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule)
{
    emit CreateRuleMatrixRuleOnWorkerThread(tabIndex, ruleMatrixIndex, ruleIndex, workerRule);
}

void MidiNotesContainer::RuleRowRuleModified(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRuleParams)
{
    emit ModifyRuleMatrixRuleOnWorkerThread(tabIndex, ruleMatrixIndex, ruleIndex, workerRuleParams);
}

void MidiNotesContainer::RuleRowRuleRemoved(int tabIndex, int ruleMatrixIndex, int ruleIndex)
{
    emit RemoveRuleMatrixRuleOnWorkerThread(tabIndex, ruleMatrixIndex, ruleIndex);
}
