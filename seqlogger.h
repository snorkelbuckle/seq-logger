#ifndef SEQLOGGER_H
#define SEQLOGGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QRegularExpression>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


class SeqLogger : public QObject
    {
        Q_OBJECT

    public:
        enum LogLevel : uint
            {
            DebugLevel = 0,
            InformationLevel = 1,
            WarningLevel = 2,
            ErrorLevel = 3,
            CriticalLevel = 4,
            NoLevel = 5
            };
        Q_ENUM(LogLevel)

        static SeqLogger *Instance();
        void SetHost(QString host, QString port = "5341", QString protocol = "http");
        void SetMinimumLogLevel(LogLevel level);
        void Critical(QString message_template, QList<QString> arguments);
        void Error(QString message_template, QList<QString> arguments);
        void Warning(QString message_template, QList<QString> arguments);
        void Information(QString message_template, QList<QString> arguments);
        void Debug(QString message_template, QList<QString> arguments);


    private:
        explicit SeqLogger(QObject *parent = nullptr);
        SeqLogger(SeqLogger const &);
        SeqLogger &operator=(SeqLogger const &);
        static SeqLogger *_instance;

        QString _protocol;
        QString _host;
        QString _port;
        QUrl _api_endpoint;

        LogLevel _minimum_log_level;
        QString _loglevel_names[6] = {"Debug", "Information", "Warning", "Error", "Critical", "None" };

        QNetworkAccessManager *_network_manager;
        QRegularExpression *_message_template_regex;


        void LogMessage(LogLevel level, QString message_template, QList<QString> arguments);
        void PostMessage(QJsonDocument *document);
        void GetMessageArguments(QJsonObject &json, QString message_template,
                                 QList<QString> arguments);
        QJsonObject CreateJsonMessage(LogLevel level, QString message_template, QList<QString> arguments);
        uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed);


    signals:

    public slots:
    };

#endif // SEQLOGGER_H
