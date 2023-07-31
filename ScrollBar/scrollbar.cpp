#include "scrollbar.h"



ScrollBar::ScrollBar(QWidget *parent,
                        int colorBackgroundR,
                            int colorBackgroundG,
                                int colorBackgroundB,
                                    float backgroundOpacity,
                                        int colorHandleR,
                                            int colorHandleG,
                                                int colorHandleB,
                                                    float handleWidthPercentage01,
                                                        int handleBorderRadius) : QWidget(parent)
{
    parent_widget = static_cast<QWidget*>(parent);

    lerp_initial_positions_initialized = false;
    list_size_calculated = false;

    scroll_bar_params = new ScrollBarParams(this);

    scroll_bar_params->color_background_r = colorBackgroundR;
    scroll_bar_params->color_background_g = colorBackgroundG;
    scroll_bar_params->color_background_b = colorBackgroundB;
    scroll_bar_params->background_opacity = backgroundOpacity;
    scroll_bar_params->color_handle_r = colorHandleR;
    scroll_bar_params->color_handle_g = colorHandleG;
    scroll_bar_params->color_handle_b = colorHandleB;
    scroll_bar_params->handle_width_percentage_01 = handleWidthPercentage01;
    scroll_bar_params->handle_border_radius = handleBorderRadius;
    scroll_bar_params->background_height_previous = -1;

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Adding transparency effect
    QGraphicsOpacityEffect* newDs = new QGraphicsOpacityEffect(this);
    newDs->setOpacity(scroll_bar_params->background_opacity);
    this->setGraphicsEffect(newDs);

    QString backgroundStyleSheet = "background-color : rgba(";
    backgroundStyleSheet += QString::number(scroll_bar_params->color_background_r);
    backgroundStyleSheet += ", ";
    backgroundStyleSheet += QString::number(scroll_bar_params->color_background_g);
    backgroundStyleSheet += ", ";
    backgroundStyleSheet += QString::number(scroll_bar_params->color_background_b);
    backgroundStyleSheet += ", ";
    backgroundStyleSheet += QString::number(scroll_bar_params->background_opacity);
    backgroundStyleSheet += ");";

    this->setStyleSheet(backgroundStyleSheet);

    // handle must not have this widget as a parent otherwise it would get a transparency effect too
    handle = new ScrollBarHandle(static_cast<QWidget*>(parent), scroll_bar_params);
    connect(handle, SIGNAL(moveItems()), this, SLOT(moveItems()));
}

ScrollBar::~ScrollBar()
{

}

void ScrollBar::addItemToList()
{
    if(itemPositions.count() == 0)
    {
        ItemPosition newPosition(this);
        newPosition.pos_x = 0;
        newPosition.pos_y = scroll_bar_params->items_space_between;
        newPosition.original_pos_x = 0;
        newPosition.original_pos_y = newPosition.pos_y;
        newPosition.index_in_array = 0;
        itemPositions.push_back(newPosition);
    }
    else
    {
        ItemPosition newPosition(this);
        newPosition.pos_x = itemPositions.last().pos_x;
        newPosition.pos_y = itemPositions.last().original_pos_y + itemPositions.last().GetItemDimension() + scroll_bar_params->items_space_between;
        newPosition.original_pos_x = 0;
        newPosition.original_pos_y = newPosition.pos_y;
        newPosition.index_in_array = itemPositions.count();
        itemPositions.push_back(newPosition);
    }
}

