#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{   
    ui->setupUi(this);

    before_maximize_pos_X = 0;
    before_maximize_pos_Y = 0;
    // Add statusBar for the SizeGrip
    QStatusBar* newStatusBar = new QStatusBar(this);
    ui->centralwidget->layout()->addWidget(newStatusBar);

    // Set backGround Color
    QString mainWindowNewStyle = "QMainWindow {background: ";
    mainWindowNewStyle += MAIN_WINDOW_BACKGROUD_COLOR;
    mainWindowNewStyle += ";}";

    this->setStyleSheet(mainWindowNewStyle);


    QString titleBarNewStyle = "background-color: ";
    titleBarNewStyle += MAIN_WINDOW_BACKGROUD_COLOR;
    titleBarNewStyle += ";";

    ui->tajtlbar->setStyleSheet(titleBarNewStyle);

    // Set TitleBar Icon functionality
    connect(ui->ButtonClose, SIGNAL(clicked()), this, SLOT(ExitApp()));
    connect(ui->ButtonMinimize, SIGNAL(clicked()), this, SLOT(MinimizeApp()));
    connect(ui->ButtonMaximize, SIGNAL(clicked()), this, SLOT(MaximizeApp()));
    // Create a new menuBar
    QMenuBar* bar = new QMenuBar(this);
    QHBoxLayout* horizontalLayOut = qobject_cast<QHBoxLayout*>(ui->tajtlbar->layout());

    QMenu* menu1 = new QMenu("File", bar);
    menu1->addAction("Load", this, SLOT(Load()));
    menu1->addAction("Save", this, SLOT(Save()));
    menu1->addAction("Activate Current Tab", this, SLOT(ActivateCurrentTab()));
    menu1->addAction("Find MegaDrum", this, SLOT(OpenCloseMegaDrum()));
    open_close_mega_drum_action = menu1->actions()[menu1->actions().count() - 1];

    QMenu* menu2 = new QMenu("Add", bar);
    menu2->addAction("New MIDI Note", this, SLOT(CreateNewMidiNote()));
    menu2->addAction("New Tab", this, SLOT(CreateTab()));

    QMenu* menu3 = new QMenu("View", bar);
    menu3->addAction("MIDI Monitor", this, SLOT(OpenMidiMonitor()));

    bar->addMenu(menu1);
    bar->addMenu(menu2);
    bar->addMenu(menu3);

    // Set menuBar StyleSheet
    QString menuBarStyle = R"(QMenuBar::item
                              {
                                 spacing: 3px;
                                 padding: 10px 10px;
                                 color: rgb(255,255,255);
                                 border-radius: 5px;
                              }

                              QMenuBar::item:selected
                              {
                                 background-color: rgb(244,164,96);
                                 color: rgb(0,0,0);
                              }

                              QMenuBar::item:pressed
                              {
                                 background: rgb(128,0,0);
                                 color: rgb(255, 255, 255);
                              })";

    bar->setStyleSheet(menuBarStyle);

    // Set Menu StyleSheet
    QString menuStyle = "QMenu { background-color: ";
    menuStyle += MAIN_WINDOW_BACKGROUD_COLOR;
    menuStyle += "; color: rgb(255, 255, 255);}";

    menu1->setStyleSheet(menuStyle);
    menu2->setStyleSheet(menuStyle);
    menu3->setStyleSheet(menuStyle);


    horizontalLayOut->insertWidget(0, bar);



    /////////////////////////////////
    //////// Worker Thread //////////
    /////////////////////////////////

    worker_thread = new WorkerThread(this);

    connect(worker_thread, SIGNAL(InitializationError(int)), this, SLOT(InitializationError(int)), Qt::QueuedConnection);
    connect(worker_thread, SIGNAL(MidiNoteSent(int, int, MidiMessage)), this, SLOT(MidiNoteSent(int, int, MidiMessage)), Qt::QueuedConnection);
    connect(worker_thread, SIGNAL(RuleWantsToSwitchTab(int)), this, SLOT(WorkerThreadRuleWantsToSwitchTab(int)), Qt::QueuedConnection);
    emit worker_thread->ReInitialize();

    ////////////////////////////////////////////////
    //////// Worker Thread HiHat HardCode //////////
    ////////////////////////////////////////////////

    worker_thread_hihat_harcode = new WorkerThreadHiHatHardCode();
    connect(worker_thread_hihat_harcode, SIGNAL(SendApplicationExitSignal()), this, SLOT(ExitApp()), Qt::ConnectionType::QueuedConnection);

    // Midi Monitor Dialog
    midi_monitor_dialog = nullptr;

    // MidiNotesContainer
    connect(ui->midiNotesContainer, SIGNAL(CreateRuleMatrixEntryOnWorkerThread(int, int, int)), this, SLOT(CreateRuleMatrixEntryOnWorkerThread(int, int, int)));
    connect(ui->midiNotesContainer, SIGNAL(ModifyRuleMatrixEntryOnWorkerThread(int, int, int, int)), this, SLOT(ModifyRuleMatrixEntryOnWorkerThread(int, int, int, int)));
    connect(ui->midiNotesContainer, SIGNAL(RemoveRuleMatrixEntryOnWorkerThread(int, int)), this, SLOT(RemoveRuleMatrixEntryOnWorkerThread(int, int)));
    connect(ui->midiNotesContainer, SIGNAL(CreateRuleMatrixRuleOnWorkerThread(int, int, int, void*)), this, SLOT(CreateRuleMatrixRuleOnWorkerThread(int, int, int, void*)));
    connect(ui->midiNotesContainer, SIGNAL(ModifyRuleMatrixRuleOnWorkerThread(int, int, int, void*)), this, SLOT(ModifyRuleMatrixRuleOnWorkerThread(int, int, int, void*)));
    connect(ui->midiNotesContainer, SIGNAL(RemoveRuleMatrixRuleOnWorkerThread(int, int, int)), this, SLOT(RemoveRuleMatrixRuleOnWorkerThread(int, int, int)));
    connect(ui->midiNotesContainer, SIGNAL(CreateTabWorkerThread()), this, SLOT(CreateTabWorkerThread()));
    connect(ui->midiNotesContainer, SIGNAL(RemoveTabWorkerThread(int)), this, SLOT(RemoveTabWorkerThread(int)));
    connect(ui->midiNotesContainer, SIGNAL(ActivateTabWorkerThread(int)), this, SLOT(ActivateTabWorkerThread(int)));
    ui->midiNotesContainer->Initialize(ui->tabs);
}

