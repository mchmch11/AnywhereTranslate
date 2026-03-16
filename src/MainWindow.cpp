#include "MainWindow.h"

#include "GlobalHotkeyWatcher.h"
#include "PopupWindow.h"
#include "SettingsDialog.h"
#include "TranslatorService.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_translator(new TranslatorService(this)),
      m_clipboard(QApplication::clipboard()),
      m_clipboardDebounceTimer(new QTimer(this)),
      m_hotkeyWatcher(new GlobalHotkeyWatcher(this)),
      m_popupWindow(new PopupWindow()) {
    buildUi();
    setupTray();
    loadSettings();

    m_clipboardDebounceTimer->setSingleShot(true);
    m_clipboardDebounceTimer->setInterval(350);

    connect(m_translateButton, &QPushButton::clicked, this, &MainWindow::requestTranslate);
    connect(m_clipboard, &QClipboard::dataChanged, this, &MainWindow::onClipboardChanged);
    connect(m_clipboardDebounceTimer, &QTimer::timeout, this, &MainWindow::onDebounceTimeout);
    connect(m_hotkeyWatcher, &GlobalHotkeyWatcher::quickTranslateRequested,
            this, &MainWindow::onQuickTranslateRequested);

    connect(m_translator, &TranslatorService::translated, this, [this](const QString &result) {
        m_isBusy = false;
        m_translateButton->setEnabled(true);
        m_resultEdit->setPlainText(result);
        m_statusLabel->setText("翻译完成");

        if (m_popupWindow && m_popupWindow->isVisible()) {
            m_popupWindow->setResult(result);
        }

        if (m_trayIcon && m_trayIcon->isVisible()) {
            QString brief = result;
            brief.replace('\n', ' ');
            if (brief.size() > 100) {
                brief = brief.left(100) + "...";
            }
            m_trayIcon->showMessage("翻译结果", brief, QSystemTrayIcon::Information, 2500);
        }
    });

    connect(m_translator, &TranslatorService::failed, this, [this](const QString &error) {
        m_isBusy = false;
        m_translateButton->setEnabled(true);
        m_statusLabel->setText(error);

        if (m_popupWindow && m_popupWindow->isVisible()) {
            m_popupWindow->setError(error);
        }
    });
}

void MainWindow::buildUi() {
    setWindowTitle("Anywhere Translate (Qt6)");
    resize(880, 620);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *sourceLangLabel = new QLabel("源语言", this);
    auto *targetLangLabel = new QLabel("目标语言", this);
    auto *engineLabel = new QLabel("引擎", this);

    m_sourceLangEdit = new QLineEdit("auto", this);
    m_targetLangEdit = new QLineEdit("zh", this);

    m_engineCombo = new QComboBox(this);
    m_engineCombo->addItem("LibreTranslate");
    m_engineCombo->addItem("AI 翻译");
    m_engineCombo->addItem("腾讯云机器翻译");

    m_translateButton = new QPushButton("立即翻译", this);
    auto *clearButton = new QPushButton("清空", this);
    auto *settingsButton = new QPushButton("设置", this);

    m_autoClipboardCheck = new QCheckBox("监听剪贴板（复制划词后自动翻译）", this);
    m_autoClipboardCheck->setChecked(false);

    m_sourceEdit = new QPlainTextEdit(this);
    m_sourceEdit->setPlaceholderText("在任意软件中划词后复制（Ctrl+C），这里会自动捕获并翻译");

    m_resultEdit = new QPlainTextEdit(this);
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText("翻译结果");

    m_statusLabel = new QLabel("就绪", this);

    auto *topForm = new QFormLayout;
    auto *langRow = new QHBoxLayout;
    langRow->addWidget(sourceLangLabel);
    langRow->addWidget(m_sourceLangEdit);
    langRow->addSpacing(8);
    langRow->addWidget(targetLangLabel);
    langRow->addWidget(m_targetLangEdit);
    langRow->addSpacing(8);
    langRow->addWidget(engineLabel);
    langRow->addWidget(m_engineCombo);
    topForm->addRow(langRow);

    auto *buttonsRow = new QHBoxLayout;
    buttonsRow->addWidget(m_translateButton);
    buttonsRow->addWidget(clearButton);
    buttonsRow->addWidget(settingsButton);
    buttonsRow->addStretch();
    buttonsRow->addWidget(m_autoClipboardCheck);

    auto *layout = new QVBoxLayout(central);
    layout->addLayout(topForm);
    layout->addLayout(buttonsRow);
    layout->addWidget(new QLabel("原文", this));
    layout->addWidget(m_sourceEdit, 1);
    layout->addWidget(new QLabel("译文", this));
    layout->addWidget(m_resultEdit, 1);
    layout->addWidget(m_statusLabel);

    connect(clearButton, &QPushButton::clicked, this, [this]() {
        m_sourceEdit->clear();
        m_resultEdit->clear();
        m_statusLabel->setText("已清空");
    });

    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
}

