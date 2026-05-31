#pragma once

#include <QObject>
#include <QEvent>
#include <QPoint>
#include <QString>

class AiPanelWidget;

class AiContextMediator : public QObject {
    Q_OBJECT

public:
    explicit AiContextMediator(QObject* parent = nullptr);

    void bindRootWidget(QWidget* root);
    void attachPanel(AiPanelWidget* panel);

    bool eventFilter(QObject* watched, QEvent* event) override;

    void pushSelectionToPanel(const QString& text);
    void clearSelection();

    void showFloatingMenu(const QPoint& pos, const QString& text);
    void hideFloatingMenu();

private:
    void makeTextSelectable(QWidget* widget);
    QString selectedTextFromWidget(QWidget* widget) const;

    QWidget* m_rootWidget = nullptr;
    AiPanelWidget* m_panel = nullptr;
    QString m_lastSelection;
    QPoint m_mousePressGlobalPos;
    bool m_leftMousePressed = false;
    bool m_draggedEnoughForSelection = false;
    QWidget* m_floatingWidget = nullptr;
};
