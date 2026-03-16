#pragma once

#include <QMainWindow>
#include <QPoint>

class QAction;
class QCheckBox;
class QClipboard;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSystemTrayIcon;
class QTimer;
class TranslatorService;
class GlobalHotkeyWatcher;
class PopupWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void requestTranslate();
    void openSettings();
    void onClipboardChanged();
    void onDebounceTimeout();
    void onQuickTranslateRequested(QPoint screenPos);

private:
    void buildUi();
    void setupTray();
    void loadSettings();
    void saveSettings() const;

    QPlainTextEdit *m_sourceEdit = nullptr;
    QPlainTextEdit *m_resultEdit = nullptr;
    QLineEdit *m_sourceLangEdit = nullptr;
    QLineEdit *m_targetLangEdit = nullptr;
    QComboBox *m_engineCombo = nullptr;
    QPushButton *m_translateButton = nullptr;
    QCheckBox *m_autoClipboardCheck = nullptr;
    QLabel *m_statusLabel = nullptr;

    TranslatorService *m_translator = nullptr;
    QClipboard *m_clipboard = nullptr;
    QTimer *m_clipboardDebounceTimer = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QAction *m_autoClipboardAction = nullptr;

    QString m_lastClipboardText;
    bool m_isBusy = false;

    GlobalHotkeyWatcher *m_hotkeyWatcher = nullptr;
    PopupWindow *m_popupWindow = nullptr;

    QString m_libreEndpoint;
    QString m_libreApiKey;
    QString m_aiEndpoint;
    QString m_aiApiKey;
    QString m_aiModel;
    QString m_tencentSecretId;
    QString m_tencentSecretKey;
    QString m_tencentRegion;
    int m_tencentProjectId = 0;
};