void MainWindow::setupTray() {
    const QIcon trayIcon = QIcon::fromTheme("accessories-dictionary",
                                            style()->standardIcon(QStyle::SP_FileDialogContentsView));

    m_trayIcon = new QSystemTrayIcon(trayIcon, this);
    auto *menu = new QMenu(this);

    auto *showAction = menu->addAction("显示窗口");
    m_autoClipboardAction = menu->addAction("监听剪贴板");
    m_autoClipboardAction->setCheckable(true);
    m_autoClipboardAction->setChecked(false);

    auto *quitAction = menu->addAction("退出");

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->show();

    connect(showAction, &QAction::triggered, this, [this]() {
        showNormal();
        activateWindow();
        raise();
    });

    connect(m_autoClipboardAction, &QAction::toggled, this, [this](bool enabled) {
        m_autoClipboardCheck->setChecked(enabled);
        m_statusLabel->setText(enabled ? "已开启剪贴板监听" : "已关闭剪贴板监听");
        saveSettings();
    });

    connect(m_autoClipboardCheck, &QCheckBox::toggled, this, [this](bool enabled) {
        if (m_autoClipboardAction->isChecked() != enabled) {
            m_autoClipboardAction->setChecked(enabled);
        }
        saveSettings();
    });

    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            showNormal();
            activateWindow();
            raise();
        }
    });
}

void MainWindow::loadSettings() {
    QSettings settings("AnywhereTranslate", "AnywhereTranslate");

    m_sourceLangEdit->setText(settings.value("lang/source", "auto").toString());
    m_targetLangEdit->setText(settings.value("lang/target", "zh").toString());
    m_engineCombo->setCurrentIndex(settings.value("engine/index", 0).toInt());

    const bool autoClipboard = settings.value("clipboard/auto", false).toBool();
    m_autoClipboardCheck->setChecked(autoClipboard);
    if (m_autoClipboardAction) {
        m_autoClipboardAction->setChecked(autoClipboard);
    }

    m_libreEndpoint = settings.value("api/libreEndpoint", "https://libretranslate.com/translate").toString();
    m_libreApiKey = settings.value("api/libreApiKey", "").toString();
    m_aiEndpoint = settings.value("api/aiEndpoint", "https://api.openai.com/v1/chat/completions").toString();
    m_aiApiKey = settings.value("api/aiApiKey", "").toString();
    m_aiModel = settings.value("api/aiModel", "gpt-4o-mini").toString();
    m_tencentSecretId = settings.value("api/tencentSecretId", "").toString();
    m_tencentSecretKey = settings.value("api/tencentSecretKey", "").toString();
    m_tencentRegion = settings.value("api/tencentRegion", "ap-guangzhou").toString();
    m_tencentProjectId = settings.value("api/tencentProjectId", 0).toInt();
}

void MainWindow::saveSettings() const {
    QSettings settings("AnywhereTranslate", "AnywhereTranslate");

    settings.setValue("lang/source", m_sourceLangEdit->text().trimmed());
    settings.setValue("lang/target", m_targetLangEdit->text().trimmed());
    settings.setValue("engine/index", m_engineCombo->currentIndex());
    settings.setValue("clipboard/auto", m_autoClipboardCheck->isChecked());

    settings.setValue("api/libreEndpoint", m_libreEndpoint);
    settings.setValue("api/libreApiKey", m_libreApiKey);
    settings.setValue("api/aiEndpoint", m_aiEndpoint);
    settings.setValue("api/aiApiKey", m_aiApiKey);
    settings.setValue("api/aiModel", m_aiModel);
    settings.setValue("api/tencentSecretId", m_tencentSecretId);
    settings.setValue("api/tencentSecretKey", m_tencentSecretKey);
    settings.setValue("api/tencentRegion", m_tencentRegion);
    settings.setValue("api/tencentProjectId", m_tencentProjectId);
}