void ScrollBar::RemoveItemPositionAt(int index)
{
    if((itemPositions.count() - 1) >= (index + 1))
    {
        // Before removing the item first, we need to alter all the positions of items coming after
        itemPositions[index + 1].original_pos_y = itemPositions[index].original_pos_y;
        itemPositions[index + 1].pos_y = itemPositions[index].pos_y;
        itemPositions[index + 1].index_in_array = index;
        for(int i = (index + 2); i < itemPositions.count(); i++)
        {
            itemPositions[i].original_pos_y = itemPositions[i - 1].original_pos_y + itemPositions[i - 1].GetItemDimension() + scroll_bar_params->items_space_between;
            itemPositions[i].pos_y = itemPositions[i - 1].pos_y + itemPositions[i - 1].GetItemDimension() + scroll_bar_params->items_space_between;
            itemPositions[i].index_in_array = i - 1;
        }
    }

    list_size_calculated = false;
    itemPositions.removeAt(index);
}

// Gets called when a new item is added to the list, this function resizes the handle and replaces it so that the list doesn't move
void ScrollBar::UpdateHandlePositionAndSize()
{
    if(list_size_calculated == false)
    {
        list_size_calculated = true;
        scroll_bar_params->list_size = 0;
        for(int i = 0; i < itemPositions.count(); i++)
        {
            scroll_bar_params->list_size += itemPositions[i].GetItemDimension();
            scroll_bar_params->list_size += scroll_bar_params->items_space_between;
        }
        scroll_bar_params->list_size += scroll_bar_params->items_space_between;
    }

    int listSizeComparedToWidgetSizePercentage = 100 - (int)(((scroll_bar_params->list_size * 100) / scroll_bar_params->background_height));
    // Positive values mean the listSize needs that much more to become equal to the parent widget size
    if(listSizeComparedToWidgetSizePercentage >= 0)
    {
        handle->handle_size = scroll_bar_params->background_height;
    }
    else // negative values mean that the litSize has already gone over the widgetSize by that amount
    {
        // We need to clamp the size reduction at some point
        // Lets say 5 %  of the parent widget size
        int newHandleSizeComparedToWidgetSize = 100 + listSizeComparedToWidgetSizePercentage;
        if(newHandleSizeComparedToWidgetSize < 5)
        {
            handle->handle_size = (scroll_bar_params->background_height * 5) / 100;
        }
        else
        {
            handle->handle_size = (newHandleSizeComparedToWidgetSize * scroll_bar_params->background_height) / 100;
        }
    }

    if((handle->handle_position + handle->handle_size) > scroll_bar_params->background_height)
    {
        handle->handle_position = scroll_bar_params->background_height - handle->handle_size;
    }


    // If the handle has moved because of a resize
    // We need to move all of the items around by that resize amount
    int parentResizeDelta = scroll_bar_params->background_height - scroll_bar_params->background_height_previous;

    if((parentResizeDelta > 0) // if the window is being resized positivly (the window is growing)
            && (handle->handle_position != 0)
                && ((handle->handle_position + handle->handle_size) == scroll_bar_params->background_height)
                    && (scroll_bar_params->list_size > parent_widget->height()))
    {
        if(lerp_initial_positions_initialized == false)
        {
            lerp_initial_positions_initialized = true;
            for(int i = 0; i < itemPositions.count(); i++)
            {
                itemPositions[i].lerp_initial_position = itemPositions[i].pos_y;
            }
            lerp_01_value_remap = ((float)handle->handle_size / (float)scroll_bar_params->background_height);
        }

        for(int i = 0; i < itemPositions.count(); i++)
        {
            itemPositions[i].pos_y = Lerp(itemPositions[i].lerp_initial_position, itemPositions[i].original_pos_y , Remap(((float)handle->handle_size / (float)scroll_bar_params->background_height), lerp_01_value_remap, 1.0f, 0.0f, 1.0f));
        }

        emit parent_widget->repaint();
    }
    else
    {
        if(handle->handle_position == 0)
        {
            if(lerp_initial_positions_initialized == true)
            {
                for(int i = 0; i < itemPositions.count(); i++)
                {
                    itemPositions[i].pos_y = itemPositions[i].original_pos_y;
                }

                lerp_initial_positions_initialized = false;
                emit parent_widget->repaint();
            }
            else // this will execute if for example the window goes fullscreen or for what ever reason the bar goes to handle position 0.0f
            {
                if(itemPositions.count() > 0)
                {
                    if(itemPositions[0].pos_y != itemPositions[0].original_pos_y)
                    {
                        for(int i = 0; i < itemPositions.count(); i++)
                        {
                            itemPositions[i].pos_y = itemPositions[i].original_pos_y;
                        }

                        emit parent_widget->repaint();
                    }
                }
            }
        }
    }
}

