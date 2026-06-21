#pragma once

#include <QFrame>
#include <QListWidget>
#include <QJsonArray>

class SuggestionListWidget : public QFrame {
    Q_OBJECT

public:
    explicit SuggestionListWidget(const QString& title, QWidget* parent = nullptr);

    void loadItems(const QJsonArray& items);
    void loadStrings(const QStringList& items);
    void clearItems();
    int count() const;

private:
    void setupUi(const QString& title);

    QListWidget* m_list = nullptr;
};
