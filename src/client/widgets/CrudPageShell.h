#pragma once

#include <QFrame>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <functional>

class CrudPageShell : public QFrame {
    Q_OBJECT

public:
    explicit CrudPageShell(QWidget* parent = nullptr);

    void setPageTitle(const QString& title);
    void setPageSubtitle(const QString& subtitle);
    void setFilterFields(const QStringList& placeholders);
    void setActionButtons(const QStringList& labels, bool addDangerOnLast = true);
    void setTableHeaders(const QStringList& headers);
    void setTableData(const QList<QStringList>& rows, const QList<int>& ids);
    void setEmptyMessage(const QString& msg);
    void setHelperText(const QString& text);

    int currentRowId() const;
    QLineEdit* filterField(int index) const;
    QTableWidget* table() const;

signals:
    void addActionClicked();
    void editActionClicked();
    void removeActionClicked();
    void rowDoubleClicked(int id);
    void filterChanged();

private:
    void setupUi();

    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QLabel* m_helperLabel = nullptr;
    QWidget* m_filterWidget = nullptr;
    QWidget* m_actionWidget = nullptr;
    QTableWidget* m_table = nullptr;
    QList<QLineEdit*> m_filterInputs;
    int m_emptyColumnSpan = 0;
};