MainWindow::~MainWindow()
{
    while(ui->midiNotesContainer->per_tab_entry_list.count() != 0)
    {
        ui->midiNotesContainer->RemoveTab(0, true);
    }

    ui->midiNotesContainer->deleteLater();
    delete worker_thread;
    delete worker_thread_hihat_harcode;
    delete ui;
}

void MainWindow::OpenCloseMegaDrum()
{
    if(worker_thread_initialized == false)
    {
        emit worker_thread->ReInitialize(); // Calling Reinitilize will emit a InitializationError with a success or fail message
    }
    else
    {
        emit worker_thread->CloseMegaDrum();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if(worker_thread_initialized == false)
    {
        if(event->key() == Qt::Key_F5)
        {
            emit worker_thread->ReInitialize(); // Calling Reinitilize will emit a InitializationError with a success or fail message
        }
    }
}

void MainWindow::InitializationError(int errorCode) // It will be called even on success with error code 0
{
    switch(errorCode)
    {
        case -1:
        {
            // Switch the Close MegaDrum QAction to be Find MegaDrum
            open_close_mega_drum_action->setText("Find MegaDrum");
            worker_thread_initialized = false;
            QString message;
            message = "MegaDrum Midi port Closed.";
            ui->midiNotesContainer->DisplayErrorMessage(message);
            break;
        }
        case 0:
        {
            // Switch the Find MegaDrum QAction to be Close MegaDrum
            open_close_mega_drum_action->setText("Close MegaDrum");
            worker_thread_initialized = true;
            ui->midiNotesContainer->HideErrorMessage();
            break;
        }
        case 1:
        {
            // Switch the Close MegaDrum QAction to be Find MegaDrum
            open_close_mega_drum_action->setText("Find MegaDrum");
            worker_thread_initialized = false;
            QString message;
            message = "Could not create MIDI-Out port!\nMake sure all prerequisites were installed.";
            ui->midiNotesContainer->DisplayErrorMessage(message);
            break;
        }
        case 2:
        {
            // Switch the Close MegaDrum QAction to be Find MegaDrum
            open_close_mega_drum_action->setText("Find MegaDrum");
            worker_thread_initialized = false;
            QString message;
            message = "Could not find the MegaDrum Module!\nPress F5 to REFRESH.";
            ui->midiNotesContainer->DisplayErrorMessage(message);
            break;
        }
        case 3:
        {
            // Switch the Close MegaDrum QAction to be Find MegaDrum
            open_close_mega_drum_action->setText("Find MegaDrum");
            worker_thread_initialized = false;
            QString message;
            message = "Could not open rtMidi-OutputPort!\nMake sure all prerequisites were installed.";
            ui->midiNotesContainer->DisplayErrorMessage(message);
            break;
        }
    }
}

void MainWindow::MidiNoteSent(int tabIndex, int midiNotesContainerEntryListIndex, MidiMessage midiMessage)
{
    if(midi_monitor_dialog != nullptr)
    {
        if(midiNotesContainerEntryListIndex == -1) // this means this note isnt covered by any rules, we receive these kind of messages only when a midi monitor dialog is open
        {
            midi_monitor_dialog->AddUnknownNoteToMonitor((int)midiMessage.message[0], (int)midiMessage.message[1], (int)midiMessage.message[2]);
        }
        else
        {
            midi_monitor_dialog->AddKnownNoteToMonitor((int)midiMessage.message[0],
                                                 ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNotesContainerEntryListIndex]->midi_note->display_name,
                                                    (int)midiMessage.message[2],
                                                        ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNotesContainerEntryListIndex]->midi_note->color);
        }
    }

    if(midiNotesContainerEntryListIndex != -1)
    {
         ui->midiNotesContainer->AddMessageToMidiNote(tabIndex, midiNotesContainerEntryListIndex, midiMessage);
    }
}

