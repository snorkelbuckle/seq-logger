#include "seqlogger.h"
#include <QtDebug>

SeqLogger *SeqLogger::_instance = nullptr;

SeqLogger::SeqLogger(QObject *parent) : QObject(parent)
    {
    _protocol = "http";
    _host = "127.0.0.1";
    _port = "5341";
    _api_endpoint = QUrl(QString("%1://%2:%3/api/events/raw?clef").arg(_protocol).arg(_host).arg(_port));
    _message_template_regex = new QRegularExpression(R"RX({(.*)})RX",
            QRegularExpression::PatternOption::InvertedGreedinessOption);
    _minimum_log_level = LogLevel::InformationLevel;

    if (!initCurl())
        throw std::invalid_argument("Unable to initialize curl");
    }

SeqLogger::~SeqLogger()
    {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
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

void SeqLogger::SetApplicationName(QString name)
    {
    if (name.isNull())
        _app_name = "_name_not_defined";
    else
        _app_name = name;
    }

void SeqLogger::SetHost(QString host,  QString port, QString protocol)
    {
    _api_endpoint = QUrl(QString("%1://%2:%3/api/events/raw?clef").arg(protocol).arg(host).arg(port));
    }

void SeqLogger::SetMinimumLogLevel(LogLevel level)
    {
    _minimum_log_level = level;
    }

SeqLogger &SeqLogger::operator=(const SeqLogger &)
    {
    return *this;
    }

void SeqLogger::LogMessage(LogLevel level, QString message_template, QList<QString> arguments, QString file_name,
                           QString func_name, QString line_no)
    {
    if (level >= _minimum_log_level)
        {
        QJsonDocument *doc = new QJsonDocument(CreateJsonMessage(level, message_template, arguments, file_name, func_name,
                                               line_no));
        PostMessage(doc);
        }
    }

void SeqLogger::PostMessage(QJsonDocument *document)
    {
    QByteArray url = _api_endpoint.url().toLatin1();
    QByteArray json = document->toJson(QJsonDocument::JsonFormat::Compact);
    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.data());
    //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_response_code);
    curl_easy_perform(curl);
    qDebug() << "response data: " << response_data.c_str();
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

void SeqLogger::Log(QString file_name, QString func_name, QString line_no, SeqLogger::LogLevel level,
                    QString message_template, ...)
    {
    va_list args;
    va_start(args, message_template);
    QList<QString> args_list;
    QRegularExpressionMatchIterator i = _message_template_regex->globalMatch(message_template);

    while (i.hasNext())
        {
        QRegularExpressionMatch m = i.next();

        if (m.hasMatch())
            {
            char *a = va_arg(args, char *);
            args_list.append(QString(a));
            }
        }

    va_end(args);
    LogMessage(level, message_template, args_list, file_name, func_name, line_no);
    }


QJsonObject SeqLogger::CreateJsonMessage(LogLevel level, QString message_template, QList<QString> arguments,
        QString file_name, QString func_name, QString line_no)
    {
    QString prefix_template = "{app}";
    arguments.prepend(_app_name);
    int precompile_args = 0;

    if (!file_name.isNull())
        {
        precompile_args++;
        prefix_template = prefix_template + "+{src}";
        arguments.insert(precompile_args, file_name);
        }

    if (!func_name.isNull())
        {
        precompile_args++;
        prefix_template = prefix_template + "::{func}";
        arguments.insert(precompile_args, func_name);
        }

    if (!line_no.isNull())
        {
        precompile_args++;
        prefix_template = prefix_template + "@{line}";
        arguments.insert(precompile_args, line_no);
        }

    message_template = prefix_template + " $ " + message_template;
    QJsonObject json;
    json["@t"] = nowWithMicroSeconds().c_str();
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

bool SeqLogger::initCurl()
    {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.69.1");
    header_slist = curl_slist_append(header_slist, "Content-Type: application/vnd.serilog.clef");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_slist);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &curl_response_code);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &SeqLogger::writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    return true;
    }

size_t SeqLogger::writeFunc(void *ptr, size_t size, size_t nmemb, string *data)
    {
    data->append((char *) ptr, size * nmemb);
    return size * nmemb;
    }

string SeqLogger::nowWithMicroSeconds()
    {
    string result = "";
    char gmtbuffer[160];
    struct timeval now_tv;
    time_t now_time_t;
    struct tm *now_tm;
    gettimeofday(&now_tv, NULL);
    now_time_t = now_tv.tv_sec;
    now_tm = gmtime(&now_time_t);
    int result_len = strftime(gmtbuffer, 160, "%Y-%m-%dT%H:%M:%S", now_tm);

    if (result_len != 0)
        {
        string microsecs = to_string(now_tv.tv_usec);
        std::reverse(std::begin(microsecs), std::end(microsecs));
        result = gmtbuffer + string(".") + microsecs.substr(0, 3) + string("Z");
        }

    return result;
    }


