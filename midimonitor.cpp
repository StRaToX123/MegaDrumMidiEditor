#include "midimonitor.h"

MidiMonitor::MidiMonitor(QObject* parent) : QObject(parent)
{

}

MidiMonitor::~MidiMonitor()
{
    emit MidiMonitorClosed();
}

void MidiMonitor::NumberOfNotesAllowedOnDisplayChanged()
{
    if((number_of_notes_allowed_on_display->text() == "") || (number_of_notes_allowed_on_display->text().toInt() == 0))
    {
        number_of_notes_allowed_on_display_value = 10;
    }
    else
    {
        number_of_notes_allowed_on_display_value = number_of_notes_allowed_on_display->text().toInt();
    }
}

MidiMonitorDialog::MidiMonitorDialog(QWidget* parent) : QDialog(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Set style sheet
    this->setStyleSheet(R"(QDialog {
                           background-color: qradialgradient(cx:0.5, cy:0.5, radius: 1, fx:0.5, fy:0.5, stop:0 rgb(51, 51, 51), stop:1 rgb(64, 64, 64));
                           })");

    this->setMinimumWidth(600);
    this->setMinimumHeight(600);

    midi_monitor = new MidiMonitor(this);

    QWidget* topWidget = new QWidget(this);
    bottom_widget = new QWidget(this);

    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
    newSizePolicy.setHorizontalStretch(100);
    newSizePolicy.setVerticalStretch(5);

    topWidget->setSizePolicy(newSizePolicy);

    newSizePolicy.setVerticalStretch(95);

    bottom_widget->setSizePolicy(newSizePolicy);

    QVBoxLayout* newVBoxLayout = new QVBoxLayout(this);
    QGridLayout* newGridLayout = new QGridLayout(topWidget);

    this->setLayout(newVBoxLayout);
    topWidget->setLayout(newGridLayout);

    newVBoxLayout->addWidget(topWidget);
    newVBoxLayout->addWidget(bottom_widget);

    QLabel* newLabel = new QLabel(topWidget);
    midi_monitor->number_of_notes_allowed_on_display = new QLineEdit(topWidget);

    newLabel->setText("Number of Notes:");
    newLabel->setStyleSheet("color : white;");
    midi_monitor->number_of_notes_allowed_on_display->setText("10");
    midi_monitor->number_of_notes_allowed_on_display->setStyleSheet("background-color: rgb(50, 50, 50); color: white;");
    midi_monitor->number_of_notes_allowed_on_display->setAlignment(Qt::AlignCenter);
    midi_monitor->number_of_notes_allowed_on_display->setValidator(new QIntValidator(0, 1000, topWidget));
   // newPushButton->setText("Set");
    //connect(newPushButton, SIGNAL(clicked()), this, SLOT(NumberOfNotesAllowedOnDisplayChanged()));

    newGridLayout->addWidget(newLabel, 0, 0, Qt::AlignCenter);
    newGridLayout->addWidget(midi_monitor->number_of_notes_allowed_on_display, 0, 1, Qt::AlignCenter);
    //newGridLayout->addWidget(newPushButton, 0, 2, Qt::AlignCenter);

    midi_monitor->number_of_notes_allowed_on_display_value = 10;
    connect(midi_monitor->number_of_notes_allowed_on_display, SIGNAL(editingFinished()), midi_monitor, SLOT(NumberOfNotesAllowedOnDisplayChanged()));
}

void MidiMonitorDialog::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    // Draw the note
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    ////////////////////////////////
    DrawOnScreenNotes();
    ///////////////////////////////

    QDialog::paintEvent(event);
}