void MainWindow::ExitApp()
{
    QApplication::exit();
}

void MainWindow::MinimizeApp()
{
    setWindowState(Qt::WindowMinimized);
}

void MainWindow::MaximizeApp()
{
    if(window()->isMaximized() == false)
    {
        before_maximize_size = window()->size();
        before_maximize_pos_X = window()->pos().x();
        before_maximize_pos_Y = window()->pos().y();
        setWindowState(Qt::WindowMaximized);
    }
    else
    {
        setWindowState(Qt::WindowNoState);
        this->window()->setGeometry(before_maximize_pos_X, before_maximize_pos_Y, before_maximize_size.width(), before_maximize_size.height());
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
}

void MainWindow::CreateTab()
{
    ui->midiNotesContainer->CreateTab(true);
}

void MainWindow::ActivateCurrentTab()
{
    ui->midiNotesContainer->ActivateCurrentTab(true, true);
}

void MainWindow::Load()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open paulio File"),
                                                            QDir::currentPath(),
                                                                tr("paulio Files (*.paul)"));

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    // Before we load the data, remove all the notes that already exist
    while(ui->midiNotesContainer->per_tab_entry_list.count() != 0)
    {
        ui->midiNotesContainer->RemoveTab(0, true);
    }

    ui->midiNotesContainer->active_tab = -1;

    QTextStream instream(&file);
    QString wholeFile = instream.readAll();
    file.close();
    QStringList lines = wholeFile.split('\n');
    QString data;
    int currentLine = 3; // skip the active tab index and current selected tab (for now), we will read those at the end

    // Looping for as long as there are tabs to cover
    while(currentLine < lines.count())
    {
        if(lines[currentLine].contains("tab_name") == true)
        {
            ///////////////////////////////////////////////////////////
            // Creating a Tab
            ui->midiNotesContainer->tabs.push_back(new Tab(ui->midiNotesContainer->tabs_container_centre));
            ui->midiNotesContainer->tabs[ui->midiNotesContainer->tabs.count() - 1]->tabs_container_index = ui->midiNotesContainer->tabs.count() - 1;
            connect(ui->midiNotesContainer->tabs[ui->midiNotesContainer->tabs.count() - 1], SIGNAL(TabClicked(int, bool)), ui->midiNotesContainer, SLOT(TabClicked(int, bool)));
            connect(ui->midiNotesContainer->tabs[ui->midiNotesContainer->tabs.count() - 1], SIGNAL(RemoveTab(int, bool)), ui->midiNotesContainer, SLOT(RemoveTab(int, bool)));
            ui->midiNotesContainer->per_tab_entry_list.push_back(QList<MidiNotesContainer::Entry*>());
            ui->midiNotesContainer->scroll_bar->CreateTab();
            // Tell workerThread to create a tab
            CreateTabWorkerThread();

            ui->midiNotesContainer->scroll_bar->BackUpTab();
            ui->midiNotesContainer->scroll_bar->ActivateTab(ui->midiNotesContainer->tabs.count() - 1);
            ui->midiNotesContainer->current_selected_tab = ui->midiNotesContainer->tabs.count() - 1;
            ///////////////////////////////////////////////////////////
            // Get tab name
            data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
            int tabIndex = ui->midiNotesContainer->tabs.count() - 1;
            ui->midiNotesContainer->tabs[tabIndex]->name = data;
            currentLine++;
            bool keep_looping = true;
            while(keep_looping == true)
            {
                if(currentLine >= lines.count())
                {
                    break;
                }
                if(lines[currentLine].contains("rule_matrix"))
                {
                    ui->midiNotesContainer->CreateMidiNote(false);
                    int midiNoteIndex = ui->midiNotesContainer->per_tab_entry_list[tabIndex].count() - 1;
                    currentLine++;
                    data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note->display_name = data;
                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note->label_display_name->setText(data);
                    currentLine++;
                    data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note->note_family = data.toInt();
                    currentLine++;
                    data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note->note_name = data.toInt();
                    currentLine++;
                    QString display_color_r = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    currentLine++;
                    QString display_color_g = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    currentLine++;
                    QString display_color_b = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                    QColor newColor;
                    newColor.setRgb(display_color_r.toInt(), display_color_g.toInt(), display_color_b.toInt());
                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->midi_note->color = newColor;
                    // Tell the worker thread that the midiNote has been modified
                    // so that the rule_matrix gets the new note_family and note_name values
                    emit ui->midiNotesContainer->MidiNoteModified(tabIndex, midiNoteIndex);

                    currentLine++;
                    if(lines[currentLine].contains("rules start"))
                    {
                        currentLine++;
                        while(!lines[currentLine].contains("rules end"))
                        {
                            data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                            // Create a ruleRow
                            ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->AddRuleRow();
                            // Create an activeRule
                            ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->ButtonOnClick(ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list.count() - 1, data.toInt());
                            // Get all the params
                            currentLine++;
                            QStringList paramsList;
                            bool paramsListIsEmpty = true;
                            while(!lines[currentLine].isEmpty())
                            {
                                if(lines[currentLine].contains("rules end"))
                                {
                                    currentLine--;
                                    break;
                                }
                                paramsListIsEmpty = false;
                                data = lines[currentLine].mid(lines[currentLine].indexOf(":") + 2, (lines[currentLine].count() - (lines[currentLine].indexOf(":") + 2)) + 1);
                                paramsList.push_back(data);
                                currentLine++;
                            }
                            // Set those params
                            if(paramsListIsEmpty == false)
                            {
                                ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list[ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list.count() - 1]->active_rule->SetParamsWithStringList(paramsList);
                                // Update worker_thread rule params
                                emit ui->midiNotesContainer->ModifyRuleMatrixRuleOnWorkerThread(tabIndex,
                                            midiNoteIndex,
                                                ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list.count() - 1,
                                                    ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list[ui->midiNotesContainer->per_tab_entry_list[tabIndex][midiNoteIndex]->rules_container->rows_list.count() - 1]->active_rule->GetRuleParamsCopy());
                            }

                            currentLine++;
                        }
                    }
                }
                else
                {
                    if(lines[currentLine].contains("Tab") == true)
                    {
                        break;
                    }
                }

                currentLine++;
            }
        }
        else
        {
            currentLine++;
        }
    }

    // read in the active tab and current selected tab
    data = lines[0].mid(lines[0].indexOf(":") + 2, (lines[0].count() - (lines[0].indexOf(":") + 2)) + 1);
    int selectedTabIndex = data.toInt();
    data = lines[1].mid(lines[1].indexOf(":") + 2, (lines[1].count() - (lines[1].indexOf(":") + 2)) + 1);
    int activeTabIndex = data.toInt();
    // Set StyleSheets
    for(int i = 0; i < ui->midiNotesContainer->tabs.count(); i++)
    {
        if(i != selectedTabIndex)
        {
            if(i != activeTabIndex)
            {
                ui->midiNotesContainer->tabs[i]->setObjectName("inactiveUnselectedTab");
                ui->midiNotesContainer->tabs[i]->setStyleSheet(ui->midiNotesContainer->inactive_unselected_tab_stylesheet);
            }
        }
        else
        {
            if(i != activeTabIndex)
            {
                ui->midiNotesContainer->tabs[i]->setObjectName("inactiveSelectedTab");
                ui->midiNotesContainer->tabs[i]->setStyleSheet(ui->midiNotesContainer->inactive_selected_tab_stylesheet);
            }
        }
    }
    // Hide all previous midiNotes and show the new ones
    for(int i = 0; i < ui->midiNotesContainer->tabs.count(); i++)
    {
        ui->midiNotesContainer->tabs[i]->lower();
        ui->midiNotesContainer->tabs[i]->show();
        for(int j = 0; j < ui->midiNotesContainer->per_tab_entry_list[i].count(); j++)
        {
            ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->hide();
            ui->midiNotesContainer->per_tab_entry_list[i][j]->rules_container->hide();
        }
    }

    ui->midiNotesContainer->scroll_bar->BackUpTab();
    ui->midiNotesContainer->scroll_bar->ActivateTab(selectedTabIndex);

    for(int i = 0; i < ui->midiNotesContainer->per_tab_entry_list[selectedTabIndex].count(); i++)
    {
        ui->midiNotesContainer->per_tab_entry_list[selectedTabIndex][i]->midi_note->lower();
        ui->midiNotesContainer->per_tab_entry_list[selectedTabIndex][i]->rules_container->lower();


        ui->midiNotesContainer->per_tab_entry_list[selectedTabIndex][i]->midi_note->show();
        ui->midiNotesContainer->per_tab_entry_list[selectedTabIndex][i]->rules_container->show();
    }
    ui->midiNotesContainer->vertical_line->lower();

    ////////////////////////////////////////////////////////////////////
    ui->midiNotesContainer->current_selected_tab = selectedTabIndex;
    ////////////////////////////////////////////////////////////////////

    ui->midiNotesContainer->tabs[activeTabIndex]->setObjectName("activeTab");
    ui->midiNotesContainer->tabs[activeTabIndex]->setStyleSheet(ui->midiNotesContainer->active_tab_stylesheet);
    ui->midiNotesContainer->active_tab = activeTabIndex;
    ActivateTabWorkerThread(activeTabIndex);

    ui->midiNotesContainer->emit repaint();
}

