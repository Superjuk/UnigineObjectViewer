#pragma once

#include <QWidget>
#include <QEvent>

class ResizeControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResizeControlWidget(QWidget* parent = nullptr);

    void setControlledWidget(QWidget* controlledWidget);

private:
    QWidget* _controlledWidget = nullptr;
    bool     _resized = false;

    bool event(QEvent* event) override;

signals:

};
