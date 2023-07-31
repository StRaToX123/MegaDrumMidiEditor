#include "midinote.h"

MidiNote::MidiNote(QWidget *parent) : QWidget(parent)
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setObjectName("midiNote");
    color.setRgb(100, 100, 100);
    label_display_name = new QLabel(this);
    label_display_name->setText("<C4>");
    label_display_name->setAlignment(Qt::AlignLeft);
    font_label_display_name = label_display_name->font();
    font_label_display_name.setPointSize(20);
    label_display_name->setFont(font_label_display_name);
    label_display_name->setStyleSheet("color: white;");
    display_name = "<C4>";
    number_of_notes_on_display = 15;
    note_family = 0;
    note_name = 0;

    parent_widget = static_cast<QWidget*>(parent);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mainLayout);


    // Set style sheet
    this->setStyleSheet(R"(QWidget#midiNote {
                           background-color: qradialgradient(cx:0.5, cy:0.5, radius: 1, fx:0.5, fy:0.5, stop:0 rgb(51, 51, 51), stop:1 rgb(64, 64, 64));
                           border-color : rgb(77, 77, 77);
                           border-width : 2px;
                           border-style : solid;
                           border-radius: 15px;
                           })");


    // Adding drop shadow
    QGraphicsDropShadowEffect* newDs = new QGraphicsDropShadowEffect(this);
    newDs->setColor(Qt::black);
    newDs->setBlurRadius(50);
    newDs->setXOffset(0);
    newDs->setYOffset(0);

    this->setGraphicsEffect(newDs);


    // Create top and bottom level and configuration and close button
    QWidget* topWidget = new QWidget(this);
    bottom_widget = new QWidget(this);

    QGridLayout* top_widget_layout_grid = new QGridLayout(topWidget);
    topWidget->setLayout(top_widget_layout_grid);

    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(bottom_widget);

    topWidget->setContentsMargins(13, 0, 13, 0); // we will calculate the placement of all elements
    bottom_widget->setContentsMargins(0, 0, 0, 0);

    topWidget->setStyleSheet("");
    bottom_widget->setStyleSheet("");

    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setHorizontalStretch(100);
    this->setSizePolicy(newSizePolicy);
    topWidget->setSizePolicy(newSizePolicy);
    bottom_widget->setSizePolicy(newSizePolicy);

    topWidget->setMinimumHeight(TOP_WIDGET_MIN_AND_MAX_HEIGHT);
    topWidget->setMaximumHeight(TOP_WIDGET_MIN_AND_MAX_HEIGHT);

    QPushButton* configurationButton = new QPushButton(topWidget);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalStretch(0);
    newSizePolicy.setHorizontalStretch(0);
    configurationButton->setSizePolicy(newSizePolicy);
    configurationButton->setMinimumSize(15, 15);
    configurationButton->setMaximumSize(15, 15);
    configurationButton->setText("");
    QIcon configurationIcon(":/Resources/Images/midiNotes/configure_icon.png");
    configurationButton->setIcon(configurationIcon);
    configurationButton->setFlat(true);
    ///////////////////////////////////////////
    connect(configurationButton, SIGNAL(clicked()), this, SLOT(CatchOnClickedConfigurationButton()));
    ///////////////////////////////////////////

    QPushButton* closeButton = new QPushButton(topWidget);
    closeButton->setSizePolicy(newSizePolicy);
    closeButton->setMinimumSize(15, 15);
    closeButton->setMaximumSize(15, 15);
    closeButton->setText("");
    QIcon closeIcon(":/Resources/Images/TitleBar/close_Icon.png");
    closeButton->setIcon(closeIcon);
    closeButton->setFlat(true);
    ///////////////////////////////////////////
    connect(closeButton, SIGNAL(clicked()), this, SLOT(CatchOnClickRemoveMidiNote()));
    ///////////////////////////////////////////

    QPushButton* addRuleRowButton = new QPushButton(topWidget);
    addRuleRowButton->setSizePolicy(newSizePolicy);
    addRuleRowButton->setMinimumSize(15, 15);
    addRuleRowButton->setMaximumSize(15, 15);
    addRuleRowButton->setText("");
    QIcon addRuleButtonIcon(":/Resources/Images/midiNotes/novo_icon.png");
    addRuleRowButton->setIcon(addRuleButtonIcon);
    addRuleRowButton->setFlat(true);
    ///////////////////////////////////////////
    connect(addRuleRowButton, SIGNAL(clicked()), this, SLOT(CatchOnClickedAddRuleRow()));
    ///////////////////////////////////////////

    QPushButton* removeRuleRowButton = new QPushButton(topWidget);
    removeRuleRowButton->setSizePolicy(newSizePolicy);
    removeRuleRowButton->setMinimumSize(15, 15);
    removeRuleRowButton->setMaximumSize(15, 15);
    removeRuleRowButton->setText("");
    QIcon removeRuleButtonIcon(":/Resources/Images/midiNotes/removeRule_icon.png");
    removeRuleRowButton->setIcon(removeRuleButtonIcon);
    removeRuleRowButton->setFlat(true);
    ///////////////////////////////////////////
    connect(removeRuleRowButton, SIGNAL(clicked()), this, SLOT(CatchOnClickedRemoveRuleRow()));
    ///////////////////////////////////////////


    QSpacerItem* newSpacer = new QSpacerItem(400, 15, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    top_widget_layout_grid->addWidget(label_display_name, 0, 0, Qt::AlignCenter);
    top_widget_layout_grid->addItem(newSpacer, 0, 1, 1, 1, Qt::AlignLeft);
    top_widget_layout_grid->addWidget(addRuleRowButton, 0, 2, Qt::AlignCenter);
    top_widget_layout_grid->addWidget(removeRuleRowButton, 0, 3, Qt::AlignCenter);
    top_widget_layout_grid->addWidget(configurationButton, 0, 4, Qt::AlignCenter);
    top_widget_layout_grid->addWidget(closeButton, 0, 5, Qt::AlignCenter);

}

