#pragma once

#include <QDialog>

#include "model/PeerBenchmark.h"

class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QWidget;

class PeerEditorDialog : public QDialog {
public:
    explicit PeerEditorDialog(QWidget* parent = nullptr);

    void setPeer(const PeerBenchmark& peer);
    PeerBenchmark peer() const;

private:
    void validateAndAccept();

    int m_peerId = 0;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_majorEdit = nullptr;
    QLineEdit* m_semesterEdit = nullptr;
    QDoubleSpinBox* m_gpaSpin = nullptr;
    QDoubleSpinBox* m_creditsSpin = nullptr;
    QSpinBox* m_achievementSpin = nullptr;
    QSpinBox* m_experienceSpin = nullptr;
    QPlainTextEdit* m_noteEdit = nullptr;
};