void MainWindow::requestTranslate() {
    if (m_isBusy) {
        return;
    }

    const QString text = m_sourceEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        m_statusLabel->setText("请输入或复制要翻译的文本");
        return;
    }

    const QString sourceLang = m_sourceLangEdit->text().trimmed().isEmpty() ? "auto" : m_sourceLangEdit->text().trimmed();
    const QString targetLang = m_targetLangEdit->text().trimmed().isEmpty() ? "zh" : m_targetLangEdit->text().trimmed();

    m_isBusy = true;
    m_translateButton->setEnabled(false);
    m_statusLabel->setText("翻译中...");

    if (m_engineCombo->currentText() == "AI 翻译") {
        m_translator->translateWithAI(text, sourceLang, targetLang, m_aiEndpoint, m_aiApiKey, m_aiModel);
    } else if (m_engineCombo->currentText() == "腾讯云机器翻译") {
        m_translator->translateWithTencent(text,
                                           sourceLang,
                                           targetLang,
                                           m_tencentSecretId,
                                           m_tencentSecretKey,
                                           m_tencentRegion,
                                           m_tencentProjectId);
    } else {
        m_translator->translateWithLibre(text, sourceLang, targetLang, m_libreEndpoint, m_libreApiKey);
    }

    saveSettings();
}

void MainWindow::openSettings() {
    SettingsDialog dialog(this);
    dialog.setLibreEndpoint(m_libreEndpoint);
    dialog.setLibreApiKey(m_libreApiKey);
    dialog.setAiEndpoint(m_aiEndpoint);
    dialog.setAiApiKey(m_aiApiKey);
    dialog.setAiModel(m_aiModel);
    dialog.setTencentSecretId(m_tencentSecretId);
    dialog.setTencentSecretKey(m_tencentSecretKey);
    dialog.setTencentRegion(m_tencentRegion);
    dialog.setTencentProjectId(QString::number(m_tencentProjectId));

    if (dialog.exec() == QDialog::Accepted) {
        m_libreEndpoint = dialog.libreEndpoint();
        m_libreApiKey = dialog.libreApiKey();
        m_aiEndpoint = dialog.aiEndpoint();
        m_aiApiKey = dialog.aiApiKey();
        m_aiModel = dialog.aiModel();
        m_tencentSecretId = dialog.tencentSecretId();
        m_tencentSecretKey = dialog.tencentSecretKey();
        m_tencentRegion = dialog.tencentRegion();
        m_tencentProjectId = dialog.tencentProjectId().toInt();
        saveSettings();
        m_statusLabel->setText("设置已保存");
    }
}

void MainWindow::onClipboardChanged() {
    if (!m_autoClipboardCheck->isChecked()) {
        return;
    }

    const QString text = m_clipboard->text().trimmed();
    if (text.isEmpty() || text == m_lastClipboardText) {
        return;
    }

    m_lastClipboardText = text;
    m_sourceEdit->setPlainText(text);
    m_statusLabel->setText("检测到剪贴板新文本，准备翻译...");
    m_clipboardDebounceTimer->start();
}

void MainWindow::onDebounceTimeout() {
    if (!m_autoClipboardCheck->isChecked()) {
        return;
    }
    requestTranslate();
}

void MainWindow::onQuickTranslateRequested(QPoint screenPos) {
    const QString text = m_clipboard->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    m_popupWindow->showAt(screenPos);
    m_popupWindow->setLoading();

    if (!m_isBusy) {
        // 用剪贴板内容触发翻译（若自动监听未开启也能工作）
        m_sourceEdit->setPlainText(text);
        requestTranslate();
    }
    // 若正在翻译中，translated 信号触发时会自动更新已显示的气泡
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
        m_statusLabel->setText("程序已最小化到托盘");
        m_trayIcon->showMessage("Anywhere Translate", "程序仍在后台运行，可继续划词复制翻译。", QSystemTrayIcon::Information, 1800);
        return;
    }

    saveSettings();
    QMainWindow::closeEvent(event);
}