void MainWindow::Save()
{   
    QString currentWorkingDirectory = QDir::currentPath();
    currentWorkingDirectory += "/untitled.paul";
    QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"),
                                                            currentWorkingDirectory,
                                                                tr("paulio files (*.paul)"));

    if(fileName.isEmpty() == false)
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&file);
            stream << "active_tab: " << ui->midiNotesContainer->active_tab << "\n";
            stream << "current_selected_tab: " << ui->midiNotesContainer->current_selected_tab << "\n\n";
            for(int i = 0; i < ui->midiNotesContainer->tabs.count(); i++)
            {
                stream << "tab_name: " << ui->midiNotesContainer->tabs[i]->name << "\n";
                for(int j = 0; j < ui->midiNotesContainer->per_tab_entry_list[i].count(); j++)
                {
                    stream << "    rule_matrix entry: " << j << "\n";
                    stream << "        display_name: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->display_name << "\n";
                    stream << "        note_family: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->note_family << "\n";
                    stream << "        note_name: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->note_name << "\n";
                    stream << "        display_color_r: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->color.red() << "\n";
                    stream << "        display_color_g: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->color.green() << "\n";
                    stream << "        display_color_b: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->midi_note->color.blue() << "\n";
                    QList<int> activeRulesIndexes;
                    for(int k = 0; k < ui->midiNotesContainer->per_tab_entry_list[i][j]->rules_container->rows_list.count(); k++)
                    {
                        if(ui->midiNotesContainer->per_tab_entry_list[i][j]->rules_container->rows_list[k]->active_rule != nullptr)
                        {
                            activeRulesIndexes.push_back(k);
                        }
                    }
                    if(activeRulesIndexes.count() != 0)
                    {
                        stream << "        #### rules start ####\n";
                        for(int k = 0; k < activeRulesIndexes.count(); k++)
                        {
                            stream << "            rule_type: " << ui->midiNotesContainer->per_tab_entry_list[i][j]->rules_container->rows_list[activeRulesIndexes[k]]->active_rule_type_index << "\n";
                            QString ruleParams = ui->midiNotesContainer->per_tab_entry_list[i][j]->rules_container->rows_list[activeRulesIndexes[k]]->active_rule->GetParamsAsString();
                            // Indent all the rule params
                            int counter = 0;
                            int start = 0;
                            while(counter < ruleParams.count())
                            {
                                while((ruleParams[counter] != '\n') && (counter < ruleParams.count()))
                                {
                                    counter++;
                                }

                                counter++;
                                ruleParams.insert(start, "            ");
                                counter += 12;
                                start = counter;
                            }

                            if(k < (activeRulesIndexes.count() - 1))
                            {
                                ruleParams += "\n";
                            }
                            stream << ruleParams;
                        }

                        stream << "        #### rules end ####\n \n";
                    }
                    else
                    {
                        stream << "\n";
                    }
                }
            }

            stream << Qt::endl;
            file.close();
        }
    }
}

