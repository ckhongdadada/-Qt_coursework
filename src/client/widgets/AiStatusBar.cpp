#include "AiStatusBar.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "service/AiService.h"
#include "client/utils/UiHelpers.h"
using namespace UiHelpers;

AiStatusBar::AiStatusBar(QWidget* parent)
    : QFrame(parent)
{
    setupUi();
}

void AiStatusBar::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(8);

    m_modeValue = new QLabel("离线模式", this);
    m_statusValue = new QLabel("加载完成", this);
    m_modelValue = new QLabel("Internal", this);

    QString tagStyle = "background: #f4f6f8; color: #555; padding: 4px 8px; border-radius: 999px; font-size: 11px;";
    m_modeValue->setStyleSheet(tagStyle);
    m_statusValue->setStyleSheet(tagStyle);
    m_modelValue->setStyleSheet(tagStyle);

    statusLayout->addWidget(m_modeValue);
    statusLayout->addWidget(m_statusValue);
    statusLayout->addWidget(m_modelValue);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);

    m_contextLabel = new QLabel(this);
    m_contextLabel->setObjectName("aiContextLabel");
    m_contextLabel->setWordWrap(true);
    m_contextLabel->setMaximumHeight(80);
    m_contextLabel->setStyleSheet(
        "background: #eef7f5; border: 0.5px solid rgba(15,111,120,0.28); border-radius: 6px;"
        "padding: 8px 10px; font-size: 12px; color: #24211d;");
    m_contextLabel->hide();
    layout->addWidget(m_contextLabel);
}

void AiStatusBar::refreshStatus()
{
    const QJsonObject status = AiService::checkStatus();
    const QString mode = safeText(status["mode"].toString(), "unavailable");
    const QString model = safeText(status["model"].toString(), "local-model");
    const bool available = status["available"].toBool();
    m_modeValue->setText(QString("模式：%1").arg(mode));
    m_modelValue->setText(QString("模型：%1").arg(model));
    m_statusValue->setText(available ? "状态：可用" : "状态：不可用");
}

void AiStatusBar::setContext(const QString& type, const QString& context)
{
    m_selectedContext = context;
    if (m_contextLabel) {
        m_contextLabel->setText(QString("上下文 [%1]：%2").arg(type, context.left(100)));
        m_contextLabel->show();
    }
}

void AiStatusBar::clearContext()
{
    m_selectedContext.clear();
    if (m_contextLabel) {
        m_contextLabel->hide();
    }
}
