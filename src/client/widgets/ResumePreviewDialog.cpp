#include "ResumePreviewDialog.h"

#include "ResumePreviewWidget.h"

#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

ResumePreviewDialog::ResumePreviewDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("简历预览"));
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose, false);
    resize(1200, 820);

    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        resize(int(available.width() * 0.76), int(available.height() * 0.82));
    }

    setupUi();
}

void ResumePreviewDialog::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(12);

    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel(QString::fromUtf8("简历预览"), this);
    titleLabel->setObjectName("sectionTitle");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_refreshButton = new QPushButton(QString::fromUtf8("刷新预览"), this);
    m_copyButton = new QPushButton(QString::fromUtf8("复制 HTML"), this);
    m_exportJsonButton = new QPushButton(QString::fromUtf8("导出 JSON"), this);
    m_exportHtmlButton = new QPushButton(QString::fromUtf8("导出 HTML"), this);
    m_closeButton = new QPushButton(QString::fromUtf8("关闭"), this);

    headerLayout->addWidget(m_refreshButton);
    headerLayout->addWidget(m_copyButton);
    headerLayout->addWidget(m_exportJsonButton);
    headerLayout->addWidget(m_exportHtmlButton);
    headerLayout->addWidget(m_closeButton);
    rootLayout->addLayout(headerLayout);

    m_previewWidget = new ResumePreviewWidget(this);
    m_previewWidget->setObjectName("richCardText");
    m_previewWidget->setStyleSheet(
        "QTextBrowser { background: #f3f5f8; border: 1px solid #d9e0ea; border-radius: 8px; padding: 18px; }");
    rootLayout->addWidget(m_previewWidget, 1);

    connect(m_previewWidget, &ResumePreviewWidget::sectionSelected, this, &ResumePreviewDialog::sectionSelected);
    connect(m_previewWidget, &ResumePreviewWidget::avatarClicked, this, &ResumePreviewDialog::avatarClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &ResumePreviewDialog::refreshRequested);
    connect(m_copyButton, &QPushButton::clicked, this, &ResumePreviewDialog::copyRequested);
    connect(m_exportJsonButton, &QPushButton::clicked, this, &ResumePreviewDialog::exportJsonRequested);
    connect(m_exportHtmlButton, &QPushButton::clicked, this, &ResumePreviewDialog::exportHtmlRequested);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void ResumePreviewDialog::setResumeHtml(const QString& html)
{
    if (m_previewWidget) {
        m_previewWidget->setResumeHtml(html);
    }
}

void ResumePreviewDialog::setSelectedSection(const QString& key)
{
    if (m_previewWidget) {
        m_previewWidget->setSelectedSection(key);
    }
}

ResumePreviewWidget* ResumePreviewDialog::previewWidget() const
{
    return m_previewWidget;
}
