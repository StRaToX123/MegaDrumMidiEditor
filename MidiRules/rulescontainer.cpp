#include "rulescontainer.h"

MyPushButton::MyPushButton(QWidget* parent) : QWidget(parent)
{

}

void MyPushButton::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit MyPushButtonOnClick(row_index, type_index);
    }
}

void MyPushButton::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);

    QPainter p(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

RulesContainer::RulesContainer(QWidget* parent, RuleTypes* ruleTypes) : QWidget(parent)
{
    this->setObjectName("RulesContainer");

    parent_widget = static_cast<QWidget*>(parent);
    rule_types = ruleTypes;
    selected_row_index = -1;
    last_set_rule_row_index = 0;


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mainLayout);

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);



    // Set style sheet
    this->setStyleSheet(R"(QWidget#RulesContainer {
                           background-color: qradialgradient(cx:0.5, cy:0.5, radius: 1, fx:0.5, fy:0.5, stop:0 rgb(51, 51, 51), stop:1 rgb(64, 64, 64));
                           border-color : rgb(77, 77, 77);
                           border-width : 2px;
                           border-style : solid;
                           border-radius: 15px;
                           })");


    // Create top and bottom level and configuration and close button
    QWidget* topLevelWidget = new QWidget(this);
    QHBoxLayout* topLevelWidgetLayout = new QHBoxLayout();
    QWidget* bottomLevelWidget = new QWidget(this);
    QHBoxLayout* bottomLevelWidgetLayout = new QHBoxLayout();

    topLevelWidgetLayout->setContentsMargins(0, 0, 0, 0);
    bottomLevelWidgetLayout->setContentsMargins(0, 0, 0, 0);

    topLevelWidget->setStyleSheet("");
    bottomLevelWidget->setStyleSheet("");

    QSizePolicy newSizePolicy;
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    newSizePolicy.setVerticalPolicy(QSizePolicy::Preferred);
    newSizePolicy.setVerticalStretch(10);
    this->setSizePolicy(newSizePolicy);


    topLevelWidget->setSizePolicy(newSizePolicy);

    newSizePolicy.setVerticalStretch(90);

    bottomLevelWidget->setSizePolicy(newSizePolicy);

    topLevelWidget->setLayout(topLevelWidgetLayout);
    bottomLevelWidget->setLayout(bottomLevelWidgetLayout);


    mainLayout->addWidget(topLevelWidget);
    mainLayout->addWidget(bottomLevelWidget);
}

RulesContainer::~RulesContainer()
{
    for(int i = 0; i < rows_list.count(); i++)
    {
        if(rows_list[i]->row != nullptr)
        {
            delete rows_list[i]->row; // this will also delete all the buttons
            rows_list[i]->row = nullptr;
        }

        if(rows_list[i]->active_rule != nullptr)
        {
            delete rows_list[i]->active_rule;
            rows_list[i]->active_rule = nullptr;

            // we dont need to tell the worker_thread to remove each rule_row here
            // because if the rulescontainer destructor is being called then that means the whole midiNote is
            // being deleted, and the RemoveMidiNote will tell the worker_thread to remove the whole rule_matrix entry
            // which in it's destructor removes all the rule_rows in one swoop
        }

        if(rows_list[i] != nullptr)
        {
            delete rows_list[i]; // delete the struct
            rows_list[i] = nullptr;
        }
    }
}

void RulesContainer::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    QPen newPen(p.pen());
    newPen.setWidth(6);
    newPen.setColor(Qt::white);
    p.setPen(newPen);

    QFont newFont(p.font());
    newFont.setPointSize(new_font_size);
    p.setFont(newFont);

    // Draw the container
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // Drawing Text
    for(int i = 0; i < rule_types->types_list.count(); i++)
    {
        p.drawText(rule_name_offsets_x[i], RULE_NAME_OFFSET_Y, rule_name_box_width, rule_name_box_height, Qt::AlignHCenter, rule_types->types_list[i]->name);
    }

    for(int i = 0; i < rows_list.count(); i++)
    {
        rows_list[i]->row->setMinimumWidth(this->width());
        rows_list[i]->row->setMaximumWidth(this->width());
        rows_list[i]->row->move(0, rule_row_offsets_y[i]);

        for(int j = 0; j < rows_list[i]->row_buttons.count(); j++)
        {
            rows_list[i]->row_buttons[j]->setMinimumWidth(rule_button_size);
            rows_list[i]->row_buttons[j]->setMaximumWidth(rule_button_size);
            rows_list[i]->row_buttons[j]->setMinimumHeight(rule_button_size);
            rows_list[i]->row_buttons[j]->setMaximumHeight(rule_button_size);
            rows_list[i]->row_buttons[j]->move(rule_button_offsets_x[j], rule_button_offset_y);
        }

    }

    //QWidget::paintEvent(event);
}

