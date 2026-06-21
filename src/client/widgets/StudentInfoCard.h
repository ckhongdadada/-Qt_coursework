#pragma once

#include <QPushButton>

class QLabel;

class StudentInfoCard : public QPushButton {
    Q_OBJECT

public:
    explicit StudentInfoCard(QWidget* parent = nullptr);

    void loadFromSettings();
    void saveToSettings();
    void updateDisplay();
    void openEditorNear(QWidget* anchor);

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

private:
    void setupUi();

    QLabel* m_avatar = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_metaLabel = nullptr;
    QWidget* m_textWidget = nullptr;

    bool m_collapsed = false;
};
