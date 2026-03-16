#include "TranslatorService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageAuthenticationCode>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScopeGuard>
#include <QTimeZone>
#include <QUrl>

namespace {

QByteArray hmacSha256(const QByteArray &key, const QByteArray &message) {
    return QMessageAuthenticationCode::hash(message, key, QCryptographicHash::Sha256);
}

QString sha256Hex(const QByteArray &data) {
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

}  // namespace

TranslatorService::TranslatorService(QObject *parent)
    : QObject(parent) {}

void TranslatorService::translateWithLibre(const QString &text,
                                           const QString &sourceLang,
                                           const QString &targetLang,
                                           const QString &endpoint,
                                           const QString &apiKey) {
    QUrl url(endpoint.trimmed());
    if (!url.isValid()) {
        emit failed("LibreTranslate 地址无效");
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body{
        {"q", text},
        {"source", sourceLang},
        {"target", targetLang},
        {"format", "text"}
    };

    if (!apiKey.trimmed().isEmpty()) {
        body["api_key"] = apiKey.trimmed();
    }

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, Engine::Libre);
    });
}

void TranslatorService::translateWithAI(const QString &text,
                                        const QString &sourceLang,
                                        const QString &targetLang,
                                        const QString &endpoint,
                                        const QString &apiKey,
                                        const QString &model) {
    if (apiKey.trimmed().isEmpty()) {
        emit failed("AI API Key 不能为空");
        return;
    }

    QUrl url(endpoint.trimmed());
    if (!url.isValid()) {
        emit failed("AI 接口地址无效");
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey.trimmed()).toUtf8());

    const QString systemPrompt = QString("你是专业翻译引擎，只输出翻译结果，不要解释。源语言: %1，目标语言: %2。")
                                     .arg(sourceLang, targetLang);

    QJsonArray messages{
        QJsonObject{{"role", "system"}, {"content", systemPrompt}},
        QJsonObject{{"role", "user"}, {"content", text}}
    };

    QJsonObject body{
        {"model", model.trimmed().isEmpty() ? QStringLiteral("gpt-4o-mini") : model.trimmed()},
        {"messages", messages},
        {"temperature", 0.1}
    };

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, Engine::AI);
    });
}

