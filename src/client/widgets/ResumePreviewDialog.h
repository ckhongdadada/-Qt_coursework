#pragma once

#include <QDialog>

class QPushButton;
class ResumePreviewWidget;

class ResumePreviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ResumePreviewDialog(QWidget* parent = nullptr);

    void setResumeHtml(const QString& html);
    void setSelectedSection(const QString& key);
    ResumePreviewWidget* previewWidget() const;

signals:
    void sectionSelected(const QString& key);
    void avatarClicked();
    void exportJsonRequested();
    void exportHtmlRequested();
    void copyRequested();
    void refreshRequested();

private:
    void setupUi();

    ResumePreviewWidget* m_previewWidget = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_copyButton = nullptr;
    QPushButton* m_exportJsonButton = nullptr;
    QPushButton* m_exportHtmlButton = nullptr;
    QPushButton* m_closeButton = nullptr;
};
