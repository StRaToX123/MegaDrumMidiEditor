#include "tab.h"

Tab::Tab(QWidget *parent, int tabsContainerIndex) : QWidget(parent)
{
    font_size = 8;
    tabs_container_index = tabsContainerIndex;
    name = "tab ";
    name += QString::number(tabs_container_index);
    default_font = QFont();
    close_button = new QPushButton(this);

    QSizePolicy newSizePolicy;
    newSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    newSizePolicy.setVerticalStretch(0);
    newSizePolicy.setHorizontalStretch(0);

    close_button->setSizePolicy(newSizePolicy);
    close_button->setMinimumSize(10, 10);
    close_button->setMaximumSize(10, 10);
    close_button->setText("");
    QIcon closeIcon(":/Resources/Images/TitleBar/close_Icon.png");
    close_button->setIcon(closeIcon);
    close_button->setFlat(true);
    ///////////////////////////////////////////
    connect(close_button, SIGNAL(clicked()), this, SLOT(CloseTab()));
    ///////////////////////////////////////////
}

void Tab::CloseTab()
{
    emit RemoveTab(tabs_container_index, false);
}

float Tab::GetWidth()
{
    return tab_width;
}

void Tab::SetHeight(float height)
{
    setMinimumHeight(height);
    setMaximumHeight(height);
    tab_height = height;
}

float Tab::GetHeight()
{
    return tab_height;
}

void Tab::SetFontSize(float size, float monitorScaleFactor)
{
    default_font.setPointSize(size * monitorScaleFactor);
    QFontMetrics fm(default_font);
    text_width = fm.size(0, name).width();
    //text_width = text_width * ((*(monitor_scale_factor)) < 1.0f ? 1.0f : (*(monitor_scale_factor)));
    tab_width = text_width + (2 * LEFT_AND_RIGHT_MARGIN) + close_button->width() + 20;
    text_height = fm.height();
    default_font.setPointSize(size);

    //qDebug() << "SPECIAL POST QT: " << text_width;

    setMinimumWidth(tab_width);
    setMaximumWidth(tab_width);
}

// Switch to newly selected tab
void Tab::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit TabClicked(tabs_container_index, true);
    }
}

// Open Dialog for tab name change on doubleClick
void Tab::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QDialog* newDialog = new QDialog(this);
        QGridLayout* newGridLayout = new QGridLayout(newDialog);
        line_edit_name = new QLineEdit(newDialog);
        line_edit_name->setText(name);
        line_edit_name->setAlignment(Qt::AlignLeft);

        QPushButton* newPushButtonAccept = new QPushButton(newDialog);
        QPushButton* newPushButtonCancel = new QPushButton(newDialog);

        newPushButtonAccept->setText("Accept");
        newPushButtonCancel->setText("Cancel");

        connect(newPushButtonAccept, SIGNAL(clicked()), newDialog, SLOT(accept()));
        connect(newPushButtonCancel, SIGNAL(clicked()), newDialog, SLOT(reject()));

        newGridLayout->addWidget(line_edit_name, 0, 0, Qt::AlignCenter);
        newGridLayout->addWidget(newPushButtonAccept, 1, 1, Qt::AlignCenter);
        newGridLayout->addWidget(newPushButtonCancel, 2, 1, Qt::AlignCenter);


        newDialog->exec();
        if(newDialog->result() == QDialog::Accepted)
        {
            name = line_edit_name->text();
        }
        delete newDialog;
    }
}

void Tab::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    QPen newPen(p.pen());
    newPen.setWidth(6);
    newPen.setColor(Qt::white);
    p.setPen(newPen);

    // Draw the container
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.setFont(default_font);

    p.drawText(LEFT_AND_RIGHT_MARGIN, (tab_height - text_height) / 2, text_width, text_height, Qt::AlignVCenter | Qt::AlignHCenter, name);

    close_button->move(tab_width - close_button->width() - LEFT_AND_RIGHT_MARGIN - 5, (height() - close_button->height()) / 2);

    QWidget::paintEvent(event);
}