void MidiMonitorDialog::AddKnownNoteToMonitor(int noteFamily, QString& noteName, int noteValue, QColor& color)
{
    OnScreenNote* newWidget = new OnScreenNote(bottom_widget);
    newWidget->note_value = noteValue;
    newWidget->top_level_widget = new QWidget(newWidget);
    newWidget->bottom_level_widget = new QWidget(newWidget);
    newWidget->spacer_widget = new QWidget(newWidget);

    // Settings sizing policies
    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);

    newWidget->setSizePolicy(newSizePolicy);
    newWidget->top_level_widget->setSizePolicy(newSizePolicy);
    newWidget->bottom_level_widget->setSizePolicy(newSizePolicy);
    newWidget->spacer_widget->setSizePolicy(newSizePolicy);

    // Settings Layouts
    QGridLayout* topLevelGridLayout = new QGridLayout(newWidget->top_level_widget);
    newWidget->top_level_widget->setLayout(topLevelGridLayout);

    QGridLayout* bottomLevelGridLayout = new QGridLayout(newWidget->bottom_level_widget);
    newWidget->bottom_level_widget->setLayout(bottomLevelGridLayout);

    // Settings content margins
    newWidget->setContentsMargins(0, 0, 0, 0);
    newWidget->spacer_widget->setContentsMargins(0, 0, 0, 0);
    newWidget->top_level_widget->setContentsMargins(0, 0, 0, 0);
    newWidget->bottom_level_widget->setContentsMargins(0, 0, 0, 0);
    topLevelGridLayout->setContentsMargins(0, 0, 0, 0);
    bottomLevelGridLayout->setContentsMargins(0, 0, 0, 0);

    // Adding labels to topLevelGridLayout
    newWidget->top_level_widget_label_note_family = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_name = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_value = new QLabel(newWidget->top_level_widget);

    newWidget->top_level_widget_label_note_family->setText(QString::number(noteFamily));
    newWidget->top_level_widget_label_note_name->setText(noteName);
    newWidget->top_level_widget_label_note_value->setText(QString::number(noteValue));

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

    newWidget->bottom_level_widget_label_note_family->setText(QString::number(noteFamily));
    newWidget->bottom_level_widget_label_note_name->setText(noteName);
    newWidget->bottom_level_widget_label_note_value->setText(QString::number(noteValue));

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
    on_screen_midi_notes.push_back(newWidget);

    newWidget->spacer_widget->show();
    newWidget->top_level_widget->show();
    newWidget->bottom_level_widget->show();
    newWidget->show();

}

MidiMonitorDialog::~MidiMonitorDialog()
{

}

void MidiMonitorDialog::AddUnknownNoteToMonitor(int noteFamily, int noteName, int noteValue)
{
    OnScreenNote* newWidget = new OnScreenNote(bottom_widget);
    newWidget->note_value = noteValue;
    newWidget->top_level_widget = new QWidget(newWidget);
    newWidget->bottom_level_widget = new QWidget(newWidget);
    newWidget->spacer_widget = new QWidget(newWidget);

    // Settings sizing policies
    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);

    newWidget->setSizePolicy(newSizePolicy);
    newWidget->top_level_widget->setSizePolicy(newSizePolicy);
    newWidget->bottom_level_widget->setSizePolicy(newSizePolicy);
    newWidget->spacer_widget->setSizePolicy(newSizePolicy);

    // Settings Layouts
    QGridLayout* topLevelGridLayout = new QGridLayout(newWidget->top_level_widget);
    newWidget->top_level_widget->setLayout(topLevelGridLayout);

    QGridLayout* bottomLevelGridLayout = new QGridLayout(newWidget->bottom_level_widget);
    newWidget->bottom_level_widget->setLayout(bottomLevelGridLayout);

    // Adding labels to topLevelGridLayout
    newWidget->top_level_widget_label_note_family = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_name = new QLabel(newWidget->top_level_widget);
    newWidget->top_level_widget_label_note_value = new QLabel(newWidget->top_level_widget);

    newWidget->top_level_widget_label_note_family->setText(QString::number(noteFamily));
    newWidget->top_level_widget_label_note_name->setText(QString::number(noteName));
    newWidget->top_level_widget_label_note_value->setText(QString::number(noteValue));

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

    newWidget->bottom_level_widget_label_note_family->setText(QString::number(noteFamily));
    newWidget->bottom_level_widget_label_note_name->setText(QString::number(noteName));
    newWidget->bottom_level_widget_label_note_value->setText(QString::number(noteValue));

    newWidget->bottom_level_widget_label_note_family->setAlignment(Qt::AlignCenter);
    newWidget->bottom_level_widget_label_note_name->setAlignment(Qt::AlignCenter);
    newWidget->bottom_level_widget_label_note_value->setAlignment(Qt::AlignCenter);

    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_family, 0, 0, Qt::AlignCenter);
    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_name, 1, 0, Qt::AlignCenter);
    bottomLevelGridLayout->addWidget(newWidget->bottom_level_widget_label_note_value, 2, 0, Qt::AlignCenter);

    // Coloring the newWidget background
    newWidget->bottom_level_widget->setStyleSheet("background-color: rgb(80, 80, 80);");
    newWidget->bottom_level_widget->setAutoFillBackground(true);

    // Add note to list
    on_screen_midi_notes.push_back(newWidget);

    newWidget->spacer_widget->show();
    newWidget->top_level_widget->show();
    newWidget->bottom_level_widget->show();
    newWidget->show();
}


