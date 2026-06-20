#include "AiContextMediator.h"
#include "client/widgets/AiPanelWidget.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QChildEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QHBoxLayout>
#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>
#include <QCursor>

AiContextMediator::AiContextMediator(QObject* parent)
    : QObject(parent)
{}

void AiContextMediator::bindRootWidget(QWidget* root)
{
    m_rootWidget = root;
    if (m_rootWidget) {
        m_rootWidget->installEventFilter(this);
        makeTextSelectable(m_rootWidget);
    }
}

void AiContextMediator::attachPanel(AiPanelWidget* panel)
{
    m_panel = panel;
}

bool AiContextMediator::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::ChildAdded) {
        if (auto* childEvent = static_cast<QChildEvent*>(event)) {
            if (auto* childWidget = qobject_cast<QWidget*>(childEvent->child())) {
                makeTextSelectable(childWidget);
            }
        }
    }

    if (event->type() == QEvent::MouseButtonPress) {
        // Don't destroy the floating menu if the click is on the menu itself
        // (otherwise the button's clicked signal never fires)
        if (m_floatingWidget) {
            QWidget* clickedWidget = qobject_cast<QWidget*>(watched);
            if (clickedWidget && (clickedWidget == m_floatingWidget || m_floatingWidget->isAncestorOf(clickedWidget))) {
                // Let the click through to the button
                return QObject::eventFilter(watched, event);
            }
        }
        hideFloatingMenu();
        if (auto* mouseEvent = static_cast<QMouseEvent*>(event)) {
            if (mouseEvent->button() == Qt::LeftButton) {
                m_leftMousePressed = true;
                m_draggedEnoughForSelection = false;
                m_mousePressGlobalPos = mouseEvent->globalPosition().toPoint();
            }
        }
    }

    if (event->type() == QEvent::MouseMove && m_leftMousePressed) {
        if (auto* mouseEvent = static_cast<QMouseEvent*>(event)) {
            const int distance = (mouseEvent->globalPosition().toPoint() - m_mousePressGlobalPos).manhattanLength();
            if (distance >= QApplication::startDragDistance()) {
                m_draggedEnoughForSelection = true;
            }
        }
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        if (auto* mouseEvent = static_cast<QMouseEvent*>(event)) {
            if (mouseEvent->button() != Qt::LeftButton || !m_draggedEnoughForSelection) {
                m_leftMousePressed = false;
                m_draggedEnoughForSelection = false;
                return QObject::eventFilter(watched, event);
            }
        }

        // 延迟检测选中文本，让 Qt 先完成文字选择
        QTimer::singleShot(50, this, [this]() {
            QWidget* focusWidget = QApplication::focusWidget();
            QString selectedText = selectedTextFromWidget(focusWidget);

            if (selectedText.isEmpty()) {
                QWidget* widgetUnderCursor = QApplication::widgetAt(QCursor::pos());
                selectedText = selectedTextFromWidget(widgetUnderCursor);
            }

            if (!selectedText.isEmpty() && selectedText.length() < 4000 && selectedText.length() > 1) {
                showFloatingMenu(QCursor::pos(), selectedText);
            }
        });

        m_leftMousePressed = false;
        m_draggedEnoughForSelection = false;
    }
    return QObject::eventFilter(watched, event);
}

void AiContextMediator::makeTextSelectable(QWidget* widget)
{
    if (!widget) {
        return;
    }

    widget->installEventFilter(this);

    if (auto* label = qobject_cast<QLabel*>(widget)) {
        label->setTextInteractionFlags(label->textInteractionFlags()
            | Qt::TextSelectableByMouse
            | Qt::LinksAccessibleByMouse);
        // 设置选中文字为蓝色背景白字
        label->setStyleSheet(label->styleSheet() +
            " QLabel::selection { background-color: #4a90e2; color: #ffffff; }");
    }

    const auto children = widget->findChildren<QWidget*>();
    for (QWidget* child : children) {
        child->installEventFilter(this);
        if (auto* label = qobject_cast<QLabel*>(child)) {
            label->setTextInteractionFlags(label->textInteractionFlags()
                | Qt::TextSelectableByMouse
                | Qt::LinksAccessibleByMouse);
            label->setStyleSheet(label->styleSheet() +
                " QLabel::selection { background-color: #4a90e2; color: #ffffff; }");
        }
    }
}

