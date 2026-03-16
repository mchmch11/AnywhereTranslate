#include "PopupWindow.h"

#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

PopupWindow::PopupWindow(QWidget *parent)
    : QWidget(parent,
              Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
      m_label(new QLabel(this)),
      m_autoCloseTimer(new QTimer(this))
{
    m_label->setWordWrap(true);
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->addWidget(m_label);

    setStyleSheet(
        "PopupWindow {"
        "  background-color: #2b2b2b;"
        "  border: 1px solid #555555;"
        "  border-radius: 6px;"
        "}"
        "QLabel {"
        "  color: #f0f0f0;"
        "  font-size: 13px;"
        "  line-height: 1.4;"
        "}"
    );

    setMinimumWidth(180);
    setMaximumWidth(480);

    m_autoCloseTimer->setSingleShot(true);
    m_autoCloseTimer->setInterval(6000);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &QWidget::hide);
}

void PopupWindow::showAt(QPoint screenPos) {
    m_anchorPos = screenPos;
    m_autoCloseTimer->stop();
    updatePosition();
    show();
    raise();
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
    m_autoCloseTimer->start();
}

void PopupWindow::setError(const QString &text) {
    m_label->setText("翻译失败：" + text);
    adjustSize();
    updatePosition();
    m_autoCloseTimer->start();
}

void PopupWindow::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    hide();
}

void PopupWindow::updatePosition() {
    adjustSize();

    // 默认出现在光标下方偏右
    QPoint pos = m_anchorPos + QPoint(0, 22);

    // 尝试找到当前屏幕，确保弹窗不超出屏幕边界
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
            // 上方显示
            pos.setY(m_anchorPos.y() - height() - 8);
        }
    }

    move(pos);
}
