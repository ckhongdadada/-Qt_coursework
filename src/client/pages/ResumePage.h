#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class ResumeEditorPanel;
class ResumeCandidatePanel;
class ResumePreviewWidget;
class ResumePreviewDialog;

class ResumePage : public BasePage {
    Q_OBJECT

public:
    explicit ResumePage(QWidget* parent = nullptr);
    ~ResumePage() override = default;

    void refresh() override;
    QJsonObject currentOptions() const;
    void resetOptions();

signals:
    void dataChanged(DataDomain domain);

private slots:
    void onPreviewClicked();
    void onRefreshClicked();
    void onResetClicked();
    void onExportJsonClicked();
    void onExportHtmlClicked();
    void onCopyToClipboardClicked();
    void onInsertToSectionRequested(const QString& section);
    void onClearCustomContentRequested();
    void onCandidateClicked(const QString& snippet);
    void onSectionSelected(const QString& key);
    void onAvatarClicked();
    void onOptionsChanged();

private:
    void setupUi();
    void updatePreview();
    void saveOptions();
    void ensureResumePreviewDialog();
    void showResumePreviewDialog();
    void refreshResumePreviewDialog();
    QString buildResumePreviewHtml() const;
    QJsonObject defaultOptions() const;
    QString selectedClass(const QString& key) const;
    QString actionHtml(const QString& key) const;
    bool isVisibleSection(const QString& section) const;

    ResumeEditorPanel* m_editorPanel = nullptr;
    ResumeCandidatePanel* m_candidatePanel = nullptr;
    ResumePreviewDialog* m_previewDialog = nullptr;

    QLabel* m_summaryLabel = nullptr;
    QLabel* m_sectionCountValue = nullptr;
    QLabel* m_identityValue = nullptr;
    QPushButton* m_previewButton = nullptr;

    QString m_selectedSection = "education";
    QString m_avatarPath;
    QTimer* m_updateTimer = nullptr;
};
