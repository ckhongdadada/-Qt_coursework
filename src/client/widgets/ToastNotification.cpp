#include "client/widgets/ToastNotification.h"

#include <QAbstractAnimation>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

void ToastNotification::display(QWidget* parent, const QString& message)
{
    if (!parent) {
        return;
    }

    auto* toast = new ToastNotification(parent, message);
    toast->show();
    toast->raise();
}

ToastNotification::ToastNotification(QWidget* parent, const QString& message)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* label = new QLabel(message, this);
    label->setStyleSheet(
        "background: rgba(43, 92, 93, 0.95); color: white; padding: 12px 24px; border-radius: 8px; "
        "font-size: 14px; font-weight: bold; border: 1px solid #1a3c3c;"
    );

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    adjustSize();
    move((parent->width() - width()) / 2, 40);

    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    auto* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(250);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(2500, this, [this, effect]() {
        auto* fadeOut = new QPropertyAnimation(effect, "opacity");
        fadeOut->setDuration(400);
        fadeOut->setStartValue(1);
        fadeOut->setEndValue(0);
        connect(fadeOut, &QPropertyAnimation::finished, this, &QObject::deleteLater);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}
