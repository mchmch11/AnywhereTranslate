#include "PopupWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

PopupWindow::PopupWindow(QWidget *parent)
    : QWidget(parent,
              Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
      m_label(new QLabel(this)),
      m_engineCombo(new QComboBox(this))
{
    // 不设置 WA_TranslucentBackground，确保白色背景不透明、边框可见

    // --- 引擎下拉 ---
    m_engineCombo->addItem("LibreTranslate");
    m_engineCombo->addItem("AI 翻译");
    m_engineCombo->addItem("腾讯云机器翻译");
    m_engineCombo->setFocusPolicy(Qt::NoFocus);

    // --- 关闭按钮 ---
    auto *closeBtn = new QPushButton("×", this);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    closeBtn->setObjectName("closeBtn");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);

    // --- 顶部工具栏 ---
    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);
    headerLayout->addWidget(m_engineCombo, 1);
    headerLayout->addWidget(closeBtn);

    // --- 翻译结果标签 ---
    m_label->setWordWrap(true);
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_label->setObjectName("resultLabel");

    // --- 主布局 ---
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 10);
    layout->setSpacing(6);
    layout->addLayout(headerLayout);
    layout->addWidget(m_label);

    setStyleSheet(
        "PopupWindow {"
        "  background-color: #ffffff;"
        "  border: 3px solid #444444;"
        "}"
        "QLabel#resultLabel {"
        "  color: #1a1a1a;"
        "  font-size: 13px;"
        "}"
        "QComboBox {"
        "  background-color: #f5f5f5;"
        "  color: #1a1a1a;"
        "  border: 1px solid #cccccc;"
        "  border-radius: 3px;"
        "  padding: 2px 6px;"
        "  font-size: 12px;"
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #ffffff;"
        "  color: #1a1a1a;"
        "  selection-background-color: #e0e0e0;"
        "}"
        "QPushButton#closeBtn {"
        "  background-color: transparent;"
        "  color: #888888;"
        "  border: none;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#closeBtn:hover {"
        "  color: #cc3333;"
        "}"
    );

    setMinimumWidth(200);
    setMaximumWidth(480);

    connect(m_engineCombo, &QComboBox::currentIndexChanged,
            this, &PopupWindow::engineChangeRequested);
}

void PopupWindow::showAt(QPoint screenPos) {
    m_anchorPos = screenPos;
    updatePosition();
    show();
    raise();
    activateWindow();
}

void PopupWindow::setCurrentEngine(int index) {
    // 临时断开信号，避免同步时触发 engineChangeRequested
    QSignalBlocker blocker(m_engineCombo);
    m_engineCombo->setCurrentIndex(index);
}

void PopupWindow::setLoading() {
    m_label->setText("翻译中...");
    adjustSize();
    updatePosition();
}

void PopupWindow::setResult(const QString &text) {
    m_label->setText(text);
    adjustSize();
    updatePosition();
}

void PopupWindow::setError(const QString &text) {
    m_label->setText("<span style='color:#ff6b6b;'>翻译失败：</span>" + text);
    adjustSize();
    updatePosition();
}

void PopupWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void PopupWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void PopupWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragOffset);
    }
    QWidget::mouseMoveEvent(event);
}

void PopupWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void PopupWindow::updatePosition() {
    adjustSize();

    QPoint pos = m_anchorPos + QPoint(0, 22);

    QScreen *screen = QApplication::screenAt(m_anchorPos);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    if (screen) {
        const QRect avail = screen->availableGeometry();
        if (pos.x() + width() > avail.right() - 4) {
            pos.setX(avail.right() - width() - 4);
        }
        if (pos.x() < avail.left()) {
            pos.setX(avail.left() + 4);
        }
        if (pos.y() + height() > avail.bottom() - 4) {
            pos.setY(m_anchorPos.y() - height() - 8);
        }
    }

    move(pos);
}
