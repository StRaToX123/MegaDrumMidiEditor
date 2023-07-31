#ifndef SCROLLBARPARAMS_H
#define SCROLLBARPARAMS_H

#include <QObject>

class ScrollBarParams : public QObject
{
    Q_OBJECT

    public:
        explicit ScrollBarParams(QObject *parent = nullptr);

        int color_background_r;
        int color_background_g;
        int color_background_b;
        float background_opacity;
        int color_handle_r;
        int color_handle_g;
        int color_handle_b;
        int handle_border_radius;
        int items_space_between;
        int position_x;
        int position_y;
        int background_width;
        int background_height;
        int background_height_previous;
        float handle_width_percentage_01;
        int list_size;

    signals:

};

#endif // SCROLLBARPARAMS_H
