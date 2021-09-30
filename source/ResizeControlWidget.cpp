#include "ResizeControlWidget.h"

#include <QDebug>

ResizeControlWidget::ResizeControlWidget(QWidget* parent)
    : QWidget(parent)
{}

void ResizeControlWidget::setControlledWidget(QWidget *controlledWidget)
{
    _controlledWidget = controlledWidget;
}

bool ResizeControlWidget::event(QEvent *event)
{
    if(_controlledWidget && event->type() == QEvent::Resize)
    {
        _controlledWidget->setVisible(false);
        _controlledWidget->setGeometry(this->geometry().x() + 10,
                                       this->geometry().y() + 10,
                                       this->geometry().width(),
                                       this->geometry().height() - 10);

        _resized = true;
    }

    if(_resized && _controlledWidget && event->type() == QEvent::Paint)
    {
        _controlledWidget->setVisible(true);
        _resized = false;
    }

    QWidget::event(event);

    return true;
}
