#ifndef RULESCONTAINER_H
#define RULESCONTAINER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QPushButton>
//#include <QDebug>
#include "MidiRules/ruletypes.h"

#define RULE_ROW_HEIGHT 40
#define SPACE_BETWEEN_RULE_ROWS 21
#define RULE_NAME_OFFSET_Y 21
#define RULE_NAME_OFFSET_X 20
#define RULE_CONTAINER_MINIMUM_HEIGHT 200

class RowWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit RowWidget(QWidget* parent, int rowIndex) : QWidget(parent)
        {
            row_index = rowIndex;
        };
        void mousePressEvent(QMouseEvent *event)
        {
            if(event->button() == Qt::LeftButton)
            {
                 emit RowClicked(row_index);
            }
        };

        int row_index;

    signals:
        void RowClicked(int rowIndex);
};

class MyPushButton : public QWidget
{
    Q_OBJECT

    public:
        explicit MyPushButton(QWidget* parent = nullptr);
         void mouseDoubleClickEvent (QMouseEvent* event);
         void paintEvent(QPaintEvent* event);

        int row_index;
        int type_index;

    signals:
        void MyPushButtonOnClick(int rowIndex, int ruleIndex);

};

class RulesContainer : public QWidget
{
    Q_OBJECT

    public:
        explicit RulesContainer(QWidget *parent = nullptr, RuleTypes* ruleTypes = nullptr);
        ~RulesContainer();
        void paintEvent(QPaintEvent* event);
        void mousePressEvent(QMouseEvent *event);
        void SetMinimumAndMaximumHorizontalVertical(int newHorizontal, int newVertical);
        void AddRuleRow();
        void RemoveRuleRow();
        void CalculateAllPositionsAndDimensions(float containerWidth);
        float rules_container_height; // in pixels for the parent to read and use to resize the midiNote
        int midi_notes_container_entry_list_tab_index;
        int midi_notes_container_entry_list_note_index;


        QWidget* parent_widget;
        RuleTypes* rule_types;

        struct ContainerRow
        {
            RowWidget* row;
            QList<MyPushButton*> row_buttons;
            Rule* active_rule;
            int active_rule_type_index; // if it gets selected again we need to open the parameter dialog
        };
        QList<ContainerRow*> rows_list;
        int selected_row_index;
        int last_set_rule_row_index; // we can have multiple rows added at the same time, but can create a rule only for the one next in line
        float rule_name_box_width;
        float rule_name_box_height;
        QList<float> rule_button_offsets_x; // so that each item can be evenly spaced
        float rule_button_offset_y;
        float rule_button_size;
        QList<float> rule_row_offsets_y;
        QList<float> rule_name_offsets_x;
        float new_font_size;

    signals:
        void RuleRowRuleCreated(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRule);
        void RuleRowRuleModified(int tabIndex, int ruleMatrixIndex, int ruleIndex, void* workerRuleParams);
        void RuleRowRuleRemoved(int tabIndex, int ruleMatrixIndex, int ruleIndex);

    public slots:
        void ButtonOnClick(int rowIndex, int typeIndex);
        void RowClicked(int rowIndex);
        void SetRowBackgroundColors();

};


#endif // RULESCONTAINER_H
