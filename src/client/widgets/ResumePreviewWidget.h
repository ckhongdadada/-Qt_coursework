#pragma once

#include <QTextBrowser>
#include <QUrl>

class ResumePreviewWidget : public QTextBrowser {
    Q_OBJECT

public:
    explicit ResumePreviewWidget(QWidget* parent = nullptr);

    void setResumeHtml(const QString& html);
    void setSelectedSection(const QString& key);
    QString selectedSection() const;

signals:
    void sectionSelected(const QString& key);
    void avatarClicked();

private slots:
    void onAnchorClicked(const QUrl& url);

private:
    QString m_selectedSection = "education";
};