QString AiContextMediator::selectedTextFromWidget(QWidget* widget) const
{
    for (QWidget* current = widget; current; current = current->parentWidget()) {
        if (auto* textEdit = qobject_cast<QTextEdit*>(current)) {
            const QString selected = textEdit->textCursor().selectedText().trimmed();
            if (!selected.isEmpty()) return selected;
        }
        if (auto* textBrowser = qobject_cast<QTextBrowser*>(current)) {
            const QString selected = textBrowser->textCursor().selectedText().trimmed();
            if (!selected.isEmpty()) return selected;
        }
        if (auto* plainTextEdit = qobject_cast<QPlainTextEdit*>(current)) {
            const QString selected = plainTextEdit->textCursor().selectedText().trimmed();
            if (!selected.isEmpty()) return selected;
        }
        if (auto* lineEdit = qobject_cast<QLineEdit*>(current)) {
            const QString selected = lineEdit->selectedText().trimmed();
            if (!selected.isEmpty()) return selected;
        }
        if (auto* label = qobject_cast<QLabel*>(current)) {
            const QString selected = label->selectedText().trimmed();
            if (!selected.isEmpty()) return selected;
        }
        if (auto* itemView = qobject_cast<QAbstractItemView*>(current)) {
            QStringList parts;
            const QModelIndexList indexes = itemView->selectionModel()
                ? itemView->selectionModel()->selectedIndexes()
                : QModelIndexList();
            for (const QModelIndex& index : indexes) {
                const QString value = index.data(Qt::DisplayRole).toString().trimmed();
                if (!value.isEmpty()) {
                    parts << value;
                }
            }
            const QString selected = parts.join(" ").trimmed();
            if (!selected.isEmpty()) return selected;
        }
    }

    return {};
}

void AiContextMediator::pushSelectionToPanel(const QString& text)
{
    const QString normalized = text.trimmed();
    if (normalized.isEmpty()) {
        return;
    }
    m_lastSelection = normalized;

    if (m_panel) {
        m_panel->addContextFromSelection(normalized);
    }
}

void AiContextMediator::clearSelection()
{
    m_lastSelection.clear();
    if (m_panel) {
        m_panel->clearContext();
    }
}

void AiContextMediator::showFloatingMenu(const QPoint& pos, const QString& text)
{
    hideFloatingMenu();

    m_floatingWidget = new QWidget(m_rootWidget);
    m_floatingWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    m_floatingWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_floatingWidget->setAttribute(Qt::WA_ShowWithoutActivating);
    
    QHBoxLayout* layout = new QHBoxLayout(m_floatingWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    QFrame* bgFrame = new QFrame(m_floatingWidget);
    bgFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e0e0e0; border-radius: 8px; }"
    );
    // Use shadow if needed, but styling border is enough for now.
    
    QHBoxLayout* innerLayout = new QHBoxLayout(bgFrame);
    innerLayout->setContentsMargins(6, 4, 6, 4);
    innerLayout->setSpacing(8);

    QString btnStyle = 
        "QPushButton { background: transparent; color: #333; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: #f0f2f5; }";
        
    QPushButton* copyBtn = new QPushButton("复制", bgFrame);
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(btnStyle);
    copyBtn->setFocusPolicy(Qt::NoFocus);
    
    QPushButton* askBtn = new QPushButton("追问", bgFrame);
    askBtn->setCursor(Qt::PointingHandCursor);
    askBtn->setStyleSheet(btnStyle);
    askBtn->setFocusPolicy(Qt::NoFocus);
    
    innerLayout->addWidget(copyBtn);
    
    QFrame* divider = new QFrame(bgFrame);
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet("color: #e0e0e0;");
    innerLayout->addWidget(divider);
    
    innerLayout->addWidget(askBtn);
    layout->addWidget(bgFrame);
    
    connect(copyBtn, &QPushButton::clicked, this, [this, text]() {
        QGuiApplication::clipboard()->setText(text);
        hideFloatingMenu();
    });

    connect(askBtn, &QPushButton::clicked, this, [this, text]() {
        pushSelectionToPanel(text);
        hideFloatingMenu();
    });
    
    m_floatingWidget->move(pos + QPoint(10, 10));
    m_floatingWidget->show();
}

void AiContextMediator::hideFloatingMenu()
{
    if (m_floatingWidget) {
        m_floatingWidget->hide();
        m_floatingWidget->deleteLater();
        m_floatingWidget = nullptr;
    }
}
