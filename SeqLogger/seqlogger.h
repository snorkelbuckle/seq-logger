#ifndef SEQLOGGER_H
#define SEQLOGGER_H

#include "SeqLogger_global.h"
#include <algorithm>
#include <curl/curl.h>
#include <sys/time.h>
#include <iostream>
#include <stdexcept>
#include <QJsonObject>
#include <QObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QUrl>

using namespace std;


#ifdef QT_DEBUG
#define SEQPSEUDOSYNCH(a) SeqLogger::Instance()->SetPseudoSynchronous(a)
#define SEQMINIMUMLEVEL(a) SeqLogger::Instance()->SetMinimumLogLevel(a)
#define SEQHOST(a) SeqLogger::Instance()->SetHost(a)
#define SEQAPPNAME(a) SeqLogger::Instance()->SetApplicationName(a)
#define SEQLOGDEBUG(a,...) SeqLogger::Instance()->Log(QString(__FILE__),QString(__FUNCTION__),QString("%1").arg(__LINE__), SeqLogger::DebugLevel, a, ##__VA_ARGS__)
#define SEQLOGINFO(a,...) SeqLogger::Instance()->Log(QString(__FILE__),QString(__FUNCTION__),QString("%1").arg(__LINE__), SeqLogger::InformationLevel, a, ##__VA_ARGS__)
#define SEQLOGWARN(a,...) SeqLogger::Instance()->Log(QString(__FILE__),QString(__FUNCTION__),QString("%1").arg(__LINE__), SeqLogger::WarningLevel, a, ##__VA_ARGS__)
#define SEQLOGERROR(a,...) SeqLogger::Instance()->Log(QString(__FILE__),QString(__FUNCTION__),QString("%1").arg(__LINE__), SeqLogger::ErrorLevel, a, ##__VA_ARGS__)
#define SEQLOGCRIT(a,...) SeqLogger::Instance()->Log(QString(__FILE__),QString(__FUNCTION__),QString("%1").arg(__LINE__), SeqLogger::CriticalLevel, a, ##__VA_ARGS__)
#elif
#define SEQPSEUDOSYNCH(a) do {} while (0)
#define SEQMINIMUMLEVEL(a) do {} while (0)
#define SEQHOST(a) do {} while (0)
#define SEQAPPNAME(a) do {} while (0)
#define SEQLOGDEBUG(a,...) do {} while (0)
#define SEQLOGINFO(a,...) do {} while (0)
#define SEQLOGWARN(a,...) do {} while (0)
#define SEQLOGERROR(a,...) do {} while (0)
#define SEQLOGCRIT(a,...) do {} while (0)
#endif

class SEQLOGGER_EXPORT SeqLogger : public QObject
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
        void SetApplicationName(QString name);
        void SetHost(QString host, QString port = "5341", QString protocol = "http");
        void SetMinimumLogLevel(LogLevel level);
        void Log(QString file_name, QString func_name, QString line_no, LogLevel level, QString message_template, ...);

    private:
        explicit SeqLogger(QObject *parent = nullptr);
        ~SeqLogger();
        SeqLogger(SeqLogger const &);
        SeqLogger &operator=(SeqLogger const &);
        static SeqLogger *_instance;

        QString _app_name;
        QString _protocol;
        QString _host;
        QString _port;
        QUrl _api_endpoint;

        LogLevel _minimum_log_level;
        QString _loglevel_names[6] = {"Debug", "Information", "Warning", "Error", "Critical", "None" };


        QRegularExpression *_message_template_regex;

        void LogMessage(LogLevel level, QString message_template, QList<QString> arguments, QString file_name,
                        QString func_name, QString line_no);
        void PostMessage(QJsonDocument *document);
        void GetMessageArguments(QJsonObject &json, QString message_template,
                                 QList<QString> arguments);
        QJsonObject CreateJsonMessage(LogLevel level, QString message_template, QList<QString> arguments, QString file_name,
                                      QString func_name, QString line_no);

        uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed);
        string nowWithMicroSeconds();



        // Curl Stuff, requires libcurl
        CURL *curl;
        struct curl_slist *header_slist = nullptr;
        long curl_response_code;
        std::string response_data;

        bool initCurl();
        static size_t writeFunc(void *ptr, size_t size, size_t nmemb, std::string *data);

    };


#endif // SEQLOGGER_H