void MidiNote::SetMinimumAndMaximumHorizontalVertical(int newHorizontal, int newVertical)
{
    this->setMinimumWidth(newHorizontal);
    this->setMaximumWidth(newHorizontal);
    this->setMinimumHeight(newVertical);
    this->setMaximumHeight(newVertical);
}

void MidiNote::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    // Draw the note
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    bottom_widget->setMinimumHeight(this->height() - TOP_WIDGET_MIN_AND_MAX_HEIGHT);
    bottom_widget->setMaximumHeight(this->height() - TOP_WIDGET_MIN_AND_MAX_HEIGHT);

    ////////////////////////////////
    DrawOnScreenNotes();
    ///////////////////////////////

    QWidget::paintEvent(event);
}

void MidiNote::CatchOnClickRemoveMidiNote()
{
    emit RemoveMidiNote(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index, true, false);
}

void MidiNote::CatchOnClickedAddRuleRow()
{
    emit AddRuleRow(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index);
}

void MidiNote::CatchOnClickedRemoveRuleRow()
{
    emit RemoveRuleRow(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index);
}

void MidiNote::CatchOnClickedConfigurationButton()
{
    QDialog* newDialog = GetMidiNoteParametersDialog();
    newDialog->exec();
    if(newDialog->result() == QDialog::Accepted)
    {
        note_family = line_edit_note_family->text().toInt();
        note_name = line_edit_note_name->text().toInt();
        display_name = line_edit_display_name->text();
        label_display_name->setText(display_name);
        if(color_copy.isValid())
        {
            color = color_copy;
        }
        emit MidiNoteModified(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index); // tell the MidiNotesContainer that a note has changed it's parameters, so that it may propagate that info to the worker thread
    }
    delete newDialog;
}