void RulesContainer::SetMinimumAndMaximumHorizontalVertical(int newHorizontal, int newVertical)
{
    this->setMinimumWidth(newHorizontal);
    this->setMaximumWidth(newHorizontal);
    this->setMinimumHeight(newVertical);
    this->setMaximumHeight(newVertical);
}

// This is where the UI thread tells the worker thread what rule to construct and for which note
void RulesContainer::ButtonOnClick(int rowIndex, int typeIndex)
{
    // If we doubleClicked on an already set rule, we need to display it's modal parameters dialog
    if(rows_list[rowIndex]->active_rule_type_index == typeIndex)
    {
        ParameterDialog* newParametersDialog = rows_list[rowIndex]->active_rule->GetDialogForParams(rows_list[rowIndex]->active_rule->GetRuleParams());
        if(newParametersDialog != nullptr)
        {
            newParametersDialog->setModal(true);
            newParametersDialog->exec();
            if(newParametersDialog->result() == QDialog::Accepted)
            {
                rows_list[rowIndex]->active_rule->SetRuleParams(newParametersDialog->GetDialogOutPut());
                // Update workerRuleParams
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                emit RuleRowRuleModified(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index, rowIndex, newParametersDialog->GetDialogOutPut());
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
            delete newParametersDialog;
        }
    }
    else
    {
        if(rowIndex <= last_set_rule_row_index)
        {
            if(rows_list[rowIndex]->active_rule != nullptr)
            {
                delete rows_list[rowIndex]->active_rule;
            }
            rows_list[rowIndex]->active_rule = rule_types->types_list[typeIndex]->GetNewInstance();
            // Add workerRule to workerThread
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            emit RuleRowRuleCreated(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index, rowIndex, rows_list[rowIndex]->active_rule->GetNewWorkerRule());
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // Update the coloring of all the other buttons
            for(int i = 0; i < rule_types->types_list.count(); i++)
            {
                if(i != typeIndex)
                {
                    QString notSelectedStyleSheet = "background-color: rgb(";
                    notSelectedStyleSheet += QString::number(rule_types->types_list[i]->color_r - 60);
                    notSelectedStyleSheet += ", ";
                    notSelectedStyleSheet += QString::number(rule_types->types_list[i]->color_g - 60);
                    notSelectedStyleSheet += ", ";
                    notSelectedStyleSheet += QString::number(rule_types->types_list[i]->color_b - 60);
                    notSelectedStyleSheet += "); border-color : rgb(77, 77, 77); border-width : 2px; border-style : solid; }";

                    rows_list[rowIndex]->row_buttons[i]->setStyleSheet(notSelectedStyleSheet);
                }
                else
                {
                    QString selectedStyleSheet = "background-color: rgb(";
                    selectedStyleSheet += QString::number(rule_types->types_list[i]->color_r);
                    selectedStyleSheet += ", ";
                    selectedStyleSheet += QString::number(rule_types->types_list[i]->color_g);
                    selectedStyleSheet += ", ";
                    selectedStyleSheet += QString::number(rule_types->types_list[i]->color_b);
                    selectedStyleSheet += "); border-color : rgb(255, 255, 255); border-width : 2px; border-style : solid; }";

                    rows_list[rowIndex]->row_buttons[i]->setStyleSheet(selectedStyleSheet);
                }
            }

            rows_list[rowIndex]->active_rule_type_index = typeIndex;
            if(rowIndex == last_set_rule_row_index)
            {
                last_set_rule_row_index++;
            }
         }
    }
}

void RulesContainer::AddRuleRow()
{
    selected_row_index = -1; // no row selected

    ContainerRow* newRulesRow = new ContainerRow;
    newRulesRow->row = new RowWidget(this, rows_list.count());
    ///////////////////////////////////////////////
    connect(newRulesRow->row, SIGNAL(RowClicked(int)), this, SLOT(RowClicked(int)));
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    rows_list.push_back(newRulesRow);
    ///////////////////////////////////////////////

    newRulesRow->active_rule = nullptr; // it will get populated when a button for a rule gets pressed
    newRulesRow->active_rule_type_index = -1;
    newRulesRow->row->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    newRulesRow->row->setMinimumHeight(RULE_ROW_HEIGHT);
    newRulesRow->row->setMaximumHeight(RULE_ROW_HEIGHT);

    newRulesRow->row->show();

    MyPushButton* newPushButton;
    for(int i = 0; i < rule_types->types_list.count(); i++)
    {
        newPushButton = new MyPushButton(newRulesRow->row);
        newPushButton->setObjectName(rule_types->types_list[i]->name);
        newRulesRow->row_buttons.push_back(newPushButton);

        QString newStyleSheet = "QWidget#";
        newStyleSheet += rule_types->types_list[i]->name;
        newStyleSheet += " { background-color: rgb(";
        newStyleSheet += QString::number(rule_types->types_list[i]->color_r);
        newStyleSheet += ", ";
        newStyleSheet += QString::number(rule_types->types_list[i]->color_g);
        newStyleSheet += ", ";
        newStyleSheet += QString::number(rule_types->types_list[i]->color_b);
        newStyleSheet += "); border-color : rgb(77, 77, 77); border-width : 2px; border-style : solid; }";
        newPushButton->setStyleSheet(newStyleSheet);



        newPushButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        // set for which row and rule should this pushButton report for
        newPushButton->row_index = rows_list.count() - 1;
        newPushButton->type_index = i;

        // set on-click event
        //////////////////////////////////////////
        connect(newPushButton, SIGNAL(MyPushButtonOnClick(int, int)), this, SLOT(ButtonOnClick(int, int)));
        //////////////////////////////////////////

        newPushButton->show();
    }
}

void RulesContainer::RemoveRuleRow()
{
    if(selected_row_index != -1)
    {
        if(rows_list[selected_row_index]->row != nullptr)
        {
            delete rows_list[selected_row_index]->row;
            rows_list[selected_row_index]->row = nullptr;
        }

        if(rows_list[selected_row_index]->active_rule != nullptr)
        {
            delete rows_list[selected_row_index]->active_rule;
            rows_list[selected_row_index]->active_rule = nullptr;

            // Before we remove this rows_list entry we need to tell the workerThread to remove this rule as well
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            emit RuleRowRuleRemoved(midi_notes_container_entry_list_tab_index, midi_notes_container_entry_list_note_index, selected_row_index);
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }

        if(rows_list[selected_row_index] != nullptr)
        {
            delete rows_list[selected_row_index];
            rows_list[selected_row_index] = nullptr;
        }

        rows_list.removeAt(selected_row_index);

        for(int i = 0; i < rows_list.count(); i++)
        {
            rows_list[i]->row->row_index = i;
            for(int j = 0; j < rows_list[i]->row_buttons.count(); j++)
            {
                rows_list[i]->row_buttons[j]->row_index = i;
            }
        }

        if(selected_row_index < last_set_rule_row_index)
        {
            last_set_rule_row_index--;
        }

        selected_row_index = -1;
    }
}

void RulesContainer::CalculateAllPositionsAndDimensions(float containerWidth)
{
    new_font_size = (containerWidth * 11) / 223;
    if(new_font_size > 20.0f)
    {
        new_font_size = 20.0f;
    }

    // Drawing each note name
    rule_name_offsets_x.clear();
    rule_button_offsets_x.clear();
    float offsetX = RULE_NAME_OFFSET_X;

    rule_name_box_width = ((float)containerWidth - 40.0f) / (float)(rule_types->types_list.count() + 1);

    QFont defaultFont;
    defaultFont.setPointSize(new_font_size);
    QFontMetrics fm(defaultFont);
    rule_name_box_height = (float)fm.height();
    float rule_name_space_in_between_boxes = rule_name_box_width / (rule_types->types_list.count() - 1);
    rule_button_size = rule_name_box_height;
    if(rule_button_size > RULE_ROW_HEIGHT)
    {
        rule_button_size = RULE_ROW_HEIGHT;
    }
    float rule_button_centered_offset_x = (rule_name_box_width - rule_button_size) / 2;

    for(int i = 0; i < rule_types->types_list.count(); i++)
    {
        rule_name_offsets_x.push_back(offsetX);
        rule_button_offsets_x.push_back(offsetX + rule_button_centered_offset_x);

        offsetX += rule_name_box_width;
        offsetX += rule_name_space_in_between_boxes;
    }

    // Drawing each row in this container
    rule_row_offsets_y.clear();
    float rowOffsetY = RULE_NAME_OFFSET_Y + rule_name_box_height + SPACE_BETWEEN_RULE_ROWS;
    rule_button_offset_y = (RULE_ROW_HEIGHT - rule_button_size) / 2;
    for(int i = 0; i < rows_list.count(); i++)
    {
        rule_row_offsets_y.push_back(rowOffsetY);
        rowOffsetY += RULE_ROW_HEIGHT;
        rowOffsetY += SPACE_BETWEEN_RULE_ROWS;
    }

    if(rowOffsetY < RULE_CONTAINER_MINIMUM_HEIGHT)
    {
        rules_container_height = RULE_CONTAINER_MINIMUM_HEIGHT;
    }
    else
    {
        rules_container_height = rowOffsetY;
    }
}

void RulesContainer::RowClicked(int rowIndex)
{
    selected_row_index = rowIndex;

    SetRowBackgroundColors();
}

void RulesContainer::mousePressEvent(QMouseEvent *event) // When user presses the background and deselects all rows
{
    if(event->button() == Qt::LeftButton)
    {
         selected_row_index = -1;
    }

    SetRowBackgroundColors();
}

void RulesContainer::SetRowBackgroundColors()
{
    for(int i = 0; i < rows_list.count(); i++)
    {
        if(i == selected_row_index)
        {
            rows_list[i]->row->setStyleSheet("background-color: rgb(80, 80, 80);");
            rows_list[i]->row->setAutoFillBackground(true);
        }
        else
        {
            rows_list[i]->row->setAutoFillBackground(false);
        }
    }
}
