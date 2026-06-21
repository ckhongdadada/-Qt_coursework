import sys
from PySide6.QtWidgets import QApplication, QFrame, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, QLineEdit

app = QApplication(sys.argv)

masterInputCard = QFrame()
masterInputLayout = QVBoxLayout(masterInputCard)

m_contextCard = QFrame()
m_contextCard.setStyleSheet("QFrame { background: #f0f2f5; border: none; border-radius: 6px; }")
contextLayout = QHBoxLayout(m_contextCard)

m_contextLabel = QLabel("Hello World")
contextLayout.addWidget(m_contextLabel)

masterInputLayout.addWidget(m_contextCard)
m_contextCard.hide()

btn = QPushButton("Show")
masterInputLayout.addWidget(btn)

def show_card():
    m_contextCard.show()

btn.clicked.connect(show_card)

masterInputCard.show()
sys.exit(app.exec())
