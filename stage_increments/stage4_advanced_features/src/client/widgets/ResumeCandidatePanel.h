#pragma once

#include <QFrame>
#include <QComboBox>
#include <QListWidget>

class QPushButton;

class ResumeCandidatePanel : public QFrame {
    Q_OBJECT

public:
    explicit ResumeCandidatePanel(QWidget* parent = nullptr);

    void refreshCandidates();
    QString currentSnippet() const;
    QString currentCandidateType() const;

signals:
    void insertToSectionRequested(const QString& section);
    void clearCustomContentRequested();
    void candidateClicked(const QString& snippet);

private:
    void setupUi();
    void updateInsertButton();

    QComboBox* m_candidateTypeCombo = nullptr;
    QListWidget* m_candidateList = nullptr;
    QPushButton* m_insertButton = nullptr;
};
