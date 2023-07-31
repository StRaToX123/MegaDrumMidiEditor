#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
//#include <QDebug>
#include "scrollbarparams.h"
#include "scrollbarhandle.h"


class ScrollBar : public QWidget
{
    Q_OBJECT

    public:
        explicit ScrollBar(QWidget *parent = nullptr,
                                int colorBackgroundR = 0,
                                    int colorBackgroundG = 0,
                                        int colorBackgroundB = 0,
                                            float backgroundOpacity = 0,
                                                int colorHandleR = 0,
                                                    int colorHandleG = 0,
                                                        int colorHandleB = 0,
                                                            float handleWidthPercentage01 = 0,
                                                                int handleBorderRadius = 0);
        ~ScrollBar();
        void paintEvent(QPaintEvent* event);
        void setPosition(int x, int y);
        void setDimension(int _width, int _height);
        void addItemToList();
        void RemoveItemPositionAt(int index);
        void setSpaceBetweenItems(int spaceBetween);
        void UpdateHandlePositionAndSize();
        void CreateTab();
        // stores the current tabs scroll_bar settings in it's slot insde the tab_data array
        void BackUpTab();
        void ActivateTab(int tabIndex);
        void RemoveTab(int tabIndex);
        float Lerp(float a, float b, float f)
        {
            return a + f * (b - a);
        };
        float Remap(float value, float low1, float high1, float low2, float high2)
        {
            return low2 + (((value - low1) * (high2 - low2)) / (high1 - low1));
        };


        class ItemPosition
        {
            public:
                ItemPosition(ScrollBar* parentScrollBar)
                {
                    parent_scroll_bar = parentScrollBar;
                    item_dimension = 0; // so that it will always be different upon setting the value for the first time
                };

                ScrollBar* parent_scroll_bar;
                float pos_x;
                float pos_y;
                float original_pos_x;
                float original_pos_y;
                float lerp_initial_position;
                int index_in_array;
                void SetItemDimension(int itemDimension)
                {
                    if(itemDimension != item_dimension)
                    {
                        parent_scroll_bar->list_size_calculated = false;

                        // Before we change the itemDimension of this ItemPosition we need to move every ItemPosition's pos_y that comes
                        // After this one by the delta of this itemPositions dimension change
                        int dimensionDelta = itemDimension - item_dimension;
                        for(int i = (index_in_array + 1); i < parent_scroll_bar->itemPositions.count(); i++)
                        {
                            parent_scroll_bar->itemPositions[i].original_pos_y += dimensionDelta;
                            parent_scroll_bar->itemPositions[i].pos_y += dimensionDelta;
                        }

                        item_dimension = itemDimension;
                    }
                };
                int GetItemDimension()
                {
                    return item_dimension;
                };
            private:
                int item_dimension;
        };

        class TabData
        {
            public:
                TabData(ScrollBar* parentScrollBar = nullptr)
                {
                    list_size = 0;
                    handle_size = parentScrollBar->scroll_bar_params->background_height;
                    handle_position = 0;
                    background_height = parentScrollBar->handle->handle_size;
                };

                QList<ItemPosition> itemPositions;
                int list_size;
                int handle_size;
                int handle_position;
                int background_height;
        };
        QList<TabData> tab_data;

        QList<ItemPosition> itemPositions;
        int active_tab_index;
        QWidget* parent_widget;
        ScrollBarParams* scroll_bar_params;
        ScrollBarHandle* handle;
        bool list_size_calculated;



        // these used to be private, but are now public so that the midiNotesContainer can copy them over
        // in order to save the state in which the scrollbar was before a tab switch
        bool lerp_initial_positions_initialized;
        float lerp_01_value_remap;


    public slots:
        void moveItems();
};


#endif // SCROLLBAR_H
