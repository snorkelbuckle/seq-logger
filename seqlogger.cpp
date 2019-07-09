#include "seqlogger.h"

SeqLogger *SeqLogger::_instance = nullptr;

SeqLogger::SeqLogger(QObject *parent) : QObject(parent)
    {
    _protocol = "http";
    _host = "127.0.0.1";
    _port = "5341";
    _api_endpoint = QUrl(QString("%1://%2:%3/api/events/raw?clef").arg(_protocol).arg(_host).arg(_port));
    _network_manager = new QNetworkAccessManager();
    _message_template_regex = new QRegularExpression(R"RX({(.*)})RX",
            QRegularExpression::PatternOption::InvertedGreedinessOption);
    _minimum_log_level = LogLevel::InformationLevel;
    }

SeqLogger::SeqLogger(const SeqLogger &) : QObject()
    {
    }

SeqLogger *SeqLogger::Instance()
    {
    if (_instance == nullptr)
        _instance = new SeqLogger;

    return _instance;
    }

void SeqLogger::SetHost(QString host, QString port, QString protocol)
    {
    _api_endpoint = QUrl(QString("%1://%2:%3/api/events/raw?clef").arg(protocol).arg(host).arg(port));
    }

void SeqLogger::SetMinimumLogLevel(LogLevel level)
    {
    _minimum_log_level = level;
    }

void SeqLogger::Critical(QString message_template, QList<QString> arguments = {})
    {
    LogMessage(LogLevel::CriticalLevel, message_template, arguments);
    }

void SeqLogger::Error(QString message_template, QList<QString> arguments = {})
    {
    LogMessage(LogLevel::ErrorLevel, message_template, arguments);
    }

void SeqLogger::Warning(QString message_template, QList<QString> arguments = {})
    {
    LogMessage(LogLevel::WarningLevel, message_template, arguments);
    }

void SeqLogger::Information(QString message_template, QList<QString> arguments = {})
    {
    LogMessage(LogLevel::InformationLevel, message_template, arguments);
    }

void SeqLogger::Debug(QString message_template, QList<QString> arguments = {})
    {
    LogMessage(LogLevel::DebugLevel, message_template, arguments);
    }

SeqLogger &SeqLogger::operator=(const SeqLogger &)
    {
    return *this;
    }

void SeqLogger::LogMessage(LogLevel level, QString message_template, QList<QString> arguments)
    {
    if (level >= _minimum_log_level)
        {
        QJsonDocument *doc = new QJsonDocument(CreateJsonMessage(level, message_template, arguments));
        PostMessage(doc);
        }
    }

void SeqLogger::PostMessage(QJsonDocument *document)
    {
    QNetworkRequest request;
    request.setUrl(_api_endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network_manager->post(request, document->toJson(QJsonDocument::JsonFormat::Compact));
    }

void SeqLogger::GetMessageArguments(QJsonObject &json, QString message_template,
                                    QList<QString> arguments)
    {
    QRegularExpressionMatchIterator i = _message_template_regex->globalMatch(message_template);

    while (i.hasNext())
        {
        QRegularExpressionMatch m = i.next();

        if (m.hasMatch())
            json[m.captured(1)] = arguments.isEmpty() ? "" : arguments.takeFirst();
        }
    }

QJsonObject SeqLogger::CreateJsonMessage(LogLevel level, QString message_template, QList<QString> arguments)
    {
    QJsonObject json;
    json["@t"] = QDateTime::currentDateTimeUtc().toString(Qt::DateFormat::ISODateWithMs);
    json["@mt"] = message_template;
    json["@l"] = _loglevel_names[level];
    QByteArray ba = message_template.toLocal8Bit().constData();
    unsigned char *res = reinterpret_cast<unsigned char *>(strdup(ba));
    json["@i"] = QString("%1").number(murmur3_32(res, static_cast<size_t>(ba.length()), 23143), 16);
    GetMessageArguments(json, message_template, arguments);
    return json;
    }

uint32_t SeqLogger::murmur3_32(const uint8_t *key, size_t len, uint32_t seed)
    {
    uint32_t h = seed;

    if (len > 3)
        {
        size_t i = len >> 2;

        do
            {
            uint32_t k;
            memcpy(&k, key, sizeof(uint32_t));
            key += sizeof(uint32_t);
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
            }
        while (--i);
        }

    if (len & 3)
        {
        size_t i = len & 3;
        uint32_t k = 0;

        do
            {
            k <<= 8;
            k |= key[i - 1];
            }
        while (--i);

        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
        }

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
    }
