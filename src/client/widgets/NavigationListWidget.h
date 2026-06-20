#pragma once

#include <QListWidget>
#include <QStringList>

class NavigationListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit NavigationListWidget(QWidget* parent = nullptr);

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;
    void setCurrentNavIndex(int index);
    int currentNavIndex() const;
    void rebuildItems();
    void syncSelectionStyle();

signals:
    void navigationRequested(int index);

private:
    bool m_collapsed = false;
    QStringList m_labels;
    QStringList m_tooltips;

    void setupItems();
};