void MainWindow::CreateNewMidiNote()
{
    ui->midiNotesContainer->CreateMidiNote(true);
}

void MainWindow::OpenMidiMonitor()
{
    midi_monitor_dialog = new MidiMonitorDialog(this);
    midi_monitor_dialog->setModal(false);
    connect(midi_monitor_dialog->midi_monitor, SIGNAL(MidiMonitorClosed()), this, SLOT(MidiMonitorClosed()));
    emit worker_thread->SetSendUiThreadInfoAboutEveryNote(true);
    midi_monitor_dialog->exec();
}

void MainWindow::MidiMonitorClosed()
{
    midi_monitor_dialog = nullptr;
    emit worker_thread->SetSendUiThreadInfoAboutEveryNote(false);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    // Set rounded corners
    if(windowState() != Qt::WindowState::WindowMaximized)
    {
        this->setMask(RoundedRect(this->rect(), 8));
    }
    else
    {
        QRegion region;
        region += this->rect();
        this->setMask(region);
    }

    QMainWindow::paintEvent(event);
}

QRegion MainWindow::RoundedRect(const QRect& rect, int r)
{
    QRegion region;
    // middle and borders
    region += rect.adjusted(r, 0, -r, 0);
    region += rect.adjusted(0, r, 0, -r);
    // top left
    QRect corner(rect.topLeft(), QSize(r*2, r*2));
    region += QRegion(corner, QRegion::Ellipse);
    // top right
    corner.moveTopRight(rect.topRight());
    region += QRegion(corner, QRegion::Ellipse);
    // bottom left
    corner.moveBottomLeft(rect.bottomLeft());
    region += QRegion(corner, QRegion::Ellipse);
    // bottom right
    corner.moveBottomRight(rect.bottomRight());
    region += QRegion(corner, QRegion::Ellipse);
    return region;
}

