#pragma once

#include <QString>
#include <QWidget>

class ToastNotification : public QWidget {
    Q_OBJECT
public:
    static void display(QWidget* parent, const QString& message);

private:
    explicit ToastNotification(QWidget* parent, const QString& message);
};