void TranslatorService::translateWithTencent(const QString &text,
                                             const QString &sourceLang,
                                             const QString &targetLang,
                                             const QString &secretId,
                                             const QString &secretKey,
                                             const QString &region,
                                             int projectId) {
    const QString cleanedSecretId = secretId.trimmed();
    const QString cleanedSecretKey = secretKey.trimmed();
    const QString cleanedRegion = region.trimmed();

    if (cleanedSecretId.isEmpty() || cleanedSecretKey.isEmpty()) {
        emit failed("腾讯云 SecretId/SecretKey 不能为空");
        return;
    }
    if (cleanedRegion.isEmpty()) {
        emit failed("腾讯云 Region 不能为空，例如 ap-guangzhou");
        return;
    }
    if (text.size() > 6000) {
        emit failed("腾讯云单次请求建议不超过 6000 字符");
        return;
    }

    const QString host = QStringLiteral("tmt.tencentcloudapi.com");
    const QString service = QStringLiteral("tmt");
    const QString action = QStringLiteral("TextTranslate");
    const QString version = QStringLiteral("2018-03-21");
    const QString algorithm = QStringLiteral("TC3-HMAC-SHA256");
    const QString contentType = QStringLiteral("application/json; charset=utf-8");

    QJsonObject body{
        {"SourceText", text},
        {"Source", sourceLang},
        {"Target", targetLang},
        {"ProjectId", projectId}
    };
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    const qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    const QString date = QDateTime::fromSecsSinceEpoch(timestamp, QTimeZone::UTC).toString("yyyy-MM-dd");

    const QString canonicalHeaders = QString("content-type:%1\nhost:%2\nx-tc-action:%3\n")
                                         .arg(contentType, host, action.toLower());
    const QString signedHeaders = QStringLiteral("content-type;host;x-tc-action");
    const QString canonicalRequest = QString("POST\n/\n\n%1\n%2\n%3")
                                         .arg(canonicalHeaders,
                                              signedHeaders,
                                              sha256Hex(payload));

    const QString credentialScope = QString("%1/%2/tc3_request").arg(date, service);
    const QString stringToSign = QString("%1\n%2\n%3\n%4")
                                     .arg(algorithm)
                                     .arg(timestamp)
                                     .arg(credentialScope)
                                     .arg(sha256Hex(canonicalRequest.toUtf8()));

    const QByteArray secretDate = hmacSha256(QString("TC3%1").arg(cleanedSecretKey).toUtf8(), date.toUtf8());
    const QByteArray secretService = hmacSha256(secretDate, service.toUtf8());
    const QByteArray secretSigning = hmacSha256(secretService, "tc3_request");
    const QString signature = QString::fromLatin1(hmacSha256(secretSigning, stringToSign.toUtf8()).toHex());

    const QString authorization = QString("%1 Credential=%2/%3, SignedHeaders=%4, Signature=%5")
                                      .arg(algorithm, cleanedSecretId, credentialScope, signedHeaders, signature);

    QNetworkRequest request(QUrl("https://tmt.tencentcloudapi.com/"));
    request.setRawHeader("Authorization", authorization.toUtf8());
    request.setRawHeader("Content-Type", contentType.toUtf8());
    request.setRawHeader("Host", host.toUtf8());
    request.setRawHeader("X-TC-Action", action.toUtf8());
    request.setRawHeader("X-TC-Version", version.toUtf8());
    request.setRawHeader("X-TC-Timestamp", QByteArray::number(timestamp));
    request.setRawHeader("X-TC-Region", cleanedRegion.toUtf8());

    QNetworkReply *reply = m_network.post(request, payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply, Engine::Tencent);
    });
}

void TranslatorService::handleReply(QNetworkReply *reply, Engine engine) {
    const auto guard = qScopeGuard([reply]() {
        reply->deleteLater();
    });

    if (reply->error() != QNetworkReply::NoError) {
        emit failed(QString("网络错误: %1").arg(reply->errorString()));
        return;
    }

    const QByteArray payload = reply->readAll();
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit failed("返回数据不是有效 JSON");
        return;
    }

    const QJsonObject root = doc.object();

    if (engine == Engine::AI) {
        const QJsonArray choices = root.value("choices").toArray();
        if (choices.isEmpty()) {
            emit failed("AI 返回缺少 choices 字段");
            return;
        }

        const QJsonObject message = choices.first().toObject().value("message").toObject();
        const QString content = message.value("content").toString().trimmed();
        if (content.isEmpty()) {
            emit failed("AI 返回内容为空");
            return;
        }

        emit translated(content);
        return;
    }

    if (engine == Engine::Tencent) {
        const QJsonObject responseObj = root.value("Response").toObject();
        if (responseObj.isEmpty()) {
            emit failed("腾讯云返回格式异常：缺少 Response");
            return;
        }

        const QJsonObject errorObj = responseObj.value("Error").toObject();
        if (!errorObj.isEmpty()) {
            const QString code = errorObj.value("Code").toString();
            const QString message = errorObj.value("Message").toString();
            emit failed(QString("腾讯云错误: %1 - %2").arg(code, message));
            return;
        }

        const QString translatedText = responseObj.value("TargetText").toString().trimmed();
        if (translatedText.isEmpty()) {
            emit failed("腾讯云翻译结果为空");
            return;
        }

        emit translated(translatedText);
        return;
    }

    const QString translatedText = root.value("translatedText").toString().trimmed();
    if (translatedText.isEmpty()) {
        emit failed("翻译结果为空，可能是接口返回格式不匹配");
        return;
    }

    emit translated(translatedText);
}
