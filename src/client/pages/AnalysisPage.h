#pragma once

#include "BasePage.h"
#include "core/DataDomain.h"
#include <QLabel>

class SemesterAnalysisWidget;
class PeerBenchmarkWidget;

class AnalysisPage : public BasePage {
    Q_OBJECT

public:
    explicit AnalysisPage(QWidget* parent = nullptr);
    ~AnalysisPage() override = default;

    void refresh() override;

signals:
    void dataChanged(DataDomain domain);

private slots:
    void onAddPeerRequested();
    void onEditPeerRequested(int id);
    void onRemovePeerRequested(int id);

private:
    void setupUi();

    SemesterAnalysisWidget* m_semesterWidget = nullptr;
    PeerBenchmarkWidget* m_peerWidget = nullptr;

    QLabel* m_summaryLabel = nullptr;
    QLabel* m_semesterValue = nullptr;
    QLabel* m_peerValue = nullptr;
    QLabel* m_suggestionValue = nullptr;
};