QDialog* MidiNote::GetMidiNoteParametersDialog()
{
    QDialog* newDialog = new QDialog(this);
    newDialog->setModal(true);
    newDialog->resize(187, 305);

    QGridLayout* newGridLayout = new QGridLayout(newDialog);
    newDialog->setLayout(newGridLayout);

    // Adding stuff to the dialog
    QLabel* newLabelDisplayName = new QLabel(newDialog);
    QLabel* newLabelNoteFamily = new QLabel(newDialog);
    QLabel* newLabelNoteName = new QLabel(newDialog);
    QLabel* newLabelDisplayColor = new QLabel(newDialog);

    newLabelDisplayName->setText("Display Name:");
    newLabelNoteFamily->setText("Note Family:");
    newLabelNoteName->setText("Note Name:");
    newLabelDisplayColor->setText("Display Color:");

    newGridLayout->addWidget(newLabelDisplayName, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelNoteFamily, 1, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelNoteName, 2, 0, Qt::AlignCenter);
    newGridLayout->addWidget(newLabelDisplayColor, 3, 0, Qt::AlignCenter);

    line_edit_display_name = new QLineEdit(newDialog);
    line_edit_note_family = new QLineEdit(newDialog);
    line_edit_note_name = new QLineEdit(newDialog);
    push_button_color_picker = new QPushButton(newDialog);

    line_edit_display_name->setText(display_name);
    line_edit_note_family->setText(QString::number(note_family));
    line_edit_note_name->setText(QString::number(note_name));
    push_button_color_picker->setText("");
    push_button_color_picker->setObjectName("ColorPicker");
    QString newStyleSheet = "QPushButton#ColorPicker { background-color: rgb(";
    newStyleSheet += QString::number(color.red());
    newStyleSheet += ", ";
    newStyleSheet += QString::number(color.green());
    newStyleSheet += ", ";
    newStyleSheet += QString::number(color.blue());
    newStyleSheet += ")};";
    push_button_color_picker->setStyleSheet(newStyleSheet);
    connect(push_button_color_picker, SIGNAL(clicked()), this, SLOT(OpenColorPicker()));

    line_edit_display_name->setAlignment(Qt::AlignCenter);
    line_edit_note_family->setAlignment(Qt::AlignCenter);
    line_edit_note_name->setAlignment(Qt::AlignCenter);

    auto intValidator = new QIntValidator(0, 1000, newDialog);
    line_edit_note_family->setValidator(intValidator);
    line_edit_note_name->setValidator(intValidator);

    newGridLayout->addWidget(line_edit_display_name, 0, 1, Qt::AlignCenter);
    newGridLayout->addWidget(line_edit_note_family, 1, 1, Qt::AlignCenter);
    newGridLayout->addWidget(line_edit_note_name, 2, 1, Qt::AlignCenter);
    newGridLayout->addWidget(push_button_color_picker, 3, 1, Qt::AlignCenter);

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

void MidiNote::OpenColorPicker()
{
    QColorDialog* newColorDialog = new QColorDialog();
    newColorDialog->setModal(true);
    newColorDialog->setAttribute(Qt::WA_DeleteOnClose);
    color_copy = newColorDialog->getColor(color, this, "Note Display Colot");
    if(color_copy.isValid())
    {
        QString newStyleSheet = "QPushButton#ColorPicker { background-color: rgb(";
        newStyleSheet += QString::number(color_copy.red());
        newStyleSheet += ", ";
        newStyleSheet += QString::number(color_copy.green());
        newStyleSheet += ", ";
        newStyleSheet += QString::number(color_copy.blue());
        newStyleSheet += ")};";
        push_button_color_picker->setStyleSheet(newStyleSheet);
    }
}

void MidiNote::AddNoteToDisplay(MidiMessage& midiMessage)
{
    OnScreenNote* newWidget = new OnScreenNote(bottom_widget);
    newWidget->note_value = (int)midiMessage.message[2];
    newWidget->top_level_widget = new QWidget(newWidget);
    newWidget->bottom_level_widget = new QWidget(newWidget);

    // Settings sizing policies
    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);

    newWidget->setSizePolicy(newSizePolicy);
    newWidget->top_level_widget->setSizePolicy(newSizePolicy);
    newWidget->bottom_level_widget->setSizePolicy(newSizePolicy);

    // Settings Layouts
    QGridLayout* topLevelGridLayout = new QGridLayout(newWidget->top_level_widget);
    newWidget->top_level_widget->setLayout(topLevelGridLayout);

    QGridLayout* bottomLevelGridLayout = new QGridLayout(newWidget->bottom_level_widget);
    newWidget->bottom_level_widget->setLayout(bottomLevelGridLayout);

    // Settings content margins
    newWidget->setContentsMargins(0, 0, 0, 0);
    newWidget->top_level_widget->setContentsMargins(0, 5, 0, 5);
    newWidget->bottom_level_widget->setContentsMargins(0, 5, 0, 5);
    topLevelGridLayout->setContentsMargins(0, 0, 0, 0);
    bottomLevelGridLayout->setContentsMargins(0, 0, 0, 0);

    // Adding labels to topLevelGridLayout
    newWidget->top_level_widget_label_note_family = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_name = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_value = new QLabel(newWidget->top_level_widget);

    newWidget->top_level_widget_label_note_family->setText(QString::number((int)midiMessage.message[0]));
    newWidget->top_level_widget_label_note_name->setText(QString::number((int)midiMessage.message[1]));
    newWidget->top_level_widget_label_note_value->setText(QString::number((int)midiMessage.message[2]));

    newWidget->top_level_widget_label_note_family->setAlignment(Qt::AlignCenter);
    newWidget->top_level_widget_label_note_name->setAlignment(Qt::AlignCenter);
    newWidget->top_level_widget_label_note_value->setAlignment(Qt::AlignCenter);

    topLevelGridLayout->addWidget(newWidget->top_level_widget_label_note_family, 0, 0, Qt::AlignCenter);
    topLevelGridLayout->addWidget(newWidget->top_level_widget_label_note_name, 1, 0, Qt::AlignCenter);
    topLevelGridLayout->addWidget(newWidget->top_level_widget_label_note_value, 2, 0, Qt::AlignCenter);

    // Adding labels to bottomLevelGridLayout
    newWidget->bottom_level_widget_label_note_family = new QLabel(newWidget->bottom_level_widget);
    newWidget->bottom_level_widget_label_note_name = new QLabel(newWidget->bottom_level_widget);
    newWidget->bottom_level_widget_label_note_value = new QLabel(newWidget->bottom_level_widget);

    newWidget->bottom_level_widget_label_note_family->setText(QString::number((int)midiMessage.message[0]));
    newWidget->bottom_level_widget_label_note_name->setText(QString::number((int)midiMessage.message[1]));
    newWidget->bottom_level_widget_label_note_value->setText(QString::number((int)midiMessage.message[2]));

    newWidget->bottom_level_widget_label_note_family->setAlignment(Qt::AlignCenter);
    newWidget->bottom_level_widget_label_note_name->setAlignment(Qt::AlignCenter);
    newWidget->bottom_level_widget_label_note_value->setAlignment(Qt::AlignCenter);

    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_family, 0, 0, Qt::AlignCenter);
    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_name, 1, 0, Qt::AlignCenter);
    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_value, 2, 0, Qt::AlignCenter);

    // Coloring the newWidget background
    QString newWidgetStyleSheet = "background-color: rgb(";
    newWidgetStyleSheet += QString::number(color.red());
    newWidgetStyleSheet += ", ";
    newWidgetStyleSheet += QString::number(color.green());
    newWidgetStyleSheet += ", ";
    newWidgetStyleSheet += QString::number(color.blue());
    newWidgetStyleSheet += ");";

    newWidget->bottom_level_widget->setStyleSheet(newWidgetStyleSheet);
    newWidget->bottom_level_widget->setAutoFillBackground(true);

    // Add note to list
    on_screen_notes.push_back(newWidget);

    //newWidget->spacer_widget->show();
    newWidget->top_level_widget->show();
    newWidget->bottom_level_widget->show();
    newWidget->show();
}