void MainWindow::CreateRuleMatrixEntryOnWorkerThread(int tabIndex, int noteFamily, int noteName)
{
    emit worker_thread->AddNoteToRuleMatrix(tabIndex, noteFamily, noteName);
}

void MainWindow::ModifyRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex, int newNoteFamily, int newNoteName)
{
    emit worker_thread->ModifyNoteFromRuleMatrix(tabIndex, ruleMatrixIndex, newNoteFamily, newNoteName);
}

void MainWindow::RemoveRuleMatrixEntryOnWorkerThread(int tabIndex, int ruleMatrixIndex)
{
    emit worker_thread->RemoveNoteFromRuleMatrix(tabIndex, ruleMatrixIndex);
}

void MainWindow::CreateRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule)
{
    emit worker_thread->AddRuleToRuleMatrix(tabIndex, ruleMatrixIndex, ruleIndex, workerRule);
}

void MainWindow::ModifyRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int workerRuleIndex, void* workerRuleParams)
{
    emit worker_thread->ModifyRuleFromRuleMatrix(tabIndex, ruleMatrixIndex, workerRuleIndex, workerRuleParams);
}

void MainWindow::RemoveRuleMatrixRuleOnWorkerThread(int tabIndex, int ruleMatrixIndex, int workerRuleIndex)
{
    emit worker_thread->RemoveRuleFromRuleMatrix(tabIndex, ruleMatrixIndex, workerRuleIndex);
}

void MainWindow::CreateTabWorkerThread()
{
    emit worker_thread->CreateTab();
}

void MainWindow::RemoveTabWorkerThread(int tabIndex)
{
    emit worker_thread->RemoveTab(tabIndex);
}

void MainWindow::ActivateTabWorkerThread(int tabIndex)
{
    emit worker_thread->ActivateTab(tabIndex);
}

void MainWindow::WorkerThreadRuleWantsToSwitchTab(int tabIndex)
{
    ui->midiNotesContainer->TabClicked(tabIndex, true);
    ui->midiNotesContainer->ActivateCurrentTab(false, true);
}

