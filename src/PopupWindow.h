#pragma once

#include <QWidget>

class QComboBox;
class QLabel;

class PopupWindow : public QWidget {
    Q_OBJECT
public:
    explicit PopupWindow(QWidget *parent = nullptr);

    // 在指定屏幕坐标附近弹出
    void showAt(QPoint screenPos);

    // 同步主窗口当前引擎
    void setCurrentEngine(int index);

    void setLoading();
    void setResult(const QString &text);
    void setError(const QString &text);

signals:
    void engineChangeRequested(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QLabel    *m_label       = nullptr;
    QComboBox *m_engineCombo = nullptr;
    QPoint     m_anchorPos;
    QPoint     m_dragOffset;
    bool       m_dragging = false;

    void updatePosition();
};