void MidiNote::DrawOnScreenNotes()
{
    // Calculate Size of Note on Display
    float noteWidth = (bottom_widget->width() - 26) / (number_of_notes_on_display + 1);
    float spaceBetweenNotes = noteWidth / (number_of_notes_on_display - 1);

    // Move all the notes into position
    float noteOffsetX = (bottom_widget->width() - 13) - noteWidth;
    int iEndingValue = on_screen_notes.size() - number_of_notes_on_display;
    if(iEndingValue < 0)
    {
        iEndingValue = 0;
    }

    for(int i = (on_screen_notes.size() - 1); i >= iEndingValue; i--)
    {
        float bottomLevelHeight = (((float)bottom_widget->height() - (2 * NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS)) * (float)on_screen_notes[i]->note_value) / 127.0f;
        float topLevelHeight = ((float)bottom_widget->height() - (2 * NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS)) - bottomLevelHeight;

        on_screen_notes[i]->setMinimumWidth(noteWidth);
        on_screen_notes[i]->setMaximumWidth(noteWidth);
        on_screen_notes[i]->setMinimumHeight(bottom_widget->height() - (2 * NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS));
        on_screen_notes[i]->setMaximumHeight(bottom_widget->height() - (2 * NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS));

        on_screen_notes[i]->bottom_level_widget->setMinimumWidth(noteWidth);
        on_screen_notes[i]->bottom_level_widget->setMaximumWidth(noteWidth);
        on_screen_notes[i]->bottom_level_widget->setMinimumHeight(bottomLevelHeight);
        on_screen_notes[i]->bottom_level_widget->setMaximumHeight(bottomLevelHeight);

        on_screen_notes[i]->top_level_widget->setMinimumWidth(noteWidth);
        on_screen_notes[i]->top_level_widget->setMaximumWidth(noteWidth);
        on_screen_notes[i]->top_level_widget->setMinimumHeight(topLevelHeight);
        on_screen_notes[i]->top_level_widget->setMaximumHeight(topLevelHeight);

        float bottomLevelOffsetY = ((float)bottom_widget->height() - NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS) - bottomLevelHeight;
        float topLevelOffsetY = (float)bottom_widget->height() - NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS - bottomLevelHeight - topLevelHeight;

        on_screen_notes[i]->bottom_level_widget->move(0, bottomLevelOffsetY);
        on_screen_notes[i]->top_level_widget->move(0, topLevelOffsetY);

        if(on_screen_notes[i]->processed == false)
        {
            ////////////////////////////////////////////////////
            on_screen_notes[i]->processed = true;
            ////////////////////////////////////////////////////

            if(on_screen_notes[i]->note_value < 50)
            {
                delete on_screen_notes[i]->bottom_level_widget_label_note_family;
                delete on_screen_notes[i]->bottom_level_widget_label_note_name;
                delete on_screen_notes[i]->bottom_level_widget_label_note_value;

                on_screen_notes[i]->bottom_level_labels_deleted = true;
            }
            else
            {
                delete on_screen_notes[i]->top_level_widget_label_note_family;
                delete on_screen_notes[i]->top_level_widget_label_note_name;
                delete on_screen_notes[i]->top_level_widget_label_note_value;

                on_screen_notes[i]->top_level_labels_deleted = true;
            }
        }

        // Move the mainNote into place
        on_screen_notes[i]->move(noteOffsetX, 0);
        on_screen_notes[i]->resize(noteWidth, bottom_widget->height() - (2 * NOTES_ON_DISPLAY_TOP_AND_BOTTOM_MARGINS));

        // Calculate new font size
        float newFontPointSize = (6.0f * noteWidth) / 27.0f;
        if(on_screen_notes[i]->top_level_labels_deleted == false)
        {
            QFont defaultFont(on_screen_notes[i]->top_level_widget_label_note_family->font());
            defaultFont.setPointSize(newFontPointSize);
            on_screen_notes[i]->top_level_widget_label_note_family->setFont(defaultFont);
            on_screen_notes[i]->top_level_widget_label_note_name->setFont(defaultFont);
            on_screen_notes[i]->top_level_widget_label_note_value->setFont(defaultFont);
        }
        if(on_screen_notes[i]->bottom_level_labels_deleted == false)
        {
            QFont defaultFont(on_screen_notes[i]->bottom_level_widget_label_note_family->font());
            defaultFont.setPointSize(newFontPointSize);
            on_screen_notes[i]->bottom_level_widget_label_note_family->setFont(defaultFont);
            on_screen_notes[i]->bottom_level_widget_label_note_name->setFont(defaultFont);
            on_screen_notes[i]->bottom_level_widget_label_note_value->setFont(defaultFont);
        }

        noteOffsetX -= spaceBetweenNotes;
        noteOffsetX -= noteWidth;
    }

    for(int i = 0; i < iEndingValue; i++)
    {
        delete on_screen_notes[0];
        on_screen_notes.removeAt(0);
    }
}
