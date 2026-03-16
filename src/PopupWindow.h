#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class PopupWindow : public QWidget {
    Q_OBJECT
public:
    explicit PopupWindow(QWidget *parent = nullptr);

    // 在指定屏幕坐标附近弹出（出现在光标下方）
    void showAt(QPoint screenPos);

    void setLoading();
    void setResult(const QString &text);
    void setError(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *m_label = nullptr;
    QTimer *m_autoCloseTimer = nullptr;
    QPoint m_anchorPos;

    void updatePosition();
};
