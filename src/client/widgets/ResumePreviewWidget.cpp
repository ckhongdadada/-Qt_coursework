#include "ResumePreviewWidget.h"

ResumePreviewWidget::ResumePreviewWidget(QWidget* parent)
    : QTextBrowser(parent)
{
    setObjectName("richCardText");
    setOpenExternalLinks(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    document()->setDocumentMargin(10);

    connect(this, &QTextBrowser::anchorClicked, this, &ResumePreviewWidget::onAnchorClicked);
}

void ResumePreviewWidget::setResumeHtml(const QString& html)
{
    setHtml(html);
}

void ResumePreviewWidget::setSelectedSection(const QString& key)
{
    m_selectedSection = key;
}

QString ResumePreviewWidget::selectedSection() const
{
    return m_selectedSection;
}

void ResumePreviewWidget::onAnchorClicked(const QUrl& url)
{
    const QString scheme = url.scheme();
    QString key = url.host();
    if (key.isEmpty()) {
        key = url.path();
    }
    if (key.startsWith('/')) {
        key.remove(0, 1);
    }

    if (scheme == "section") {
        m_selectedSection = key;
        emit sectionSelected(key);
    } else if (scheme == "action" && key == "avatar") {
        emit avatarClicked();
    }
}
