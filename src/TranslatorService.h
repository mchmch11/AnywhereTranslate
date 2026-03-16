#pragma once

#include <QObject>
#include <QNetworkAccessManager>

class QNetworkReply;

class TranslatorService : public QObject {
    Q_OBJECT
public:
    enum class Engine {
        Libre,
        AI,
        Tencent
    };

    explicit TranslatorService(QObject *parent = nullptr);

    void translateWithLibre(const QString &text,
                            const QString &sourceLang,
                            const QString &targetLang,
                            const QString &endpoint,
                            const QString &apiKey = QString());

    void translateWithAI(const QString &text,
                         const QString &sourceLang,
                         const QString &targetLang,
                         const QString &endpoint,
                         const QString &apiKey,
                         const QString &model);

    void translateWithTencent(const QString &text,
                              const QString &sourceLang,
                              const QString &targetLang,
                              const QString &secretId,
                              const QString &secretKey,
                              const QString &region,
                              int projectId);

signals:
    void translated(const QString &result);
    void failed(const QString &error);

private:
    void handleReply(QNetworkReply *reply, Engine engine);

    QNetworkAccessManager m_network;
};
