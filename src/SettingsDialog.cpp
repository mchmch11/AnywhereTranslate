#include "SettingsDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("翻译设置");
    setModal(true);
    resize(620, 420);

    auto *tips = new QLabel("提示：AI 使用 OpenAI 兼容接口；腾讯云机器翻译使用 SecretId/SecretKey 并按 TC3 签名调用。", this);
    tips->setWordWrap(true);

    m_libreEndpointEdit = new QLineEdit(this);
    m_libreApiKeyEdit = new QLineEdit(this);
    m_aiEndpointEdit = new QLineEdit(this);
    m_aiApiKeyEdit = new QLineEdit(this);
    m_aiApiKeyEdit->setEchoMode(QLineEdit::Password);
    m_aiModelEdit = new QLineEdit(this);
    m_tencentSecretIdEdit = new QLineEdit(this);
    m_tencentSecretKeyEdit = new QLineEdit(this);
    m_tencentSecretKeyEdit->setEchoMode(QLineEdit::Password);
    m_tencentRegionEdit = new QLineEdit(this);
    m_tencentProjectIdEdit = new QLineEdit(this);

    auto *form = new QFormLayout;
    form->addRow("Libre 接口", m_libreEndpointEdit);
    form->addRow("Libre API Key", m_libreApiKeyEdit);
    form->addRow("AI 接口", m_aiEndpointEdit);
    form->addRow("AI API Key", m_aiApiKeyEdit);
    form->addRow("AI 模型", m_aiModelEdit);
    form->addRow("腾讯云 SecretId", m_tencentSecretIdEdit);
    form->addRow("腾讯云 SecretKey", m_tencentSecretKeyEdit);
    form->addRow("腾讯云 Region", m_tencentRegionEdit);
    form->addRow("腾讯云 ProjectId", m_tencentProjectIdEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tips);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void SettingsDialog::setLibreEndpoint(const QString &value) { m_libreEndpointEdit->setText(value); }
void SettingsDialog::setLibreApiKey(const QString &value) { m_libreApiKeyEdit->setText(value); }
void SettingsDialog::setAiEndpoint(const QString &value) { m_aiEndpointEdit->setText(value); }
void SettingsDialog::setAiApiKey(const QString &value) { m_aiApiKeyEdit->setText(value); }
void SettingsDialog::setAiModel(const QString &value) { m_aiModelEdit->setText(value); }
void SettingsDialog::setTencentSecretId(const QString &value) { m_tencentSecretIdEdit->setText(value); }
void SettingsDialog::setTencentSecretKey(const QString &value) { m_tencentSecretKeyEdit->setText(value); }
void SettingsDialog::setTencentRegion(const QString &value) { m_tencentRegionEdit->setText(value); }
void SettingsDialog::setTencentProjectId(const QString &value) { m_tencentProjectIdEdit->setText(value); }

QString SettingsDialog::libreEndpoint() const { return m_libreEndpointEdit->text().trimmed(); }
QString SettingsDialog::libreApiKey() const { return m_libreApiKeyEdit->text().trimmed(); }
QString SettingsDialog::aiEndpoint() const { return m_aiEndpointEdit->text().trimmed(); }
QString SettingsDialog::aiApiKey() const { return m_aiApiKeyEdit->text().trimmed(); }
QString SettingsDialog::aiModel() const { return m_aiModelEdit->text().trimmed(); }
QString SettingsDialog::tencentSecretId() const { return m_tencentSecretIdEdit->text().trimmed(); }
QString SettingsDialog::tencentSecretKey() const { return m_tencentSecretKeyEdit->text().trimmed(); }
QString SettingsDialog::tencentRegion() const { return m_tencentRegionEdit->text().trimmed(); }
QString SettingsDialog::tencentProjectId() const { return m_tencentProjectIdEdit->text().trimmed(); }
