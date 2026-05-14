#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>

class BasePage : public QWidget {
    Q_OBJECT

public:
    explicit BasePage(QWidget* parent = nullptr);
    ~BasePage() override;

    virtual void refresh() = 0;

protected:
    QFrame* createMetricCard(const QString& labelText, QLabel** valueLabel,
                             const QString& helperText = QString());
};
