#pragma once

#include <QFrame>
#include <QTableWidget>

struct PeerBenchmark;

class PeerBenchmarkWidget : public QFrame {
    Q_OBJECT

public:
    explicit PeerBenchmarkWidget(QWidget* parent = nullptr);

    void loadPeers(const QList<PeerBenchmark>& peers);
    int currentPeerId() const;
    int peerCount() const;

signals:
    void addPeerRequested();
    void editPeerRequested(int id);
    void removePeerRequested(int id);

private slots:
    void onCellDoubleClicked(int row, int col);

private:
    void setupUi();

    QTableWidget* m_peerTable = nullptr;
    int m_peerCount = 0;
};