void MidiMonitorDialog::DrawOnScreenNotes()
{
    // Calculate Size of Note on Display
    float noteWidth = (bottom_widget->width() - (2 * BOTTOM_WIDGET_HORIZONTAL_INDENT)) / (midi_monitor->number_of_notes_allowed_on_display_value + 1);
    float spaceBetweenNotes = noteWidth / (midi_monitor->number_of_notes_allowed_on_display_value - 1);

    // Move all the notes into position
    float noteOffsetX = bottom_widget->width() - BOTTOM_WIDGET_HORIZONTAL_INDENT - noteWidth;
    int iEndingValue = on_screen_midi_notes.size() - midi_monitor->number_of_notes_allowed_on_display_value;
    if(iEndingValue < 0)
    {
        iEndingValue = 0;
    }

    for(int i = (on_screen_midi_notes.size() - 1); i >= iEndingValue; i--)
    {
        float bottomLevelHeight = ((float)bottom_widget->height() * (float)on_screen_midi_notes[i]->note_value) / 127.0f;
        float topLevelHeight = (((float)bottom_widget->height() - bottomLevelHeight) * 40.0f) / 100.0f;
        float spacerHeight = 0.0f;
        if(topLevelHeight > 0.0f)
        {
            spacerHeight = (float)bottom_widget->height() - (bottomLevelHeight + topLevelHeight);
        }

        on_screen_midi_notes[i]->setMinimumWidth(noteWidth);
        on_screen_midi_notes[i]->setMaximumWidth(noteWidth);
        on_screen_midi_notes[i]->setMinimumHeight(bottom_widget->height());
        on_screen_midi_notes[i]->setMaximumHeight(bottom_widget->height());

        on_screen_midi_notes[i]->bottom_level_widget->setMinimumWidth(noteWidth);
        on_screen_midi_notes[i]->bottom_level_widget->setMaximumWidth(noteWidth);
        on_screen_midi_notes[i]->bottom_level_widget->setMinimumHeight(bottomLevelHeight);
        on_screen_midi_notes[i]->bottom_level_widget->setMaximumHeight(bottomLevelHeight);

        on_screen_midi_notes[i]->top_level_widget->setMinimumWidth(noteWidth);
        on_screen_midi_notes[i]->top_level_widget->setMaximumWidth(noteWidth);
        on_screen_midi_notes[i]->top_level_widget->setMinimumHeight(topLevelHeight);
        on_screen_midi_notes[i]->top_level_widget->setMaximumHeight(topLevelHeight);

        on_screen_midi_notes[i]->spacer_widget->setMinimumWidth(noteWidth);
        on_screen_midi_notes[i]->spacer_widget->setMaximumWidth(noteWidth);
        on_screen_midi_notes[i]->spacer_widget->setMinimumHeight(spacerHeight);
        on_screen_midi_notes[i]->spacer_widget->setMaximumHeight(spacerHeight);

        float bottomLevelOffsetY = (float)bottom_widget->height() - bottomLevelHeight;
        float topLevelOffsetY = (float)bottom_widget->height() - (bottomLevelHeight + topLevelHeight);
        float spacerOffsetY = (float)bottom_widget->height() - (bottomLevelHeight + topLevelHeight + spacerHeight);

        on_screen_midi_notes[i]->bottom_level_widget->move(0, bottomLevelOffsetY);
        on_screen_midi_notes[i]->top_level_widget->move(0, topLevelOffsetY);
        on_screen_midi_notes[i]->spacer_widget->move(0, spacerOffsetY);

        if(on_screen_midi_notes[i]->processed == false)
        {
            ////////////////////////////////////////////////////
            on_screen_midi_notes[i]->processed = true;
            ////////////////////////////////////////////////////

            if(on_screen_midi_notes[i]->note_value < 15)
            {
                delete on_screen_midi_notes[i]->bottom_level_widget_label_note_family;
                delete on_screen_midi_notes[i]->bottom_level_widget_label_note_name;
                delete on_screen_midi_notes[i]->bottom_level_widget_label_note_value;
            }
            else
            {
                delete on_screen_midi_notes[i]->top_level_widget_label_note_family;
                delete on_screen_midi_notes[i]->top_level_widget_label_note_name;
                delete on_screen_midi_notes[i]->top_level_widget_label_note_value;
            }
        }

        // Move the mainNote into place
        on_screen_midi_notes[i]->move(noteOffsetX, 0);
        on_screen_midi_notes[i]->resize(noteWidth, bottom_widget->height());

        noteOffsetX -= spaceBetweenNotes;
        noteOffsetX -= noteWidth;
    }

    for(int i = 0; i < iEndingValue; i++)
    {
        delete on_screen_midi_notes[0];
        on_screen_midi_notes.removeAt(0);
    }
}