void ScrollBar::moveItems()
{
    lerp_initial_positions_initialized = false;

    if(list_size_calculated == false)
    {
        list_size_calculated = true;
        scroll_bar_params->list_size = 0;
        for(int i = 0; i < itemPositions.count(); i++)
        {
            scroll_bar_params->list_size += itemPositions[i].GetItemDimension();
            scroll_bar_params->list_size += scroll_bar_params->items_space_between;
        }
        scroll_bar_params->list_size += scroll_bar_params->items_space_between;
    }

    int offset = ((scroll_bar_params->list_size - scroll_bar_params->background_height) * handle->handle_position) / (scroll_bar_params->background_height - handle->handle_size);

    for(int i = 0; i < itemPositions.count(); i++)
    {
        itemPositions[i].pos_y = itemPositions[i].original_pos_y - offset;
    }

    emit parent_widget->repaint();
}

void ScrollBar::setPosition(int x, int y)
{
    scroll_bar_params->position_x = x;
    scroll_bar_params->position_y = y;
}

void ScrollBar::setDimension(int _width, int _height)
{
    scroll_bar_params->background_width = _width;

    if(scroll_bar_params->background_height_previous == -1)
    {
        scroll_bar_params->background_height_previous = _height;
    }
    else
    {
        scroll_bar_params->background_height_previous = scroll_bar_params->background_height;
    }

    scroll_bar_params->background_height = _height;
}

void ScrollBar::setSpaceBetweenItems(int spaceBetween)
{
    scroll_bar_params->items_space_between = spaceBetween;
}

void ScrollBar::paintEvent(QPaintEvent* event)
{
        QStyleOption opt;
        opt.initFrom(this);

        QPainter p(this);

        ////////////////////////////////////////////
        UpdateHandlePositionAndSize();
        ////////////////////////////////////////////

        this->move(scroll_bar_params->position_x, scroll_bar_params->position_y);
        this->resize(scroll_bar_params->background_width, scroll_bar_params->background_height);

        // Draw the ScrollBar background
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

        handle->paintEvent(event);

        QWidget::paintEvent(event);
}

void ScrollBar::CreateTab()
{
    tab_data.push_back(TabData(this));
    if(tab_data.count() == 1)
    {
        active_tab_index = 0;
    }
}

void ScrollBar::RemoveTab(int tabIndex)
{
    tab_data.removeAt(tabIndex);
    if(tabIndex == active_tab_index)
    {
        active_tab_index = -1;
    }
}

void ScrollBar::BackUpTab()
{
    if(active_tab_index != -1)
    {
        tab_data[active_tab_index].itemPositions = itemPositions;
        tab_data[active_tab_index].list_size = scroll_bar_params->list_size;
        tab_data[active_tab_index].handle_size = handle->handle_size;
        tab_data[active_tab_index].handle_position = handle->handle_position;
        tab_data[active_tab_index].background_height = scroll_bar_params->background_height;
    }
}

void ScrollBar::ActivateTab(int tabIndex)
{
    itemPositions = tab_data[tabIndex].itemPositions;
    scroll_bar_params->list_size = tab_data[tabIndex].list_size;
    scroll_bar_params->background_height = tab_data[tabIndex].background_height;
    handle->handle_position = tab_data[tabIndex].handle_position;
    handle->handle_size = tab_data[tabIndex].handle_size;

    list_size_calculated = true;

    active_tab_index = tabIndex;
}




