#pragma once

#include <QObject>
#include <QPoint>
#include <windows.h>

class QTimer;

class GlobalHotkeyWatcher : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyWatcher(QObject *parent = nullptr);
    ~GlobalHotkeyWatcher() override;

signals:
    void quickTranslateRequested(QPoint screenPos);

private:
    static LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_hook = nullptr;
    QTimer *m_resetTimer = nullptr;
    bool m_waitingForC = false;

    static GlobalHotkeyWatcher *s_instance;
};
