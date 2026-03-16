#include "GlobalHotkeyWatcher.h"

#include <QMetaObject>
#include <QTimer>

GlobalHotkeyWatcher *GlobalHotkeyWatcher::s_instance = nullptr;

GlobalHotkeyWatcher::GlobalHotkeyWatcher(QObject *parent)
    : QObject(parent),
      m_resetTimer(new QTimer(this))
{
    s_instance = this;

    m_resetTimer->setSingleShot(true);
    m_resetTimer->setInterval(600); // Ctrl+C 后等待第二个 C 的窗口期
    connect(m_resetTimer, &QTimer::timeout, this, [this]() {
        m_waitingForC = false;
    });

    // 安装全局低级键盘钩子（运行于安装它的线程，即 Qt 主线程）
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookProc, nullptr, 0);
}

GlobalHotkeyWatcher::~GlobalHotkeyWatcher() {
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

LRESULT CALLBACK GlobalHotkeyWatcher::hookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && s_instance && wParam == WM_KEYDOWN) {
        const auto *kbd = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        if (kbd->vkCode == 'C') {
            const bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

            if (s_instance->m_waitingForC) {
                // 等待状态下再次按 C（无论 Ctrl 是否仍按着）：触发弹出翻译
                s_instance->m_waitingForC = false;
                s_instance->m_resetTimer->stop();

                POINT pt;
                GetCursorPos(&pt);
                const QPoint pos(pt.x, pt.y);

                QMetaObject::invokeMethod(s_instance, [pos]() {
                    emit s_instance->quickTranslateRequested(pos);
                }, Qt::QueuedConnection);
            } else if (ctrlDown) {
                // Ctrl+C（非等待状态）：开始等待第二个 C
                s_instance->m_waitingForC = true;
                s_instance->m_resetTimer->start();
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
