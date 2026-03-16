#pragma once

#include <QDialog>

class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setLibreEndpoint(const QString &value);
    void setLibreApiKey(const QString &value);
    void setAiEndpoint(const QString &value);
    void setAiApiKey(const QString &value);
    void setAiModel(const QString &value);
    void setTencentSecretId(const QString &value);
    void setTencentSecretKey(const QString &value);
    void setTencentRegion(const QString &value);
    void setTencentProjectId(const QString &value);

    QString libreEndpoint() const;
    QString libreApiKey() const;
    QString aiEndpoint() const;
    QString aiApiKey() const;
    QString aiModel() const;
    QString tencentSecretId() const;
    QString tencentSecretKey() const;
    QString tencentRegion() const;
    QString tencentProjectId() const;

private:
    QLineEdit *m_libreEndpointEdit = nullptr;
    QLineEdit *m_libreApiKeyEdit = nullptr;
    QLineEdit *m_aiEndpointEdit = nullptr;
    QLineEdit *m_aiApiKeyEdit = nullptr;
    QLineEdit *m_aiModelEdit = nullptr;
    QLineEdit *m_tencentSecretIdEdit = nullptr;
    QLineEdit *m_tencentSecretKeyEdit = nullptr;
    QLineEdit *m_tencentRegionEdit = nullptr;
    QLineEdit *m_tencentProjectIdEdit = nullptr;
};
