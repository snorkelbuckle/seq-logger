#include "mainwindow.h"
#include <QApplication>
#include "seqlogger.h"


int main(int argc, char *argv[])
    {
    SeqLogger *_myLog = SeqLogger::Instance();
    _myLog->SetHost("127.0.0.1");
    _myLog->SetMinimumLogLevel(SeqLogger::LogLevel::InformationLevel);
    _myLog->Information("Starting Main: {data}", {"Some data"});
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
    }
